# required by CMake
cmake_minimum_required(VERSION 3.1.0)

# set the project name
project(RenderFlow)

# ensures that the project is build with C++11 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# add C++ source files to the project
add_library(${PROJECT_NAME} SHARED
    src/Main.cpp
    src/ELibRenderFlow.cpp
    # add your own .cpp files here
	src/PythonRender.cpp
)

find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PYTHON_LIBRARIES} Python3::NumPy)
# target_link_libraries(${PROJECT_NAME} PRIVATE Python3::NumPy)

# TODO Unix/MacOS
# TODO bundle tensorflow.dll
# add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} PRIVATE tensorflow)
configure_file(${CMAKE_SOURCE_DIR}/lib/tensorflow.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tensorflow.dll COPYONLY)


# pass the project name to the Main.cpp file in form of the define 'LIBRARY_NAME'
set_source_files_properties(src/Main.cpp PROPERTIES COMPILE_DEFINITIONS LIBRARY_NAME="${PROJECT_NAME}")

# make sure that the built .dll or .so file is placed into the 'build' or 'bin' directory
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")


# defines dllexport macro required by windows dll's, e.g., EXAMPLEPROJECTAPI
string(TOUPPER "${PROJECT_NAME}API" PROJECT_API)
if(MSVC)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
	target_compile_definitions(${PROJECT_NAME} PRIVATE "${PROJECT_API}=__declspec(dllexport)")
	target_compile_definitions(${PROJECT_NAME} INTERFACE "${PROJECT_API}=__declspec(dllimport)")
else()
	target_compile_definitions(${PROJECT_NAME} PRIVATE "${PROJECT_API}=")
	target_compile_definitions(${PROJECT_NAME} INTERFACE "${PROJECT_API}=")
endif()

# EXAMPLEPROJECTAPI alias for main file
set_property(SOURCE src/Main.cpp APPEND PROPERTY COMPILE_DEFINITIONS LIBRARY_API=${PROJECT_API})


# --- Set up dependencies to the PADrend libraries ---

# add ./cmake to module search path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# find PADrend libraries
find_package(PADrend)
if(PADREND_FOUND)
	target_link_libraries(${PROJECT_NAME} PRIVATE PADrend::PADrend)
endif()

# find EScript library
find_package(EScript)
if(ESCRIPT_FOUND)
	target_link_libraries(${PROJECT_NAME} PRIVATE EScript::EScript)
endif()
