# Try to find PADrend. Once done, this will define:
#
#   PADREND_FOUND - variable which returns the result of the search
#   PADREND_INCLUDE_DIRS - list of include directories
#   PADREND_LIBRARIES - options for the linker

#=============================================================================
#
# This work is licensed under the Creative Commons Attribution-ShareAlike 4.0 International License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/4.0/.
# Author: Sascha Brandt (sascha@brandt.graphics)
#
#=============================================================================


# Find PADrend include dir
find_path(PADREND_INCLUDE_DIR
	MinSG/MinSG.h
	PATHS
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../modules
	${CMAKE_CURRENT_SOURCE_DIR}/../../modules
	/upb/groups/fg-madh/public/share/padrend/include
)

set(PADREND_DEFAULT_LIBRARY_PATHS 
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

# Find Geometry
if(TARGET Geometry)
	# The library is included as a build target
	message("Found target Geometry")
	set(PADREND_GEOMETRY_LIBRARY Geometry)
else()
	find_library(PADREND_GEOMETRY_LIBRARY
		libGeometry.so Geometry
		PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/../build/modules/Geometry
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/modules/Geometry
		${PADREND_DEFAULT_LIBRARY_PATHS}
	)
endif()

# Find Rendering
if(TARGET Rendering)
	# The library is included as a build target
	message("Found target Rendering")
	set(PADREND_RENDERING_LIBRARY Rendering)
else()
	find_library(PADREND_RENDERING_LIBRARY
		libRendering.so Rendering
		PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/../build/modules/Rendering
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/modules/Rendering
		${PADREND_DEFAULT_LIBRARY_PATHS}
	)
endif()

# Find Util
if(TARGET Util)
	# The library is included as a build target
	message("Found target Util")
	set(PADREND_UTIL_LIBRARY Util)
else()
	find_library(PADREND_UTIL_LIBRARY
		libUtil.so Util
		PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/../build/modules/Util
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/modules/Util
		${PADREND_DEFAULT_LIBRARY_PATHS}
	)
endif()

# Find MinSG
if(TARGET MinSG)
	# The library is included as a build target
	message("Found target MinSG")
	set(PADREND_MINSG_LIBRARY MinSG)
else()
	find_library(PADREND_MINSG_LIBRARY
		libMinSG.so MinSG
		PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/../build/modules/MinSG
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/modules/MinSG
		${PADREND_DEFAULT_LIBRARY_PATHS}
	)
endif()

# Find E_Geometry
if(TARGET E_Geometry)
	# The library is included as a build target
	message("Found target E_Geometry")
	set(PADREND_E_GEOMETRY_LIBRARY E_Geometry)
else()
	find_library(PADREND_E_GEOMETRY_LIBRARY
		libE_Geometry.so E_Geometry
		PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/../build/modules/E_Geometry
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/modules/E_Geometry
		${PADREND_DEFAULT_LIBRARY_PATHS}
	)
endif()

# Find E_Rendering
if(TARGET E_Rendering)
	# The library is included as a build target
	message("Found target E_Rendering")
	set(PADREND_E_RENDERING_LIBRARY E_Rendering)
else()
	find_library(PADREND_E_RENDERING_LIBRARY
		libE_Rendering.so E_Rendering
		PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/../build/modules/E_Rendering
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/modules/E_Rendering
		${PADREND_DEFAULT_LIBRARY_PATHS}
	)
endif()

# Find E_Util
if(TARGET E_Util)
	# The library is included as a build target
	message("Found target E_Util")
	set(PADREND_E_UTIL_LIBRARY E_Util)
else()
	find_library(PADREND_E_UTIL_LIBRARY
		libE_Util.so E_Util
		PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/../build/modules/E_Util
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/modules/E_Util
		${PADREND_DEFAULT_LIBRARY_PATHS}
	)
endif()

# Find E_MinSG
if(TARGET E_MinSG)
	# The library is included as a build target
	message("Found target E_MinSG")
	set(PADREND_E_MINSG_LIBRARY E_MinSG)
else()
	find_library(PADREND_E_MINSG_LIBRARY
		libE_MinSG.so E_MinSG
		PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/../build/modules/E_MinSG
		${CMAKE_CURRENT_SOURCE_DIR}/../../build/modules/E_MinSG
		${PADREND_DEFAULT_LIBRARY_PATHS}
	)
endif()

set(PADREND_INCLUDE_DIRS ${PADREND_INCLUDE_DIR})
set(PADREND_LIBRARIES 
	${PADREND_GEOMETRY_LIBRARY}
	${PADREND_RENDERING_LIBRARY}
	${PADREND_UTIL_LIBRARY}
	${PADREND_MINSG_LIBRARY}
	${PADREND_E_GEOMETRY_LIBRARY}
	${PADREND_E_RENDERING_LIBRARY}
	${PADREND_E_UTIL_LIBRARY}
	${PADREND_E_MINSG_LIBRARY}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PADrend DEFAULT_MSG
	PADREND_INCLUDE_DIR
	PADREND_GEOMETRY_LIBRARY
	PADREND_RENDERING_LIBRARY
	PADREND_UTIL_LIBRARY
	PADREND_MINSG_LIBRARY
	PADREND_E_GEOMETRY_LIBRARY
	PADREND_E_RENDERING_LIBRARY
	PADREND_E_UTIL_LIBRARY
	PADREND_E_MINSG_LIBRARY
)

mark_as_advanced(
	PADREND_INCLUDE_DIR
	PADREND_GEOMETRY_LIBRARY
	PADREND_RENDERING_LIBRARY
	PADREND_UTIL_LIBRARY
	PADREND_MINSG_LIBRARY
	PADREND_E_GEOMETRY_LIBRARY
	PADREND_E_RENDERING_LIBRARY
	PADREND_E_UTIL_LIBRARY
)

# Create imported target PADrend::PADrend
if(PADREND_FOUND AND NOT TARGET PADrend::PADrend)
	add_library(PADrend::PADrend INTERFACE IMPORTED)
	target_link_libraries(PADrend::PADrend INTERFACE ${PADREND_LIBRARIES})
	target_include_directories(PADrend::PADrend INTERFACE ${PADREND_INCLUDE_DIR})
	if(MSVC)
		set_target_properties(PADrend::PADrend PROPERTIES INTERFACE_COMPILE_DEFINITIONS "GEOMETRYAPI=__declspec(dllimport);RENDERINGAPI=__declspec(dllimport);UTILAPI=__declspec(dllimport);MINSG_DEPRECATED=__declspec(deprecated);MINSGAPI=__declspec(dllimport);EGEOMETRYAPI=__declspec(dllimport);ERENDERINGAPI=__declspec(dllimport);EUTILAPI=__declspec(dllimport);EMINSGAPI=__declspec(dllimport);")
	else()
		set_target_properties(PADrend::PADrend PROPERTIES INTERFACE_COMPILE_DEFINITIONS "GEOMETRYAPI=;RENDERINGAPI=;UTILAPI=;MINSG_DEPRECATED=;MINSGAPI=;EGEOMETRYAPI=;ERENDERINGAPI=;EUTILAPI=;EMINSGAPI=;")
	endif()
endif()