cmake_minimum_required(VERSION 3.16)
project(imgedit VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)
find_package(Fontconfig REQUIRED)

set(IMGEDIT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h

    src/core/imagedocument.cpp
    src/core/imagedocument.h
    src/core/operation.cpp
    src/core/operation.h
    src/core/undostack.cpp
    src/core/undostack.h
    src/core/layerstack.cpp
    src/core/layerstack.h

    src/filters/filter.cpp
    src/filters/filter.h

    src/tools/tool.cpp
    src/tools/tool.h
    src/tools/brush.cpp
    src/tools/brush.h

    src/ui/canvasview.cpp
    src/ui/canvasview.h
    src/ui/toolbar.cpp
    src/ui/toolbar.h
    src/ui/propertiespanel.cpp
    src/ui/propertiespanel.h
    src/ui/colorpicker.cpp
    src/ui/colorpicker.h

    src/utils/iconengine.cpp
    src/utils/iconengine.h
    src/utils/fontawesome.cpp
    src/utils/fontawesome.h

    src/project/projectfile.cpp
    src/project/projectfile.h
)

set(IMGEDIT_HEADERS
    src/mainwindow.h
    src/core/imagedocument.h
    src/core/operation.h
    src/core/undostack.h
    src/core/layerstack.h
    src/filters/filter.h
    src/tools/tool.h
    src/tools/brush.h
    src/ui/canvasview.h
    src/ui/toolbar.h
    src/ui/propertiespanel.h
    src/ui/colorpicker.h
    src/utils/iconengine.h
    src/utils/fontawesome.h
    src/project/projectfile.h
)

add_executable(imgedit ${IMGEDIT_SOURCES} ${IMGEDIT_HEADERS})

target_link_libraries(imgedit Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Fontconfig::Fontconfig)

target_compile_options(imgedit PRIVATE -Wall -Wextra -Wno-unused-parameter)

install(TARGETS imgedit DESTINATION bin)