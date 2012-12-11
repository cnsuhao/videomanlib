
SET( VMOpenNI_SRCS
	VMOpenNI.cpp	
	KinectCamera.h	
	KinectCamera.cpp
)
INCLUDE_DIRECTORIES(  $ENV{OPEN_NI_INCLUDE}  )
LINK_DIRECTORIES($ENV{OPEN_NI_LIB})
ADD_LIBRARY( VMOpenNI   SHARED ${VMOpenNI_SRCS} )
TARGET_LINK_LIBRARIES( VMOpenNI   
	VideoMan
	OpenNI.lib
)
set_target_properties( VMOpenNI PROPERTIES FOLDER "inputs")
add_dependencies( VMOpenNI   VideoMan )