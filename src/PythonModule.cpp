#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <EScript/StdObjects.h>
#include <string>
#include <iostream>
#include <cstdlib>

#include <thread>

#include "PythonRender.h"

#include "PythonModule.h"

// TODO int float conversions

PythonModule::PythonModule(std::string path, bool reuse /* = true */)
{
    std::cout << "load module: " << path << std::endl;

    std::replace(path.begin(), path.end(), '\'', ' ');
    name = path + "";
    std::replace(name.begin(), name.end(), '.', ' ');

    if (!reuse)
        name += std::to_string(std::rand());

    std::cout << "name: " << name << std::endl;

    PyRun_SimpleString(("if '" + name + "' in sys.modules: module = sys.modules['" + name + "']").c_str());
    PyRun_SimpleString(("if '" + name + "' not in sys.modules: spec = importlib.util.spec_from_file_location('" + name + "', '" + path + "'); module = importlib.util.module_from_spec(spec);sys.modules['" + name + "'] = module;spec.loader.exec_module(module)").c_str());
    PyRun_SimpleString("module.init()");

    pyModule = PyObject_GetAttrString(PythonRender::mainModule, "module");
}

EScript::Object *PythonModule::pythonToEScript(PyObject *obj, bool hashable)
{
    if (obj == Py_None || obj == NULL)
    {
        return EScript::create(nullptr);
    }
    else if (PyLong_Check(obj))
    {
        return EScript::create(PyLong_AsDouble(obj));
    }
    else if (PyBool_Check(obj))
    {
        return EScript::create(obj == Py_True);
    }
    else if (hashable)
    {
        return EScript::create(PyUnicode_AS_DATA(PyObject_Str(obj)));
    }
    else if (PyList_Check(obj))
    {
        EScript::Array *array = EScript::Array::create();
        array->reserve(PyList_Size(obj));

        for (int i = 0; i < array->size(); i++)
            array->setValue(EScript::create(i), pythonToEScript(PyList_GET_ITEM(obj, i)));

        return array;
    }
    else if (PyDict_Check(obj))
    {
        EScript::Map *map = EScript::Map::create();

        PyObject *key, *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(obj, &pos, &key, &value))
        {
            map->setValue(pythonToEScript(key, true), pythonToEScript(value));
        }

        return map;
    }

    return EScript::create(PyUnicode_AS_DATA(PyObject_Str(obj)));
}

PyObject *PythonModule::escriptToPython(EScript::Object *obj, bool hashable /* = false */)
{
    if (dynamic_cast<EScript::Void *>(obj) || (obj == nullptr))
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else if (EScript::Object *number = dynamic_cast<EScript::Number *>(obj))
    {
        return PyFloat_FromDouble(number->toDouble());
    }
    else if (EScript::Bool *b = dynamic_cast<EScript::Bool *>(obj))
    {
        if (b->toBool())
        {
            Py_INCREF(Py_True);
            return Py_True;
        }
        else
        {
            Py_INCREF(Py_False);
            return Py_False;
        }
    }
    else if (hashable)
    {
        return PyUnicode_FromString(obj->toString().c_str());
    }
    else if (EScript::Array *a = dynamic_cast<EScript::Array *>(obj))
    {
        PyObject *list = PyList_New(a->size());
        for (size_t i = 0; i < a->size(); i++)
            PyList_SET_ITEM(list, i, escriptToPython(a->at(i).get()));

        return list;
    }
    else if (EScript::Map *m = dynamic_cast<EScript::Map *>(obj))
    {
        PyObject *dict = PyDict_New();
        EScript::ERef<EScript::Iterator> itRef = dynamic_cast<EScript::Iterator *>(m->getIterator());

        while ((!itRef.isNull()) && (!itRef->end()))
        {
            EScript::ObjRef keyRef = itRef->key();
            EScript::ObjRef valueRef = itRef->value();

            PyDict_SetItem(dict, escriptToPython(keyRef.get(), true), escriptToPython(valueRef.get()));

            itRef->next();
        }
        return dict;
    }

    return PyUnicode_FromString(obj->toString().c_str());
}

PyObject *PythonModule::execute(std::string name, PyObject *args, bool optional /* = false */)
{

    PyObject *pyFunction = PyObject_HasAttrString(pyModule, name.c_str()) ? PyObject_GetAttrString(pyModule, name.c_str()) : NULL;
    if (!pyFunction || !PyCallable_Check(pyFunction))
    {
        if (optional)
            Py_RETURN_NONE;

        std::cerr << "python function '" << name << "' not found." << std::endl;
        return NULL;
    }

    PyObject *result = PyObject_CallObject(pyFunction, args);
    Py_DECREF(pyFunction);

    if (result == NULL) // TODO what is if no return?
    {
        PyErr_Print();
        std::cout << "python call '" << name << "' failed." << std::endl;
        return NULL;
    }

    return result;
}

EScript::Object *PythonModule::execute(std::string name, EScript::Array *args, bool optional /* = false */)
{
    PyObject *result = execute(name, PyList_AsTuple(escriptToPython(args)), optional);

    if (result == NULL) // TODO what is if no return?
    {
        return EScript::create(nullptr);
    }

    EScript::Object *eResult = pythonToEScript(result);
    Py_DecRef(result);

    return eResult;
}