set(APP_NAME 2-3-AdjustedMeasurement)

project(${APP_NAME})

set(APP_SOURCES
    main.cpp
    vkg.cpp
   )

set(APP_INCLUDES
    vkg.h
   )

set(APP_SHADERS
    performance.comp
   )

# executable
include(vkgMacros.cmake)
vkg_add_shaders("${APP_SHADERS}" APP_SHADER_DEPS)
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES} ${APP_SHADER_DEPS})

# target
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 20)
target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
