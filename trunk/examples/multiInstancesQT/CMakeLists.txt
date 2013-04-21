SET( QT_USE_QTOPENGL 1 )
FIND_PACKAGE( Qt REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
IF( QT_FOUND )
 	INCLUDE( ${QT_USE_FILE} )
ELSE( QT_FOUND )
 	MESSAGE( FATAL_ERROR "This application requires Qt. One of these components is missing. Please verify configuration")
ENDIF( QT_FOUND )

INCLUDE_DIRECTORIES( ${QT_INCLUDES} ${OPENGL_INCLUDE_DIR})

SET( MOC_FILES
 GLWindow.h
 mainWindow.h
 cameraSelector.h )

QT4_WRAP_CPP(MOC_SOURCE_FILES ${MOC_FILES}  )
SOURCE_GROUP( "Generated Files" FILES ${MOC_SOURCE_FILES} )

ADD_EXECUTABLE( exQtMultiInstances main.cpp GLWindow.cpp GLWindow.h mainWindow.cpp mainWindow.h ${MOC_SOURCE_FILES} cameraSelector.cpp cameraSelector.h )
TARGET_LINK_LIBRARIES( exQtMultiInstances VideoMan ${QT_LIBRARIES} ${OPENGL_LIBRARIES} )
set_target_properties( exQtMultiInstances PROPERTIES FOLDER "examples")
add_dependencies( exQtMultiInstances VideoMan )
