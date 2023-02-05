#define PY_SSIZE_T_CLEAN
#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/arrayobject.h"

#include <EScript/Utils/StringData.h>
#include <EScript/Utils/ObjArray.h>
#include <EScript/Utils/RuntimeHelper.h>
#include <EScript/StdObjects.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <time.h>
#include "PythonRender.h"

// TODO error when numpy is imported a second time, import it for all?

PyObject *pyModule;
EScript::Runtime *runtime;

static PyObject *eval(PyObject *self, PyObject *args)
{
    const char *s;
    if (!PyArg_ParseTuple(args, "s:code", &s))
        return NULL;

    std::string c = s;
    if (c[c.size() - 1] != ';')
        c += ';';

    auto r = EScript::eval(*runtime, EScript::StringData(c));

    if (!r.first)
    {
        // TODO error handling
        return NULL;
    }
    return PythonRender::escriptToPython(r.second.detach()); // detachAndDecrease?
}

static PyMethodDef Methods[] = {{"eval", eval, METH_VARARGS, "evaluate the escript code and return the result if primary types"}, {NULL, NULL, 0, NULL}};
static PyModuleDef Module = {PyModuleDef_HEAD_INIT, "escript", NULL, -1, Methods, NULL, NULL, NULL, NULL};
static PyObject *PyInit_escript(void) { return PyModule_Create(&Module); }

bool PythonRender::init(EScript::Runtime &rt, std::string path)
{
    if (!runtime)
    {
        runtime = &rt;

        PyConfig config;
        PyConfig_InitPythonConfig(&config);

        // config.module_search_paths_set = 1;
        // PyWideStringList_Append(&config.module_search_paths, std::filesystem::current_path().c_str());

        PyImport_AppendInittab("escript", &PyInit_escript);

        Py_InitializeFromConfig(&config); // TODO error checking

        import_array();
    }

    PyRun_SimpleString("import importlib.util; import sys");

    return loadModule(path);
}

bool PythonRender::loadModule(std::string path)
{
    if (pyModule)
        Py_DECREF(pyModule);

    std::replace(path.begin(), path.end(), '\'', ' ');
    PyRun_SimpleString(("spec = importlib.util.spec_from_file_location('renderModule', '" + path + "'); module = importlib.util.module_from_spec(spec);sys.modules['renderModule'] = module;spec.loader.exec_module(module)").c_str());

    PyObject *m = PyImport_AddModule("__main__");
    pyModule = PyObject_GetAttrString(m, "module");

    return pyModule != NULL;
}

// crashes with numpy
// bool PythonRender::finalize()
// {
//     if (pyModule)
//         Py_DECREF(pyModule);
//     pyModule = NULL;
//     if (Py_FinalizeEx() < 0)
//     {
//         std::cerr << "error while finalizing python" << std::endl;
//         return false;
//     }
//     return true;
// }

// TODO matrices?
PyObject *PythonRender::escriptToPython(EScript::Object *obj, bool hashable /* = false */)
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
        EScript::ERef<EScript::Iterator> itRef = dynamic_cast<EScript::Iterator *>(a->getIterator());

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

// TODO NPY_FLOAT64 only when float?
std::vector<float> PythonRender::render(std::vector<int> &prerender)
{
    // time_t start = clock();

    PyObject *pyFunction = PyObject_GetAttrString(pyModule, "render");
    if (!pyFunction || !PyCallable_Check(pyFunction))
    {
        std::cerr << "python render function not found." << std::endl;
        return {};
    }

    npy_intp dims[1] = {(long long int)prerender.size()};
    PyObject *in_array = PyArray_SimpleNewFromData(1, dims, NPY_INT32, prerender.data());
    PyObject *args = Py_BuildValue("(O)", in_array);

    PyObject *result = PyObject_CallObject(pyFunction, args);

    Py_DECREF(args);
    Py_DECREF(in_array);
    Py_DECREF(pyFunction);

    if (!result)
    {
        PyErr_Print();
        std::cerr << "python render call failed." << std::endl;
        return {};
    }

    // std::cout << "time call: " << clock() - start << std::endl;
    // start = clock();

    if (!PyArray_Check(result))
    {
        std::cerr << "wrong return type: use numpy array" << std::endl;
        return {};
    }
    PyArrayObject *array = (PyArrayObject *)PyArray_FromArray((PyArrayObject *)result, PyArray_DescrFromType(NPY_FLOAT64), 0);
    array = (PyArrayObject *)PyArray_Flatten(array, NPY_CORDER);

    // std::cout << "time prepare numpy array: " << clock() - start << std::endl;
    // start = clock();

    Py_ssize_t size = PyArray_SIZE(array);
    std::vector<float> vector(size);

    for (size_t i = 0; i < size; i++)
    {
        vector[i] = *(double *)PyArray_GETPTR1(array, i);
    }
    // std::cout << "time to c array: " << clock() - start << std::endl;

    return vector;
}