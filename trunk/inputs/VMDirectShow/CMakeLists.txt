LINK_DIRECTORIES(
	  ${VideoMan_BINARY_DIR}/bin/
	  ${CMAKE_CURRENT_SOURCE_DIR}/lib
)


INCLUDE_DIRECTORIES(
include
)

SET( VMDirectShow_SRCS
	CaptureDeviceDShow.cpp
	DirectShowHelpers.cpp
	stdafx.cpp
	VideoFileDShow.cpp
	VideoInputDShow.cpp
	VMDirectShow.cpp
	VideoInputDShow.h
	VideoFileDShow.h
	stdafx.h
	DirectShowHelpers.h
	CaptureDeviceDShow.h
	crossbar.h
	crossbar.cpp
	CaptureDeviceDSController.cpp
	CaptureDeviceDSController.h	
)

#IDS uEye Camera Controller
OPTION( BUILD_INPUT_DS_uEye_CONTROLLER "Build controller for IDS uEye cameras using DirectShow" OFF )
IF( BUILD_INPUT_DS_uEye_CONTROLLER )
	FIND_PACKAGE( UEYEDS REQUIRED )
	IF (NOT UEYEDS_FOUND)
		MESSAGE(FATAL_ERROR "UEye DirectShow Interface (uEyeCaptureInterface.h) required, please specify it's location with IDS_UEYEDS_INTERFACE or disable BUILD_INPUT_DS_uEye_CONTROLLER")	
	ENDIF (NOT UEYEDS_FOUND)
	IF (UEYEDS_FOUND)
		SET( VMDirectShow_SRCS
			${VMDirectShow_SRCS}
			DSuEyeCameraController.cpp
			DSuEyeCameraController.h
		)
		ADD_DEFINITIONS( -DIDS_UEYE )
		INCLUDE_DIRECTORIES(${UEYEDS_INCLUDE_DIR})
	ENDIF (UEYEDS_FOUND)
ENDIF( BUILD_INPUT_DS_uEye_CONTROLLER )

#PointGrey Camera Controller
OPTION( BUILD_INPUT_DS_PointGrey_CONTROLLER "Build controller for PointGrey cameras using DirectShow" OFF )
IF( BUILD_INPUT_DS_PointGrey_CONTROLLER )
	FIND_PACKAGE( PGRDirectShow REQUIRED )
	
	IF (NOT PGRDirectShow_FOUND)
		MESSAGE(FATAL_ERROR "PointGrey DirectShow Interface required (PGRDirectShow.h), please specify it's location with _INTERFACE or disable BUILD_INPUT_DS_PointGrey_CONTROLLER")	
	ENDIF (NOT PGRDirectShow_FOUND)
	IF (PGRDirectShow_FOUND)
		SET( VMDirectShow_SRCS
			${VMDirectShow_SRCS}
			DSPointGreyController.cpp
			DSPointGreyController.h
		)
		ADD_DEFINITIONS( -DDS_POINTGREY )
		INCLUDE_DIRECTORIES(${PGRDirectShow_INCLUDE_DIR})
	ENDIF (PGRDirectShow_FOUND)
ENDIF( BUILD_INPUT_DS_PointGrey_CONTROLLER )




IF (NOT MSVC60)
IF (NOT MSVC71)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")
ENDIF (NOT MSVC71)
ENDIF (NOT MSVC60)

ADD_LIBRARY( VMDirectShow SHARED
${VMDirectShow_SRCS}
)

TARGET_LINK_LIBRARIES( VMDirectShow   
	VideoMan
	strmiids.lib
	comsupp.lib 
	winmm.lib
)

set_target_properties( VMDirectShow PROPERTIES FOLDER "inputs")
add_dependencies( VMDirectShow VideoMan )