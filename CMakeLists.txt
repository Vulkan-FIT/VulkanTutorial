cmake_minimum_required(VERSION 3.16.0)

project(VulkanTutorial)

set(PACKAGES
    1-1-DeviceList
    1-3-DeviceInfo
    1-4-AdvancedInfo
    1-5-LogicalDevice
   )

foreach(pkg ${PACKAGES})
	add_subdirectory(${pkg}/code)
endforeach()

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT 01-DeviceList)
