
macro(vkg_find_sources vkg_INCLUDE_VARIABLE vkg_SOURCE_VARIABLE)
	find_file(${vkg_INCLUDE_VARIABLE}
		NAMES
			vkg.h
		PATHS
			${CURRENT_SOURCE_DIR}
			/usr/include
			/usr/local/include
	)
	find_file(${vkg_SOURCE_VARIABLE}
		NAMES
			vkg.cpp
		PATHS
			${CURRENT_SOURCE_DIR}
			/usr/include
			/usr/local/include
	)
endmacro()


macro(vkg_find_glslangValidator)

	# glslangValidator executable
	find_program(vkg_GLSLANG_VALIDATOR_EXECUTABLE
		NAMES
			glslangValidator
		PATHS
			"$ENV{VULKAN_SDK}/bin"
			"$ENV{VULKAN_SDK}/bin32"
			/usr/bin
			/usr/local/bin
	)

	# vkg::glslangValidator target
	if(vkg_GLSLANG_VALIDATOR_EXECUTABLE AND NOT TARGET vkg::glslangValidator)
		add_executable(vkg::glslangValidator IMPORTED)
		set_property(TARGET vkg::glslangValidator PROPERTY IMPORTED_LOCATION "${vkg_GLSLANG_VALIDATOR_EXECUTABLE}")
	endif()

endmacro()


# add_shaders macro to convert GLSL shaders to spir-v
# and creates depsList containing name of files that should be included in the list of source files
macro(vkg_add_shaders nameList depsList)

	vkg_find_glslangValidator()
	if(NOT TARGET vkg::glslangValidator)
		message(FATAL_ERROR "vkg: glslangValidator executable not found.")
	endif()

	foreach(name ${nameList})
		get_filename_component(directory ${name} DIRECTORY)
		if(directory)
			file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${directory}")
		endif()
		add_custom_command(COMMENT "Converting ${name} to spir-v..."
		                   MAIN_DEPENDENCY ${name}
		                   OUTPUT ${name}.spv
		                   COMMAND ${vkg_GLSLANG_VALIDATOR_EXECUTABLE} --target-env vulkan1.0 -x ${CMAKE_CURRENT_SOURCE_DIR}/${name} -o ${name}.spv)
		source_group("Shaders" FILES ${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}.spv)
		list(APPEND ${depsList} ${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}.spv)
	endforeach()

endmacro()
