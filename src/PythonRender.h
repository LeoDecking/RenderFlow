#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <string>
#include <vector>
#include "EScript/Runtime/Runtime.h"

class PythonRender
{
public:
    static void test();
    static bool init(EScript::Runtime &runtime, std::string path = "");
    static bool loadModule(std::string path);
    static bool loadModel(std::string pythonPath, std::string modelPath, EScript::Object *shape, std::string input_name, std::string output_name);
    static std::vector<float> predict(std::vector<float> &input, bool cache);
    static PyObject *escriptToPython(EScript::Object *obj, bool hashable = false);
    // static bool finalize();
    static std::vector<float> render(std::vector<int> &prerender);
};