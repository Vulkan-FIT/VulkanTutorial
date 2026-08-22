set(APP_NAME 3-1-HelloWindow)

project(${APP_NAME})

set(APP_SOURCES
	main.cpp
)

set(APP_INCLUDES
)

set(APP_SHADERS
)

# executable
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES})

# vkg and shaders
include(vkgMacros.cmake)
vkg_configure(${APP_NAME} vkg.hpp vkg.cpp)
vkg_add_shaders(${APP_NAME} "${APP_SHADERS}")

# VulkanWindow
include(VulkanWindow.cmake)
VulkanWindowConfigure(${APP_NAME} VulkanWindow.h VulkanWindow.cpp)

# target
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 20)

# copy DLLs (SDL3.dll, SDL2.dll, glfw3.dll, Qt stuff,...)
if(WIN32)
	if(${GUI_TYPE} STREQUAL "SDL3" AND SDL3_DLL)
		add_custom_command(TARGET ${APP_NAME}
			POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_DLL}" $<TARGET_FILE_DIR:${APP_NAME}>)
	elseif(${GUI_TYPE} STREQUAL "SDL2" AND SDL2_DLL)
		add_custom_command(TARGET ${APP_NAME}
			POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL2_DLL}" $<TARGET_FILE_DIR:${APP_NAME}>)
	elseif(${GUI_TYPE} STREQUAL "GLFW" AND glfw3_DLL)
		add_custom_command(TARGET ${APP_NAME}
			POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${glfw3_DLL}" $<TARGET_FILE_DIR:${APP_NAME}>)
	elseif(${GUI_TYPE} STREQUAL "Qt6")
		add_custom_command(TARGET ${APP_NAME}
			POST_BUILD COMMAND Qt6::windeployqt
					--no-translations  # skip Qt translations
					--no-opengl-sw  # skip software OpenGL
					--no-system-d3d-compiler  # skip D3D stuff
					--no-svg  # skip svg support
					$<TARGET_FILE_DIR:${APP_NAME}>
					COMMENT "Deploying Qt related dependencies...")
	elseif(${GUI_TYPE} STREQUAL "Qt5")
		add_custom_command(TARGET ${APP_NAME}
			POST_BUILD COMMAND "${QT5_WINDEPLOYQT_EXECUTABLE}"
					--no-translations  # skip Qt translations
					--no-widgets  # skip Qt widgets
					--no-opengl-sw  # skip software OpenGL
					--no-angle  # skip software OpenGL (ANGLE)
					$<TARGET_FILE_DIR:${APP_NAME}>
					COMMENT "Deploying Qt related dependencies...")
	endif()
endif()
