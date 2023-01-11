#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <EScript/Utils/StringData.h>
#include <EScript/Utils/ObjArray.h>
#include <EScript/Utils/RuntimeHelper.h>
#include <iostream>
#include <string>
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

bool PythonRender::init(std::string path, EScript::Runtime &rt)
{
    runtime = &rt;

    PyImport_AppendInittab("escript", &PyInit_escript);

    Py_Initialize();
    PyObject *pyPath = PyUnicode_DecodeFSDefault(path.c_str());
    pyModule = PyImport_Import(pyPath);

    if (pyModule == NULL)
    {
        if (PyErr_Occurred())
            PyErr_Print();
        std::cout << "failed to load: " << path << std::endl;
        Py_DECREF(pyPath);
        return false;
    }
    Py_DECREF(pyPath);

    return true;
}

bool PythonRender::finalize()
{
    if (pyModule)
        Py_DECREF(pyModule);
    pyModule = NULL;
    if (Py_FinalizeEx() < 0)
    {
        std::cerr << "error while finalizing python" << std::endl;
        return false;
    }
    return true;
}

std::vector<float> PythonRender::render(std::vector<int> prerender)
{
    // time_t start = clock();

    PyObject *pyFunction = PyObject_GetAttrString(pyModule, "render");
    if (!pyFunction || !PyCallable_Check(pyFunction))
    {
        std::cerr << "python render function not found." << std::endl;
        return {};
    }

    PyObject *list = PyList_New(prerender.size());
    for (size_t i = 0; i < prerender.size(); i++)
        PyList_SET_ITEM(list, i, PyLong_FromLong(prerender[i]));
    PyObject *args = Py_BuildValue("(O)", list);

    PyObject *result = PyObject_CallObject(pyFunction, args);

    // std::cout << "time call: " << clock() - start << std::endl;
    // start = clock();

    Py_DECREF(args);
    Py_DECREF(list);
    Py_DECREF(pyFunction);

    if (!result)
    {
        PyErr_Print();
        std::cerr << "python render call failed." << std::endl;
        return {};
    }

    // TODO slow
    result = PySequence_Fast(result, "return type must be iterable"); // TODO reference count?

    if (!result)
    {
        PyErr_Print();
        std::cerr << "wrong python render result type." << std::endl;
        return {};
    }

    if (!result)
        return {};

    Py_ssize_t size = PySequence_Fast_GET_SIZE(result);
    std::vector<float> vector(size);

    // std::cout << "time prepare to c array: " << clock() - start << std::endl;
    // start = clock();

    for (size_t i = 0; i < size; i++)
        vector[i] = PyFloat_AS_DOUBLE(PyNumber_Float(PySequence_Fast_GET_ITEM(result, i)));

    // std::cout << "time to c array: " << clock() - start << std::endl;

    return vector;
}