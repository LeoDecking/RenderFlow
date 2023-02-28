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
#include "PythonModule.h"

#include "PythonRender.h"

PythonModule *pyTensorflowModule;
PythonModule *pyModule;

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
    return PythonModule::escriptToPython(r.second.detach()); // detachAndDecrease?
}

static PyMethodDef Methods[] = {{"eval", eval, METH_VARARGS, "evaluate the escript code and return the result if primary types"}, {NULL, NULL, 0, NULL}};
static PyModuleDef Module = {PyModuleDef_HEAD_INIT, "escript", NULL, -1, Methods, NULL, NULL, NULL, NULL};
static PyObject *PyInit_escript(void) { return PyModule_Create(&Module); }

void PythonRender::init(EScript::Runtime &rt, std::string path /* = "" */)
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

        if (_import_array() < 0)
        {
            PyErr_Print();
            std::cerr << "numpy.core.multiarray failed to import";
        }
        PyRun_SimpleString("import importlib.util; import sys; import gc"); // TODO move up?
    }

    if (path.size())
    {
        pyModule = new PythonModule(path);
        pyModule->execute("init", EScript::Array::create(), true);
    }
}

void PythonRender::finalizeModule()
{
    if (pyModule)
    {
        pyModule->execute("finalize", EScript::Array::create(), true);
        PyRun_SimpleString("gc.collect()");
    }
}

// TODO NPY_FLOAT64 only when float?
std::vector<float> PythonRender::render(std::vector<int> &prerender)
{
    // time_t start = clock();

    npy_intp dims[1] = {(npy_intp)prerender.size()};
    PyObject *in_array = PyArray_SimpleNewFromData(1, dims, NPY_INT32, prerender.data());
    PyObject *args = Py_BuildValue("(O)", in_array);

    PyObject *result = pyModule->execute("render", prerender.size() ? args : NULL);

    Py_DECREF(args);
    Py_DECREF(in_array);

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

void PythonRender::loadModel(std::string pythonPath, std::string modelPath, EScript::Object *shape, std::string input_name, std::string output_name)
{
    std::cout << "load model: " << modelPath << std::endl;
    if (!pyTensorflowModule)
    {
        pyTensorflowModule = new PythonModule(pythonPath);

        std::cout << "python loaded" << std::endl;
    }

    PyObject *pathObj = PyUnicode_FromString(modelPath.c_str());
    PyObject *shapeObj = PythonModule::escriptToPython(shape);
    PyObject *inObj = PyUnicode_FromString(input_name.c_str());
    PyObject *outObj = PyUnicode_FromString(output_name.c_str());

    PyObject *args = PyTuple_New(4);
    PyTuple_SetItem(args, 0, pathObj);
    PyTuple_SetItem(args, 1, shapeObj);
    PyTuple_SetItem(args, 2, inObj);
    PyTuple_SetItem(args, 3, outObj);

    std::cout << "load model:" << std::endl;
    PyObject *result = pyTensorflowModule->execute("loadModel", args);
    std::cout << "model loaded" << std::endl;

    // Py_DECREF(pathObj);
    // Py_DECREF(shapeObj);
    // Py_DECREF(inObj);
    // Py_DECREF(outObj);
    Py_DECREF(args);

    if (!result)
    {
        std::cerr << "error:" << std::endl;
        PyErr_Print();
        std::cerr << "python load model call failed." << std::endl;
    }
    else
    {
        Py_DECREF(result);
    }
}

void PythonRender::unloadloadModel()
{
    if (pyTensorflowModule)
    {
        pyTensorflowModule->execute("unload", EScript::Array::create(), true);
        PyRun_SimpleString("gc.collect()");
    }
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

    npy_intp dims[1] = {(long long int)input.size()};
    PyObject *in_array = PyArray_SimpleNewFromData(1, dims, NPY_FLOAT32, input.data());
    PyObject *args = Py_BuildValue("(O)", in_array);

    PyObject *result = pyTensorflowModule->execute("predict", args);
    Py_DECREF(args);
    Py_DECREF(in_array);

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
