# SPDX-FileCopyrightText: 2025-2026 PCJohn (Jan Pečiva, peciva@fit.vut.cz)
#
# SPDX-License-Identifier: MIT-0


# configure target to use vkg
macro(vkg_configure targetName vkg_include vkg_source)
	target_sources(${targetName} PRIVATE ${vkg_include} ${vkg_source})
endmacro()


macro(vkg_find_glslang)

	# glslang executable
	find_program(vkg_GLSLANG_EXECUTABLE
		NAMES
			glslang glslangValidator
		PATHS
			"$ENV{VULKAN_SDK}/bin"
			"$ENV{VULKAN_SDK}/bin32"
			/usr/bin
			/usr/local/bin
	)

	# vkg::glslang target
	if(vkg_GLSLANG_EXECUTABLE AND NOT TARGET vkg::glslang)
		add_executable(vkg::glslang IMPORTED)
		set_property(TARGET vkg::glslang PROPERTY IMPORTED_LOCATION "${vkg_GLSLANG_EXECUTABLE}")
	endif()

endmacro()


# add_shaders macro converts GLSL shaders to spir-v
macro(vkg_add_shaders targetName glslFileList)

	vkg_find_glslang()
	if(NOT vkg_GLSLANG_EXECUTABLE)
		message(FATAL_ERROR "vkg: glslang executable not found.")
	endif()

	foreach(name ${glslFileList})
		get_filename_component(directory ${name} DIRECTORY)
		if(directory)
			file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${directory}")
		endif()
		add_custom_command(COMMENT "Converting ${name} to spir-v..."
		                   MAIN_DEPENDENCY ${name}
		                   OUTPUT ${name}.spv
		                   COMMAND ${vkg_GLSLANG_EXECUTABLE} --target-env vulkan1.0 -x ${CMAKE_CURRENT_SOURCE_DIR}/${name} -o ${name}.spv)
		source_group("Shaders" FILES ${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}.spv)
		target_sources(${targetName} PRIVATE ${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}.spv)
	endforeach()
	target_include_directories(${targetName} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

endmacro()
