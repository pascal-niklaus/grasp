cmake_minimum_required(VERSION 3.16)
project(kromit VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS  " ${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -std=c++20")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Widgets)

qt_add_executable(kromit WIN32 MACOSX_BUNDLE
  singletonapplication.h
  drawingscene.cpp drawingscene.h
  drawingview.cpp drawingscene.h
  xy.cpp xy.h
    kromitpath.cpp kromitpath.h
    main.cpp
    mainwindow.cpp mainwindow.h
)
target_link_libraries(kromit PRIVATE
    Qt::Core
    Qt::Gui
)

if((QT_VERSION_MAJOR GREATER 4))
    target_link_libraries(kromit PRIVATE
        Qt::Widgets
    )
endif()

install(TARGETS kromit
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
