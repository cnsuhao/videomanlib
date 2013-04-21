FIND_PACKAGE( FlyCapture2 REQUIRED )
IF (NOT FlyCapture2_FOUND)
	MESSAGE(FATAL_ERROR "Pointgrey Flycapture2 SDK required, please specify it's location with Flycapture2 variables or disable VIDEOMAN_BUILD_INPUT_VMPointGrey2")	
ENDIF (NOT FlyCapture2_FOUND)
SET( VMPointGrey_SRCS
	VMPointGrey.cpp	
	PGRCamera.cpp
	PGRCamera.h
	PointGreyController.cpp
	PointGreyController.h
)
INCLUDE_DIRECTORIES(  ${FlyCapture2_INCLUDE_DIR}  )
ADD_LIBRARY( VMPointGrey2   SHARED ${VMPointGrey_SRCS} )
TARGET_LINK_LIBRARIES( VMPointGrey2   
	VideoMan
	${FlyCapture2_LIBRARIES}
	${FlyCapture2_GUI_LIBRARY}	
)
set_target_properties( VMPointGrey2 PROPERTIES FOLDER "inputs")
add_dependencies( VMPointGrey2 VideoMan )