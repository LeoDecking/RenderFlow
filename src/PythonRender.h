#include <string>
#include <vector>
#include "EScript/Runtime/Runtime.h"

class PythonRender
{
public:
    static void test();
    static bool init(std::string path, EScript::Runtime &runtime);
    static bool finalize();
    static std::vector<float> render(std::vector<int> prerender);
};