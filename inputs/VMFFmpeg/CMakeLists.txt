FIND_PACKAGE( FFmpeg REQUIRED )

SET( VMFFmpeg_SRCS
	VMFFmpeg.cpp	
	videofile.cpp
	videofile.h	
)


find_path(MSINTTYPES_DIR inttypes)

INCLUDE_DIRECTORIES(  ${FFMPEG_INCLUDE_DIR} ${MSINTTYPES_DIR} )
ADD_LIBRARY( VMFFmpeg SHARED ${VMFFmpeg_SRCS} )
TARGET_LINK_LIBRARIES( VMFFmpeg   
	VideoMan
	${FFMPEG_LIBRARIES}
)
set_target_properties( VMFFmpeg PROPERTIES FOLDER "inputs")
add_dependencies( VMFFmpeg VideoMan )