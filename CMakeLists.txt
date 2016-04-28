cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0017 OLD)

project(ussd-gui)

SET( QT_USE_QTCORE TRUE )
SET( QT_USE_QTGUI TRUE )

add_definitions( -Wextra -Wall -pedantic -std=c++14 -I${PROJECT_BINARY_DIR} )

set( PGR_VERSION "1.2.0" )

file( WRITE ${PROJECT_BINARY_DIR}/version.h "
#ifndef VERSION
#define VERSION \"${PGR_VERSION}\"
#endif
\n" )

# uninstall target
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY )

add_custom_target( uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake )

INCLUDE( CMakeDependentOption )
INCLUDE( FindPkgConfig )

pkg_check_modules( GAMMU gammu )

if( NOT GAMMU_FOUND )
	message( FATAL_ERROR "ERROR: could not find gammu development files" )
endif( NOT GAMMU_FOUND )

include_directories( ${GAMMU_INCLUDE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party )
include_directories( ${GAMMU_INCLUDE_DIRS} )

include_directories( ${PROJECT_BINARY_DIR} )

if( QT5 )
	find_package( Qt5Widgets REQUIRED )
	find_package( Qt5Core REQUIRED )

	include_directories( ${Qt5Widgets_INCLUDE_DIRS} )
	add_definitions( ${Qt5Widgets_DEFINITIONS} )

	QT5_WRAP_CPP( MOC src/mainwindow.h src/task.h src/favorites.h src/dialogmsg.h )
	QT5_WRAP_UI( UI src/mainwindow.ui src/favorites.ui src/dialogmsg.ui )
	QT5_ADD_RESOURCES( ICON icons/icons.qrc )
else()
	find_package( Qt4 REQUIRED QtCore QtGui )

	add_definitions( -I${Qt4_INCLUDE_DIR} )
	add_definitions( -I${QT_HEADERS_DIR} )

	INCLUDE( ${QT_USE_FILE} )
	INCLUDE( ${QT_USE_FILE} )

	QT4_WRAP_CPP( MOC src/mainwindow.h src/task.h src/favorites.h src/dialogmsg.h)
	QT4_WRAP_UI( UI src/mainwindow.ui src/favorites.ui src/dialogmsg.ui )
	QT4_ADD_RESOURCES( ICON icons/icons.qrc )
endif()

add_executable( ussd-gui src/utility.cpp src/favorites.cpp src/tablewidget.cpp src/internal.cpp src/libgammu.cpp
src/main.cpp src/gsm.cpp src/mainwindow.cpp src/dialogmsg.cpp 3rd_party/qgsmcodec.cpp ${MOC} ${UI} ${ICON} )

set_target_properties( ussd-gui PROPERTIES COMPILE_FLAGS "-Wextra -Wall -s -fPIE -pthread  -pedantic" )
set_target_properties( ussd-gui PROPERTIES LINK_FLAGS "-pie" )

if( QT5 )
	target_link_libraries( ussd-gui ${Qt5Widgets_LIBRARIES} ${Qt5Core_LIBRARIES} ${GAMMU_LIBRARIES} )
else()
	target_link_libraries( ussd-gui ${QT_LIBRARIES} ${GAMMU_LIBRARIES} )
endif()

if( "${DATA_INSTALL_DIR}" STREQUAL "" )
	set( DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share" )
endif()

file( WRITE "${PROJECT_BINARY_DIR}/language_path.h" "\n#define LANGUAGE_FILE_PATH \"${DATA_INSTALL_DIR}/ussd-gui/translations.qm/\"" )

install( TARGETS ussd-gui RUNTIME DESTINATION bin )

install( FILES ${PROJECT_BINARY_DIR}/ussd-gui.desktop DESTINATION share/applications
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
install( FILES icons/ussd-gui.png DESTINATION /usr/share/icons/hicolor/48x48/apps/ )
install( FILES icons/ussd-gui.png DESTINATION /usr/share/pixmaps )

#install( FILES translations.qm/swahili.qm   DESTINATION "${DATA_INSTALL_DIR}/ussd-gui/translations.qm/" )

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
Categories=Network;X-MandrivaLinux-Internet;\n" )
