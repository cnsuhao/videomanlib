FIND_PACKAGE( UEYE REQUIRED )
IF (NOT UEYE_FOUND)
	MESSAGE(FATAL_ERROR "IDS UEye SDK required, please specify it's location with IDS_UEYE_SDK or disable BUILD_INPUT_VMIDSuEye")	
ENDIF (NOT UEYE_FOUND)
SET( VMIDSuEye_SRCS
	VMIDSuEye.cpp	
	IDSuEye.cpp
	IDSuEye.h
	uEyeCameraController.cpp
	uEyeCameraController.h
)
INCLUDE_DIRECTORIES(  ${UEYE_INCLUDE_DIR}  )
ADD_LIBRARY( VMIDSuEye SHARED ${VMIDSuEye_SRCS} )
TARGET_LINK_LIBRARIES( VMIDSuEye   
	VideoMan
	${UEYE_LIBRARIES}
)
set_target_properties( VMIDSuEye PROPERTIES FOLDER "inputs")
add_dependencies( VMIDSuEye VideoMan )