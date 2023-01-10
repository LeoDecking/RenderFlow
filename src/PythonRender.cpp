#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <iostream>
#include <string>
#include <time.h>
#include "PythonRender.h"

void PythonRender::test()
{
    wchar_t *program = Py_DecodeLocale("Test42", NULL);
    if (program == NULL)
    {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        // exit(1);
    }
    Py_SetProgramName(program); /* optional but recommended */
    Py_Initialize();
    PyRun_SimpleString("from time import time,ctime\n"
                       "print('Today is', ctime(time()))\n");
    if (Py_FinalizeEx() < 0)
    {
        // exit(120);
    }
    PyMem_RawFree(program);
    std::cout << "Python juhuu!" << std::endl;
}

PyObject *pyModule;

bool PythonRender::init(std::string path)
{
    // Py_SetProgramName(std::wstring(L"../extPlugins/RenderFlow/Test.py").c_str());
    Py_Initialize();
    PyObject *pyPath = PyUnicode_DecodeFSDefault(path.c_str());
    pyModule = PyImport_Import(pyPath);
    Py_DECREF(pyPath);

    if (pyModule == NULL)
    {
        PyErr_Print();
        std::cout << "failed to load: " << path << std::endl;
        return false;
    }
    return true;
}

bool PythonRender::finalize()
{
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