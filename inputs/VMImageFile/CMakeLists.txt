FIND_PACKAGE( OpenCV REQUIRED )
LINK_DIRECTORIES(
	  ${VideoMan_BINARY_DIR}/bin/
)
SET( VMImageFile_SRCS
	VMImageFile.cpp
	ImageFile.cpp
	ImageFile.h
)
INCLUDE_DIRECTORIES(  ${OpenCV_INCLUDE_DIRS}  )
ADD_LIBRARY( VMImageFile SHARED ${VMImageFile_SRCS} )
TARGET_LINK_LIBRARIES( VMImageFile   
	VideoMan
	${OpenCV_LIBS}
)
set_target_properties( VMImageFile PROPERTIES FOLDER "inputs")
add_dependencies( VMImageFile VideoMan )