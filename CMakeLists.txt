cmake_minimum_required(VERSION 3.16.0)

project(VulkanTutorial)

set(PACKAGES
    1-1-DeviceList
    1-3-DeviceInfo
    1-4-AdvancedInfo
    1-5-LogicalDevice
    2-1-CommandSubmission
    2-2-ComputeShader
   )

foreach(pkg ${PACKAGES})
	add_subdirectory(${pkg}/code)
endforeach()

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT 1-1-DeviceList)
