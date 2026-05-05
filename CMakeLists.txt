cmake_minimum_required(VERSION 3.20)
project(FormatFactory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core Widgets Gui Multimedia MultimediaWidgets
)
qt_standard_project_setup()

qt_add_executable(FormatFactory
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(FormatFactory PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)
# OpenCV 配置（Qt6 & Arch 专用 不报错）
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(FormatFactory PRIVATE ${OpenCV_LIBS})
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)
target_link_libraries(FormatFactory PRIVATE Qt6::Svg)
