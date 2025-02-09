set(APP_NAME 1-5-LogicalDevice)

project(${APP_NAME})

set(APP_SOURCES
    main.cpp
    vkg.cpp
   )

set(APP_INCLUDES
    vkg.h
   )

# target
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES})
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 20)
