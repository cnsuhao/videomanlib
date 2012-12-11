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
 GLWindow.h )

QT4_WRAP_CPP(MOC_SOURCE_FILES ${MOC_FILES}  )
SOURCE_GROUP( "Generated Files" FILES ${MOC_SOURCE_FILES} )

ADD_EXECUTABLE( exampleVMwithQT VMwithQT.cpp GLWindow.cpp GLWindow.h ${MOC_SOURCE_FILES} )
TARGET_LINK_LIBRARIES( exampleVMwithQT VideoMan ${QT_LIBRARIES} ${OPENGL_LIBRARIES} )
set_target_properties( exampleVMwithQT PROPERTIES FOLDER "examples")
add_dependencies( exampleVMwithQT VideoMan )
