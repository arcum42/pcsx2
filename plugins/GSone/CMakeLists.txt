# Check that people use the good file
if(NOT TOP_CMAKE_WAS_SOURCED)
    message(FATAL_ERROR "
    You did not 'cmake' the good CMakeLists.txt file. Use the one in the top dir.
    It is advice to delete all wrongly generated cmake stuff => CMakeFiles & CMakeCache.txt")
endif()


# plugin name
set(Output GSone)
set(GSnullFinalFlags "")

# GSone sources
set(GSoneSources
	GifTransfer.cpp
	GS.cpp
	GSLog.cpp
	Registers.cpp)

# GSone headers
set(GSoneHeaders
	GifTransfer.h
	GS.h
	Registers.h)
	
# GSone Vulkan sources
set(GSoneVulkanSources
	Vulkan/GSVulkan.cpp)

# GSone Vulkan headers
set(GSoneVulkanHeaders
	Vulkan/GSVulkan.h)
	
# GSone Linux sources
set(GSoneLinuxSources
	Linux/Config.cpp
	Linux/GSLinux.cpp
	Linux/Linux.cpp)

# GSone Linux headers
set(GSoneLinuxHeaders
	Linux/Config.h
	Linux/GSLinux.h
	Linux/Linux.h)

# GSone null sources
set(GSonenullSources
	null/GSnull.cpp)

# GSone null headers
set(GSonenullHeaders
	null/GSnull.h)

# GSone Windows sources
set(GSoneWindowsSources
	Windows/Config.cpp
	Windows/GS.def
	Windows/GS.rc
	Windows/GSwin.cpp
	Windows/Win32.cpp)

# GSone Windows headers
set(GSoneWindowsHeaders
	)

set(GSoneFinalSources
	${GSoneSources}
	${GSoneHeaders}
	${GSonenullSources}
	${GSonenullHeaders}
	${GSoneLinuxSources}
	${GSoneLinuxHeaders}
	${GSoneVulkanSources}
	${GSoneVulkanHeaders}
)

set(GSoneFinalLibs
	${GTK2_LIBRARIES}
	${X11_LIBRARIES}
	${Vulkan_LIBRARIES}
    ${GLM_LIBRARIES}
)

add_pcsx2_plugin(${Output} "${GSoneFinalSources}" "${GSoneFinalLibs}" "${GSoneFinalFlags}")
