cmake_minimum_required(VERSION 3.16.0)

project(VulkanTutorial)

set(PACKAGES
    01-DeviceList
    03-DeviceInfo
   )

foreach(pkg ${PACKAGES})
	add_subdirectory(${pkg}/code)
endforeach()

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT 01-DeviceList)
