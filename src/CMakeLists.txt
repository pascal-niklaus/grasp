cmake_minimum_required(VERSION 3.25)
project(Kromit VERSION 0.9 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS  " ${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -std=c++20")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

add_definitions(-DAPP_VERSION="${CMAKE_PROJECT_VERSION}" -DAPP_NAME="${CMAKE_PROJECT_NAME}")

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Widgets)

qt_add_executable(kromit WIN32 MACOSX_BUNDLE
  kromit.cpp kromit.h
  singletonapplication.h
  dbus.cpp dbus.h
  main.cpp
  mainwindow.cpp mainwindow.h
  drawingscene.cpp drawingscene.h
  drawingview.cpp drawingscene.h
  xy.cpp xy.h
  path.cpp path.h
  tools.cpp tools.h
  undo.h
  arrowitem.cpp arrowitem.h
  kromititem.cpp kromititem.h
  kromitpath.cpp kromitpath.h
  kromitline.cpp kromitline.h
  kromitarrow.cpp kromitarrow.h
  kromitframe.cpp kromitframe.h
  kromithandle.cpp kromithandle.h
  kromitrotate.cpp kromitrotate.h
  kromitmove.cpp kromitmove.h
)

target_link_libraries(kromit PRIVATE
    Qt::Core
    Qt::Gui
    Qt::DBus
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
