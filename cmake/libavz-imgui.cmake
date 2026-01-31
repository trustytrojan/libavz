# Central ImGui dependency configuration for libavz
# This module handles fetching and configuring ImGui and ImGui-SFML
# Usage: include(${CMAKE_SOURCE_DIR}/cmake/libavz-imgui.cmake)
# Then link with: target_link_libraries(your_target PUBLIC ImGui-SFML::ImGui-SFML)
# And define: target_compile_definitions(your_target PUBLIC LIBAVZ_IMGUI)

if(NOT LIBAVZ_USE_IMGUI)
	return()
endif()

# Fetch ImGui
FetchContent_Declare(imgui URL https://github.com/ocornut/imgui/archive/v1.91.8.tar.gz)
FetchContent_MakeAvailable(imgui)

# ImGui-SFML requires SFML to be available
find_package(SFML COMPONENTS Graphics QUIET)
if(NOT SFML_FOUND)
	# SFML will be fetched by gfx library, so we can't use ImGui-SFML
	# Set a flag to indicate that ImGui-SFML couldn't be initialized
	set(LIBAVZ_IMGUI_AVAILABLE FALSE CACHE INTERNAL "ImGui-SFML is available")
	message(WARNING "[libavz-imgui] SFML not found; ImGui-SFML will not be available")
	return()
endif()

# Set up imgui-sfml (depends on imgui being downloaded manually)
if(NOT WIN32)
	# imgui-sfml's cmake sets SFML_STATIC_LIBRARIES based on BUILD_SHARED_LIBS
	set(BUILD_SHARED_LIBS ON)
endif()
set(IMGUI_DIR ${imgui_SOURCE_DIR})
FetchContent_Declare(imgui-sfml URL https://github.com/SFML/imgui-sfml/archive/v3.0.tar.gz)
FetchContent_MakeAvailable(imgui-sfml)

set(LIBAVZ_IMGUI_AVAILABLE TRUE CACHE INTERNAL "ImGui-SFML is available")
