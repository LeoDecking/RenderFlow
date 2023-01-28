#define PY_SSIZE_T_CLEAN
#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/arrayobject.h"

#include <EScript/Utils/StringData.h>
#include <EScript/Utils/ObjArray.h>
#include <EScript/Utils/RuntimeHelper.h>
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
    return PyUnicode_FromString(r.second.toString().c_str());
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

    return loadModule(path);
    // if (!numpyImported)
    // {
    //     PyRun_SimpleString("import numpy");
    //     PyRun_SimpleString("print('import numpy')");
    //     numpyImported = true;
    // }

    // TODO path
    // std::string p = "import sys;sys.path.append(\"" + std::filesystem::current_path().string() + "\")";
    // std::replace(p.begin(), p.end(), '\\', '/');
    // PyRun_SimpleString(p.c_str());
}

bool PythonRender::loadModule(std::string path)
{
    PyObject *pyPath = PyUnicode_DecodeFSDefault(path.c_str());
    PyObject *loadedModule = PyImport_GetModule(pyPath);

    if (pyModule)
        Py_DECREF(pyModule);

    pyModule = loadedModule ? PyImport_ReloadModule(loadedModule) : PyImport_Import(pyPath);

    if (pyModule == NULL)
    {
        if (PyErr_Occurred())
            PyErr_Print();
        std::cout << (loadedModule ? "failed to reload: " : "failed to load: ") << path << std::endl;
        Py_DECREF(pyPath);
        return false;
    }

    Py_DECREF(pyPath);
    return true;
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