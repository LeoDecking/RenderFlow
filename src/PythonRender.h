#include <string>
#include <vector>

class PythonRender
{
public:
    static void test();
    static bool init(std::string path);
    static bool finalize();
    static std::vector<float> render(std::vector<int> prerender);
};