set(APP_NAME 2-5-TimestampQueries)

project(${APP_NAME})

set(APP_SOURCES
	main.cpp
)

set(APP_INCLUDES
)

set(APP_SHADERS
	performance.comp
)

# executable
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES})

# vkg and shaders
include(vkgMacros.cmake)
vkg_configure(${APP_NAME} vkg.hpp vkg.cpp)
vkg_add_shaders(${APP_NAME} "${APP_SHADERS}")

# target
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 20)
