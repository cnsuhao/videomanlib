# serach for IDS DirectShow Interface and set the following variables:
# UEYEDS_FOUND
# UEYEDS_INCLUDE_DIR
# UEYE_LIBRARIES

  FIND_PATH( IDS_UEYEDS_INTERFACE "uEyeCaptureInterface.h"
    "$ENV{ProgramFiles}/IDS/uEye/Develop/include/"
	"$ENV{ProgramW6432}/IDS/uEye/Develop/include/" )

IF(IDS_UEYEDS_INTERFACE)
	# MESSAGE(" uEye DirectShow interface is used from: ${IDS_UEYEDS_INTERFACE}")
	SET( UEYEDS_INCLUDE_DIR ${IDS_UEYEDS_INTERFACE} )
	SET(UEYEDS_FOUND TRUE)
ELSE(IDS_UEYEDS_INTERFACE)	
	SET(UEYEDS_FOUND FALSE)
ENDIF(IDS_UEYEDS_INTERFACE)


 