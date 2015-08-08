FIND_PACKAGE( ImagingSource REQUIRED )
IF (NOT ImagingSource_FOUND)
	MESSAGE(FATAL_ERROR "The Imaging Source SDK required, please specify it's location with IMAGING_SOURCE_SDK or disable VIDEOMAN_BUILD_INPUT_VMImagingSource")	
ENDIF (NOT ImagingSource_FOUND)
SET( SRCS
	VMImagingSource.cpp	
	TISInput.cpp
	TISInput.h	
)
INCLUDE_DIRECTORIES(  ${ImagingSource_INCLUDE_DIR}  )
ADD_LIBRARY( VMImagingSource SHARED ${SRCS} )
TARGET_LINK_LIBRARIES( VMImagingSource   
	VideoMan
	${ImagingSource_LIBRARIES}
)
set_target_properties( VMImagingSource PROPERTIES FOLDER "inputs")
add_dependencies( VMImagingSource VideoMan )