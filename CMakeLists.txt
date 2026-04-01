cmake_minimum_required(VERSION 3.16)
project(cube LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL Gui)
set(CMAKE_AUTOUIC OFF)

set(PROJECT_SOURCES
    main.cpp
    cubewindow.cpp cubewindow.h
    ui_widgets.cpp ui_widgets.h
)

qt_add_executable(cube ${PROJECT_SOURCES}
    shaders/vertex_shader.vsh
    shaders/fragment_shader.fsh
    ui_widgets.h
    ui_widgets.cpp
    CubeUIManager.h
    CubeUIManager.cpp
    lamp.qrc
    tools.h
    spotlight.qrc
    dir_light.qrc
    textures.qrc

)

target_link_libraries(cube PRIVATE
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::Gui
)

set_target_properties(cube PROPERTIES WIN32_EXECUTABLE TRUE)
