#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <string>
#include <vector>
#include "EScript/Runtime/Runtime.h"

class PythonRender
{
public:
    static void init(EScript::Runtime &runtime, std::string path = "");
    static void loadModule(std::string path);
    static void finalizeModule();
    static void loadModel(std::string pythonPath, std::string modelPath, EScript::Object *shape, std::string input_name, std::string output_name);
    static void unloadloadModel();
    static std::vector<float> predict(std::vector<float> &input, bool cache);
    // static bool finalize();
    static std::vector<float> render(std::vector<int> &prerender);
};