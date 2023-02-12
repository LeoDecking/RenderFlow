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

PyObject *pyTensorflowModule;

// TODO unload

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
    // TODO escript decref?
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

bool PythonRender::init(EScript::Runtime &rt, std::string path /* = "" */)
{
    std::cout << "init python: " << pthread_self() << std::endl;
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
    PyRun_SimpleString("import importlib.util; import sys; import gc"); // TODO move up?

    if (path.size())
        return loadModule(path);
    else
        return true;
}

bool PythonRender::loadModule(std::string path)
{
    std::cout << "load module: " << path << std::endl;

    std::replace(path.begin(), path.end(), '\'', ' ');
    std::string name = path + "";
    std::replace(name.begin(), name.end(), '.', ' ');

    PyRun_SimpleString(("if '" + name + "' in sys.modules: module = sys.modules['" + name + "']").c_str());
    PyRun_SimpleString(("if '" + name + "' not in sys.modules: spec = importlib.util.spec_from_file_location('" + name + "', '" + path + "'); module = importlib.util.module_from_spec(spec); sys.modules['" + name + "'] = module;spec.loader.exec_module(module)").c_str());
    PyRun_SimpleString("if hasattr(module, 'init'): module.init()");


    if (pyModule)
        Py_DECREF(pyModule);

    PyObject *m = PyImport_AddModule("__main__");
    pyModule = PyObject_GetAttrString(m, "module");

    return pyModule != NULL;
}

void PythonRender::finalizeModule()
{
    if (runtime)
        PyRun_SimpleString("if module != None and hasattr(module, 'finalize'): module.finalize(); gc.collect()");
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

    npy_intp dims[1] = {(npy_intp)prerender.size()};
    PyObject *in_array = PyArray_SimpleNewFromData(1, dims, NPY_INT32, prerender.data());
    PyObject *args =  Py_BuildValue("(O)", in_array);

    PyObject *result = PyObject_CallObject(pyFunction, prerender.size() ?args:NULL);

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
    PyArrayObject *flattenedArray = (PyArrayObject *)PyArray_Flatten(array, NPY_CORDER);

    // std::cout << "time prepare numpy array: " << clock() - start << std::endl;
    // start = clock();

    Py_ssize_t size = PyArray_SIZE(flattenedArray);
    std::vector<float> vector(size);

    for (size_t i = 0; i < size; i++)
    {
        vector[i] = *(double *)PyArray_GETPTR1(flattenedArray, i);
    }
    // std::cout << "time to c array: " << clock() - start << std::endl;

    Py_DecRef((PyObject *)flattenedArray);
    Py_DecRef((PyObject *)array);
    Py_DecRef(result);
    return vector;
}

bool PythonRender::loadModel(std::string pythonPath, std::string modelPath, EScript::Object *shape, std::string input_name, std::string output_name)
{
    std::cout << "load model: " << modelPath << std::endl;
    if (!pyTensorflowModule)
    {
        std::replace(pythonPath.begin(), pythonPath.end(), '\'', ' ');
        std::replace(modelPath.begin(), modelPath.end(), '\'', ' ');
        PyRun_SimpleString(("tSpec = importlib.util.spec_from_file_location('tensorflowModule', '" + pythonPath + "'); tMmodule = importlib.util.module_from_spec(tSpec);sys.modules['tensorflowModule'] = tMmodule;tSpec.loader.exec_module(tMmodule)").c_str());

        std::cout << "python loaded" << std::endl;

        PyObject *m = PyImport_AddModule("__main__");
        pyTensorflowModule = PyObject_GetAttrString(m, "tMmodule");
    }

    PyObject *pyFunction = PyObject_GetAttrString(pyTensorflowModule, "loadModel");
    if (!pyFunction || !PyCallable_Check(pyFunction))
    {
        std::cerr << "python loadModel function not found." << std::endl;
        return {};
    }

    PyObject *pathObj = PyUnicode_FromString(modelPath.c_str());
    PyObject *shapeObj = escriptToPython(shape);
    PyObject *inObj = PyUnicode_FromString(input_name.c_str());
    PyObject *outObj = PyUnicode_FromString(output_name.c_str());

    PyObject *args = PyTuple_New(4);
    PyTuple_SetItem(args, 0, pathObj);
    PyTuple_SetItem(args, 1, shapeObj);
    PyTuple_SetItem(args, 2, inObj);
    PyTuple_SetItem(args, 3, outObj);

    std::cout << "load model:" << std::endl;
    PyObject *result = PyObject_CallObject(pyFunction, args);
    std::cout << "model loaded" << std::endl;

    // Py_DECREF(pathObj);
    // Py_DECREF(shapeObj);
    // Py_DECREF(inObj);
    // Py_DECREF(outObj);
    Py_DECREF(args);
    Py_DECREF(pyFunction);

    if (!result)
    {
        std::cerr << "error:" << std::endl;
        PyErr_Print();
        std::cerr << "python load module call failed." << std::endl;
        return {};
    }

    Py_DECREF(result);

    return pyTensorflowModule != NULL;
}

void PythonRender::unloadloadModel()
{
    if (runtime)
        PyRun_SimpleString("if tMmodule != None: tMmodule.unload(); gc.collect()");
}
std::vector<float> lastInput;
std::vector<float> lastOutput;

std::vector<float> PythonRender::predict(std::vector<float> &input, bool cache)
{
    if (cache && input.size() == lastInput.size())
    {
        bool same = true;
        for (int i = 0; i < input.size(); i++)
            if (input[i] != lastInput[i])
            {
                same = false;
                break;
            }
        if (same)
            return lastOutput;
    }

    // time_t start = clock();

    PyObject *pyFunction = PyObject_GetAttrString(pyTensorflowModule, "predict");
    if (!pyFunction || !PyCallable_Check(pyFunction))
    {
        std::cerr << "python predict function not found." << std::endl;
        return {};
    }

    npy_intp dims[1] = {(long long int)input.size()};
    PyObject *in_array = PyArray_SimpleNewFromData(1, dims, NPY_FLOAT32, input.data());
    PyObject *args = Py_BuildValue("(O)", in_array);

    PyObject *result = PyObject_CallObject(pyFunction, args);
    Py_DECREF(args);
    Py_DECREF(in_array);
    Py_DECREF(pyFunction);

    if (!result)
    {
        PyErr_Print();
        std::cout << "python predict call failed." << std::endl;
        return {};
    }
    // std::cout << "time call: " << clock() - start << std::endl;
    // start = clock();

    PyArrayObject *array = (PyArrayObject *)PyArray_FromArray((PyArrayObject *)result, PyArray_DescrFromType(NPY_FLOAT64), 0);
    PyArrayObject *flattenedArray = (PyArrayObject *)PyArray_Flatten(array, NPY_CORDER);

    // std::cout << "time prepare numpy array: " << clock() - start << std::endl;
    // start = clock();

    Py_ssize_t size = PyArray_SIZE(flattenedArray);
    std::vector<float> vector(size);

    for (size_t i = 0; i < size; i++)
    {
        vector[i] = *(double *)PyArray_GETPTR1(flattenedArray, i);
    }
    // std::cout << "time to c array: " << clock() - start << std::endl;

    Py_DecRef((PyObject *)flattenedArray);
    Py_DecRef((PyObject *)array);
    Py_DecRef(result);

    return vector;
}
