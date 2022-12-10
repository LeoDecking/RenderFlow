# Try to find EScript. Once done, this will define:
#
#   ESCRIPT_FOUND - variable which returns the result of the search
#   ESCRIPT_INCLUDE_DIRS - list of include directories
#   ESCRIPT_LIBRARIES - options for the linker

#=============================================================================
#
# This work is licensed under the Creative Commons Attribution-ShareAlike 4.0 International License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/4.0/.
# Author: Sascha Brandt (sascha@brandt.graphics)
#
#=============================================================================


# Find EScript include dir
find_path(ESCRIPT_INCLUDE_DIR
	EScript/EScript.h
	PATHS
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../modules/EScript
	${CMAKE_CURRENT_SOURCE_DIR}/../../modules/EScript
	/upb/groups/fg-madh/public/share/padrend/include
)
# Find EScript
if(TARGET EScript)
	# The library is included as a build target
	message("Found target EScript")
	set(ESCRIPT_LIBRARY EScript)
else()
	find_library(ESCRIPT_LIBRARY
		libEScript.so EScript
		PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/../build/modules/EScript
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/modules/EScript
		${CMAKE_CURRENT_SOURCE_DIR}/../bin
		${CMAKE_CURRENT_SOURCE_DIR}/../lib
		${CMAKE_CURRENT_SOURCE_DIR}/../build
		${CMAKE_CURRENT_SOURCE_DIR}/../../build
		${CMAKE_CURRENT_SOURCE_DIR}/../build/lib
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/lib
		${CMAKE_CURRENT_SOURCE_DIR}/../build/$<CONFIG>
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/$<CONFIG>
		${CMAKE_CURRENT_SOURCE_DIR}/../build/lib/$<CONFIG>
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/lib/$<CONFIG>
		/upb/groups/fg-madh/public/share/padrend/linux/lib
	)
endif()

set(ESCRIPT_INCLUDE_DIRS ${ESCRIPT_INCLUDE_DIR})
set(ESCRIPT_LIBRARIES ${ESCRIPT_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EScript DEFAULT_MSG
	ESCRIPT_INCLUDE_DIR
	ESCRIPT_LIBRARY
)

mark_as_advanced(
	ESCRIPT_INCLUDE_DIR
	ESCRIPT_LIBRARY
)

if(ESCRIPT_FOUND AND NOT TARGET EScript::EScript)
	add_library(EScript::EScript INTERFACE IMPORTED)
	target_link_libraries(EScript::EScript INTERFACE ${ESCRIPT_LIBRARY})
	target_include_directories(EScript::EScript INTERFACE ${ESCRIPT_INCLUDE_DIR})
	if(MSVC)
		set_target_properties(EScript::EScript PROPERTIES INTERFACE_COMPILE_DEFINITIONS "ESCRIPTAPI=__declspec(dllimport)")
	else()
		set_target_properties(EScript::EScript PROPERTIES INTERFACE_COMPILE_DEFINITIONS "ESCRIPTAPI=")
	endif()
endif()