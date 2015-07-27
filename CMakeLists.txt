cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0017 OLD)

SET( QT_USE_QTCORE TRUE )
SET( QT_USE_QTGUI TRUE )

add_definitions( -Wextra -Wall -pedantic -std=c++11 -I${PROJECT_BINARY_DIR})

# uninstall target
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY )

add_custom_target( uninstall
COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake )

INCLUDE(CMakeDependentOption)
INCLUDE(FindPkgConfig)

pkg_check_modules( GAMMU gammu )

if( NOT GAMMU_FOUND )
	message ( FATAL_ERROR "ERROR: could not find gammu development files" )
endif( NOT GAMMU_FOUND )

add_definitions( -I${GAMMU_INCLUDE_DIR})
include_directories( ${GAMMU_INCLUDE_DIRS} )

find_package( Qt4 REQUIRED QtCore QtGui )

add_definitions( -I${Qt4_INCLUDE_DIR} )
add_definitions( -I${QT_HEADERS_DIR} )

INCLUDE( ${QT_USE_FILE} )
INCLUDE( ${QT_USE_FILE} )

QT4_WRAP_CPP( MOC mainwindow.h )
QT4_WRAP_UI( UI mainwindow.ui )
QT4_ADD_RESOURCES( ICON icon.qrc )

add_executable( ussd-gui main.cpp mainwindow.cpp ${MOC} ${UI} ${ICON} )

set_target_properties( ussd-gui PROPERTIES COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64 -Wextra -Wall -s -fPIE -pthread  -pedantic " )
set_target_properties( ussd-gui PROPERTIES LINK_FLAGS "-pie" )

target_link_libraries( ussd-gui ${QT_LIBRARIES} ${GAMMU_LIBRARIES} )

install( TARGETS ussd-gui RUNTIME DESTINATION bin )

install( FILES ${PROJECT_BINARY_DIR}/ussd-gui.desktop DESTINATION share/applications
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
install( FILES ussd-gui.png DESTINATION /usr/share/icons/hicolor/48x48/apps/ )
install( FILES ussd-gui.png DESTINATION /usr/share/pixmaps )

file( WRITE "${PROJECT_BINARY_DIR}/ussd-gui.desktop"
"#!/usr/bin/env xdg-open
[Desktop Entry]
Comment[en_US]=
Comment=
Exec=${CMAKE_INSTALL_PREFIX}/bin/ussd-gui
GenericName[en_US]=a GUI tool to send ussd codes
GenericName=a GUI tool to send ussd codes
Icon=ussd-gui
MimeType=
Name[en_US]=ussd-gui
Name=ussd-gui
NoDisplay=false
Path=
StartupNotify=true
Terminal=false
TerminalOptions=
Type=Application
X-DBUS-ServiceName=
X-DBUS-StartupType=Unique
X-KDE-SubstituteUID=false
X-KDE-Username=
Categories=Network;X-MandrivaLinux-Internet;\n")
