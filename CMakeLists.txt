cmake_minimum_required(VERSION 3.16)

# Project configuration
project(qtprocessmonitor
    VERSION 1.0.0
    DESCRIPTION "Qt Application for Process Monitoring and Management"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Xml
)

# Automatically handle Qt's MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    a_settingsclass.cpp
    a_process.cpp
)

# Header files
set(HEADERS
    mainwindow.h
    a_settingsclass.h
    a_process.h
)

# Create executable
add_executable(qtprocessmonitor
    ${SOURCES}
    ${HEADERS}
)

# Link Qt libraries
target_link_libraries(qtprocessmonitor
    Qt6::Core
    Qt6::Widgets
    Qt6::Xml
)

# Set output directory
set_target_properties(qtprocessmonitor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy config file to build directory
configure_file(
    "${CMAKE_SOURCE_DIR}/config.xml"
    "${CMAKE_BINARY_DIR}bin/config.xml"
    COPYONLY
)

# Platform-specific settings
if(WIN32)
    # Windows specific settings
    set_target_properties(qtprocessmonitor PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(UNIX AND NOT APPLE)
    # Linux specific settings
    target_compile_definitions(qtprocessmonitor PRIVATE LINUX_BUILD)

    # Install rules for Linux
    install(TARGETS qtprocessmonitor
        RUNTIME DESTINATION bin
    )

    install(FILES config.xml
        DESTINATION bin
    )
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(qtprocessmonitor PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Debug/Release configurations
target_compile_definitions(qtprocessmonitor PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:RELEASE_BUILD>
)

message(STATUS "CMake Configuration Complete")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Qt Version: ${Qt6_VERSION}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

# Build instructions
message(STATUS "")
message(STATUS "Build Instructions:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make -j$(nproc)")
message(STATUS "  ./bin/qtprocessmonitor")
message(STATUS "")
