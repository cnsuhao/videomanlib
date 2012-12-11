FIND_PACKAGE( FlyCapture1 REQUIRED )
IF (NOT FlyCapture1_FOUND)
	MESSAGE(FATAL_ERROR "PointGrey Flycapture1 SDK required, please specify it's location with FlyCapture1 variables or disable BUILD_INPUT_VMPointGrey")	
ENDIF (NOT FlyCapture1_FOUND)
SET( VMPointGrey_SRCS
	VMPointGrey.cpp	
	PGRCamera.cpp
	PGRCamera.h
	PointGreyController.h
	PointGreyController.cpp
)
INCLUDE_DIRECTORIES(  ${FlyCapture1_INCLUDE_DIR}  )
ADD_LIBRARY( VMPointGrey SHARED ${VMPointGrey_SRCS} )
TARGET_LINK_LIBRARIES( VMPointGrey   
	VideoMan
	${FlyCapture1_LIBRARIES}
	${FlyCapture1_GUI_LIBRARY}	
)
set_target_properties( VMPointGrey PROPERTIES FOLDER "inputs")
add_dependencies( VMPointGrey VideoMan )