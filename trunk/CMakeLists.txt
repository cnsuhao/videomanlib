cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
     cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

PROJECT( VideoMan )

SET ( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules )
SET ( CMAKE_DEBUG_POSTFIX d )

SET ( LIBRARY_OUTPUT_PATH ${VideoMan_BINARY_DIR}/bin/ CACHE PATH "Single output directory for building all libraries." FORCE )
SET ( EXECUTABLE_OUTPUT_PATH ${VideoMan_BINARY_DIR}/bin/ CACHE PATH "Single output directory for building all executables." FORCE )

set_property(GLOBAL PROPERTY USE_FOLDERS On)

INCLUDE_DIRECTORIES( include )

option(BUILD_OPENGL_RENDERER "Use OpenGL Renderer" ON)
IF ( BUILD_OPENGL_RENDERER )
	FIND_PACKAGE( OpenGL )
	add_definitions(-DVM_OGLRenderer)
	INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR} )
ENDIF ( BUILD_OPENGL_RENDERER )

LINK_DIRECTORIES(
)

SET( VideoMan_SRCS
	control/VideoManControl.cpp
	include/VideoManControl.h
	control/VideoManControlPrivate.h
	control/VideoManFactory.cpp
	control/VideoManFactory.h
	control/VideoManInputFormat.cpp
	include/VideoManInputFormat.h	
)

SET( VideoMan_SRCS_INPUTS
	control/inputs/UserInput.cpp
	control/inputs/UserInput.h
	control/inputs/VideoManInputController.cpp
	include/VideoManInputController.h	
	include/VideoInput.h
)

SET( VideoMan_SRCS_RENDERER
	control/renderers/VideoManRenderer.cpp
	control/renderers/VideoManRenderer.h
)

IF ( BUILD_OPENGL_RENDERER )
	SET( VideoMan_SRCS_RENDERER
		${VideoMan_SRCS_RENDERER}
		control/renderers/VideoManRendererOGL.cpp
		control/renderers/VideoManRendererOGL.h
	)
ENDIF ( BUILD_OPENGL_RENDERER )

SOURCE_GROUP(inputs FILES ${VideoMan_SRCS_INPUTS})
SOURCE_GROUP(renderer FILES ${VideoMan_SRCS_RENDERER})

ADD_LIBRARY( VideoMan SHARED
  ${VideoMan_SRCS}
  ${VideoMan_SRCS_INPUTS}
  ${VideoMan_SRCS_RENDERER}
)
SET_PROPERTY(TARGET VideoMan PROPERTY LINK_INTERFACE_LIBRARIES "")

IF ( BUILD_OPENGL_RENDERER )
	TARGET_LINK_LIBRARIES( VideoMan ${OPENGL_LIBRARIES} )
ENDIF ( BUILD_OPENGL_RENDERER )

add_subdirectory( inputs )

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_EXAMPLES_OPENCV "Build examples using OpenCV" ON)
IF(BUILD_OPENGL_RENDERER)
	option(BUILD_EXAMPLES_GLUT "Build examples based on GLUT" ON)
ENDIF ( BUILD_OPENGL_RENDERER )
IF ( NOT BUILD_EXAMPLES)	
	SET( BUILD_EXAMPLES_OPENCV OFF CACHE BOOL "" FORCE )
	SET( BUILD_EXAMPLES_GLUT OFF CACHE BOOL "" FORCE )
ENDIF ( NOT BUILD_EXAMPLES )
IF(NOT BUILD_OPENGL_RENDERER)
	SET( BUILD_EXAMPLES_GLUT OFF CACHE BOOL "" FORCE )
ENDIF(NOT BUILD_OPENGL_RENDERER)	
IF(BUILD_EXAMPLES)
	add_subdirectory( examples )
ENDIF(BUILD_EXAMPLES)

#------ FindVideoMan.cmake ---------
set(${PROJECT_NAME}_VERSION "1.0.0" )
string(REGEX MATCHALL "[0-9]" ${PROJECT_NAME}_VERSION_PARTS "${${PROJECT_NAME}_VERSION}" )
list(GET ${PROJECT_NAME}_VERSION_PARTS 0 ${PROJECT_NAME}_VERSION_MAJOR)
list(GET ${PROJECT_NAME}_VERSION_PARTS 1 ${PROJECT_NAME}_VERSION_MINOR)
list(GET ${PROJECT_NAME}_VERSION_PARTS 2 ${PROJECT_NAME}_VERSION_PATCH)
set(${PROJECT_NAME}_SOVERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}" )

#set(CMAKE_INCLUDE_DIRS_CONFIGCMAKE ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})
set(CMAKE_INCLUDE_DIRS_CONFIGCMAKE ${PROJECT_SOURCE_DIR}/include/)
#set(CMAKE_LIB_DIRS_CONFIGCMAKE ${CMAKE_INSTALL_PREFIX}/lib CACHE PATH "Output directory for libraries" )
set(CMAKE_LIB_DIRS_CONFIGCMAKE ${VideoMan_BINARY_DIR}/bin/  )

set( VideoMan_DEBUG_POSTFIX d)

CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/config.cmake.in" "${VideoMan_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)

SET( $ENV{VideoMan_DIR} ${PROJECT_BINARY_DIR} )
