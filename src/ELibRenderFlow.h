#ifndef ELIB_RENDERFLOW_H_ /* begin of include guard: ELIB_RENDERFLOW_H_ */
#define ELIB_RENDERFLOW_H_ /* C++ header files should always be enclosed by unique include guards */

#include <Rendering/RenderingContext/RenderingContext.h>
#include <Rendering/Texture/Texture.h>

// Forward declaration
namespace EScript {
class Namespace;
}

// Your namespace
namespace RenderFlow {
// EScript initializer method
void init(EScript::Namespace* lib);
std::vector<int> getTextureData(Rendering::RenderingContext &context, Rendering::Texture &texture);
}

#endif /* end of include guard: ELIB_RENDERFLOW_H_ */