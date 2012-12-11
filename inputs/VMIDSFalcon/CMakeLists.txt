FIND_PACKAGE( Falcon REQUIRED )
SET( VMIDSFalcon_SRCS
	VMIDSFalcon.cpp
	IDSFalconFrameGrabber.cpp
	IDSFalconFrameGrabber.h
	falconFrameGrabberController.h	
	falconFrameGrabberController.cpp
)
INCLUDE_DIRECTORIES(  ${FALCON_INCLUDE_DIR}  )
ADD_LIBRARY( VMIDSFalcon   SHARED ${VMIDSFalcon_SRCS} )
TARGET_LINK_LIBRARIES( VMIDSFalcon   
	VideoMan
	${FALCON_LIBRARIES}
)
set_target_properties( VMIDSFalcon PROPERTIES FOLDER "inputs")
add_dependencies( VMIDSFalcon VideoMan )