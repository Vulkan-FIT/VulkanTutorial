cmake_minimum_required(VERSION 3.16.0)

project(VulkanTutorial)

set(PACKAGES
    01-deviceList
   )

foreach(pkg ${PACKAGES})
	add_subdirectory(${pkg}/code)
endforeach()
