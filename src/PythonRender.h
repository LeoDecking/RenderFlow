#include <string>
#include <vector>
#include "EScript/Runtime/Runtime.h"

class PythonRender
{
public:
    static void test();
    static bool init(EScript::Runtime &runtime, std::string path);
    static bool loadModule(std::string path);
    // static bool finalize();
    static std::vector<float> render(std::vector<int> &prerender);
};