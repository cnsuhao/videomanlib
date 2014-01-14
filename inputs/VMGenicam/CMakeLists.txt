SET( VMGenicam_SRCS
	VMGenicam.cpp )

FIND_LIBRARY(GENICAMAPI_LIB GenApi_MD_VC80_v2_3 ${CMAKE_CURRENT_SOURCE_DIR}/genicamapi/lib/Win32_i86 )

INCLUDE_DIRECTORIES( 
${CMAKE_CURRENT_SOURCE_DIR}/genicamapi/include )

ADD_LIBRARY( VMGenicam SHARED ${VMGenicam_SRCS} )
TARGET_LINK_LIBRARIES( VMGenicam   
	VideoMan
	${GENICAMAPI_LIB}
)
set_target_properties( VMGenicam PROPERTIES FOLDER "inputs")
add_dependencies( VMGenicam VideoMan )