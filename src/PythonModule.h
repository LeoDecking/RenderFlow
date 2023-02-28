#ifndef RENDERFLOW_PYTHONMODULE_H_
#define RENDERFLOW_PYTHONMODULE_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <EScript/StdObjects.h>
#include <string>

class PythonModule
{
private:
    PyObject *pyModule;
    std::string name;

public:
    PythonModule(std::string path, bool reuse = true);
    static PyObject *escriptToPython(EScript::Object *obj, bool hashable = false);
    static EScript::Object *pythonToEScript(PyObject *obj, bool hashable = false);
    PyObject *execute(std::string name, PyObject *args, bool optional = false);
    EScript::Object *execute(std::string name, EScript::Array *args, bool optional = false);

    inline bool operator==(const PythonModule &other)
    {
        return name == other.name;
    }

    inline bool operator!=(const PythonModule &other)
    {
        return name != other.name;
    }

    virtual ~PythonModule()
    {
        Py_DecRef(pyModule);
    }
};

#endif