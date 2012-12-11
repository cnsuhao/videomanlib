FIND_PACKAGE( OpenCV REQUIRED )
LINK_DIRECTORIES(
	  ${VideoMan_BINARY_DIR}/bin/
)
SET( VMHighgui_SRCS
	VMHighgui.cpp
	HighguiDeviceInput.cpp
	HighguiDeviceInput.h
	HighguiVideoFileInput.cpp
	HighguiVideoFileInput.h
)
INCLUDE_DIRECTORIES(  ${OpenCV_INCLUDE_DIRS}  )
ADD_LIBRARY( VMHighgui SHARED ${VMHighgui_SRCS} )
TARGET_LINK_LIBRARIES( VMHighgui   
	VideoMan
	${OpenCV_LIBS}
)
set_target_properties( VMHighgui PROPERTIES FOLDER "inputs")
add_dependencies( VMHighgui VideoMan )