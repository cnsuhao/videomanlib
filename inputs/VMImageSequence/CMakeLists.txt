FIND_PACKAGE( OpenCV REQUIRED )
LINK_DIRECTORIES(
	  ${VideoMan_BINARY_DIR}/bin/
)
SET( VMImageSequence_SRCS
	VMImageSequence.cpp
	ImageSequence.cpp
	ImageSequence.h
)
INCLUDE_DIRECTORIES(  ${OpenCV_INCLUDE_DIRS}  )
ADD_LIBRARY( VMImageSequence SHARED ${VMImageSequence_SRCS} )
TARGET_LINK_LIBRARIES( VMImageSequence   
	VideoMan
	${OpenCV_LIBS}
)
set_target_properties( VMImageSequence PROPERTIES FOLDER "inputs")
add_dependencies( VMImageSequence VideoMan )