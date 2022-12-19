#include "ELibRenderFlow.h" // change this according to your project name
#include <EScript/EScript.h>
#include <iostream>

#ifndef LIBRARY_NAME
#define LIBRARY_NAME "RenderFlow" // Set the library name
#endif

/**
 * Main entry point for this library.
 * This function is called from EScript using the 'LibUtilExt/LibraryLoader' module.
 * The 'lib' parameter is usually the GLOBALS namespace from EScript.
 *
 * The 'extern "C"' signalizes the compiler that it should tread this function as a plain C function
 * which can be easily found during dynamic linking (otherwise the compiler mangles the name).
 */
extern "C" LIBRARY_API void initLibrary(EScript::Namespace *lib)
{
    std::cout << "Loading Library: " << LIBRARY_NAME << "..." << std::endl;
    RenderFlow::init((EScript::Namespace *)(lib->getAttribute(LIBRARY_NAME).getValue()));
}