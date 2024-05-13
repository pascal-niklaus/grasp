cmake_minimum_required(VERSION 3.25)
project(Grasp VERSION 0.9 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS  " ${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -std=c++20")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

add_definitions(-DAPP_VERSION="${CMAKE_PROJECT_VERSION}" -DAPP_NAME="${CMAKE_PROJECT_NAME}")

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Widgets)

qt_add_executable(grasp WIN32 MACOSX_BUNDLE
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
  colors.cpp colors.h
  undo.cpp undo.h
  utility.cpp utility.h
  selectkey.cpp selectkey.h
  selectcolor.cpp selectcolor.h
  selectfont.cpp selectfont.h
  doubleedit.cpp doubleedit.h
  intedit.cpp intedit.h
  settings.cpp settings.h
  arrowitem.cpp arrowitem.h
  kromitdash.cpp kromitdash.h
  kromititem.cpp kromititem.h
  kromitpath.cpp kromitpath.h
  kromitsmooth.cpp kromitsmooth.h
  kromitortho.cpp kromitortho.h
  kromitline.cpp kromitline.h
  kromitfader.cpp kromitfader.h
  kromithighlight.cpp kromithighlight.h
  kromitrect.cpp kromitrect.h
  kromittext.cpp kromittext.h
  kromitpixmap.cpp kromitpixmap.h
  kromitarrow.cpp kromitarrow.h
  kromitframe.cpp kromitframe.h
  kromithandle.cpp kromithandle.h
  kromitrotate.cpp kromitrotate.h
  kromitmove.cpp kromitmove.h
)

qt_add_resources(grasp "cursors"
    PREFIX "/cursors"
    FILES ../cursors/text.png
    FILES ../cursors/smooth.png
    FILES ../cursors/rect.png
    FILES ../cursors/line.png
    FILES ../cursors/pen.png
    FILES ../cursors/ortho.png
    FILES ../cursors/fader.png
    FILES ../cursors/highlight.png
    FILES ../cursors/grab.png
)

qt_add_resources(grasp "icons"
    PREFIX "/icons"
    FILES ../icons/settings.png
)

target_link_libraries(grasp PRIVATE
    Qt::Core
    Qt::Gui
    Qt::DBus
)

if((QT_VERSION_MAJOR GREATER 4))
    target_link_libraries(grasp PRIVATE
        Qt::Widgets
    )
endif()

install(TARGETS grasp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
