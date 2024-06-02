cmake_minimum_required(VERSION 3.25)
project(Grasp VERSION 0.9 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS  " ${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -std=c++20")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

add_definitions(-DAPP_VERSION="${CMAKE_PROJECT_VERSION}" -DAPP_NAME="${CMAKE_PROJECT_NAME}")

# --- keep this
add_compile_options(-fsanitize=address -fsanitize=undefined -fsanitize=leak)
add_link_options(-fsanitize=address -fsanitize=undefined -fsanitize=leak)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Widgets)

qt_add_executable(grasp WIN32 MACOSX_BUNDLE
  grasp.cpp grasp.h
  singletonapplication.h
  verbosity.cpp verbosity.h
  dbus.cpp dbus.h
  signalblocker.h
  screenres.cpp screenres.h
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
  gfxarrowitem.cpp gfxarrowitem.h
  gfxdash.cpp gfxdash.h
  gfxitem.cpp gfxitem.h
  gfxpath.cpp gfxpath.h
  gfxsmooth.cpp gfxsmooth.h
  gfxortho.cpp gfxortho.h
  gfxline.cpp gfxline.h
  gfxfader.cpp gfxfader.h
  gfxhighlight.cpp gfxhighlight.h
  gfxrect.cpp gfxrect.h
  gfxtext.cpp gfxtext.h
  gfxpixmap.cpp gfxpixmap.h
  gfxarrow.cpp gfxarrow.h
  gfxframe.cpp gfxframe.h
  gfxhandle.cpp gfxhandle.h
  gfxrotate.cpp gfxrotate.h
  gfxmove.cpp gfxmove.h
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
    FILES ../icons/grasp.png
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
