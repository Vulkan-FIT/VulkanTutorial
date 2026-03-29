set(APP_NAME 2-7-ArchitectureInfo-cpp)

project(${APP_NAME})

set(APP_SOURCES
    main.cpp
    cpuInfo.cpp
   )

set(APP_INCLUDES
    cpuInfo.h
   )

# executable
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES})

# target
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 20)
