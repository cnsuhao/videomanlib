# serach for IDS Falcon SDK  and set the following variables:
# FALCON_FOUND
# FALCON_INCLUDE_DIR
# FALCON_LIBRARIES


   IF(WIN32)
    FIND_PATH(IDS_FALCON_SDK "include/Falcon.h"
      "$ENV{ProgramFiles}/IDS/Falcon/Develop/"
	  "$ENV{ProgramW6432}/IDS/Falcon/Develop/")
   ELSE(WIN32)
    FIND_PATH(IDS_FALCON_SDK "include/falcon.h" "/usr/")
   ENDIF(WIN32)
   
   IF(IDS_FALCON_SDK)
      SET(FALCON_INCLUDE_DIR ${IDS_FALCON_SDK}/include )
      IF(WIN32)
	SET(FALCON_LIBRARIES "${IDS_FALCON_SDK}/Lib/falcon.lib")
      ELSE(WIN32)
	SET(FALCON_LIBRARIES "${IDS_FALCON_SDK}/lib/libfalcon.so.1.0.0")
      ENDIF(WIN32)

   SET(FALCON_FOUND TRUE)

   ELSE(IDS_FALCON_SDK)
     SET(FALCON_FOUND FALSE)
   ENDIF(IDS_FALCON_SDK)
