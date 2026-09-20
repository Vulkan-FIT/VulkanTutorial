set(APP_NAME 1-3-DeviceInfo)

project(${APP_NAME})

set(APP_SOURCES
	main.cpp
)

set(APP_INCLUDES
)

# executable
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES})

# vkg and shaders
include(vkgMacros.cmake)
vkg_configure(${APP_NAME} vkg.hpp vkg.cpp)

# target
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 20)
