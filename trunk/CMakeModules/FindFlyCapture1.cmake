#
# Try to find Pont Grey FlyCapture 1 SDK.
# available from: www.ptgrey.com
#
# Once run this will define: 
#
# FlyCapture1_FOUND
# FlyCapture1_INCLUDE_DIR
# FlyCapture1_LIBRARIES
# FlyCapture1_GUI_LIBRARY . additional library if you want to use teh (MFC) PTG Gui controls.
# FlyCapture1_HEADER  This is a hack to allow command completion in MSVS
#
#  (FlyCapture1_LINK_DIRECTORIES: not yet...)
# --------------------------------

IF (NOT WIN32)
  MESSAGE("FindFlyCapture1.cmake: This library supports only WIN32. skipping.")
  SET(FlyCapture1_FOUND FALSE)
ENDIF (NOT WIN32)

IF (WIN32)
  FIND_PATH(FlyCapture1_INCLUDE_DIR PGRFlyCapture.h
    # $ENV{FlyCapture1_HOME}/include
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Point Grey Research, Inc.\\PGRFlyCapture;InstallDir]/include"
    "$ENV{ProgramFiles}/Point Grey Research/PGR FlyCapture/include"
	"$ENV{ProgramFiles}/Point Grey Research/FlyCapture2/include/FC1"
    "$ENV{EXTRA}/include"
    )
  #MESSAGE("DBG FlyCapture1_INCLUDE_DIR=${FlyCapture1_INCLUDE_DIR}")

  FIND_LIBRARY(FlyCapture1_LIBRARY
    NAMES PGRFlyCapture
    PATHS 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Point Grey Research, Inc.\\PGRFlyCapture;InstallDir]/lib"
    "$ENV{ProgramFiles}/Point Grey Research/PGR FlyCapture/lib"
	"$ENV{ProgramFiles}/Point Grey Research/FlyCapture2/lib/FC1"
    "$ENV{EXTRA}/lib"
    )
  #MESSAGE("DBG FlyCapture1_LIBRARY=${FlyCapture1_LIBRARY}")


  FIND_LIBRARY(FlyCapture1_GUI_LIBRARY
    NAMES pgrflycapturegui
    PATHS 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Point Grey Research, Inc.\\PGRFlyCapture;InstallDir]/lib"
    "$ENV{ProgramFiles}/Point Grey Research/PGR FlyCapture/lib"
	"$ENV{ProgramFiles}/Point Grey Research/FlyCapture2/lib/FC1"
    "$ENV{EXTRA}/include"
    )
  #MESSAGE("DBG FlyCapture1_GUI_LIBRARY=${FlyCapture1_GUI_LIBRARY}")

  
  # --------------------------------

  IF(FlyCapture1_LIBRARY)
    SET(FlyCapture1_LIBRARIES ${FlyCapture1_LIBRARY})
  ELSE(FlyCapture1_LIBRARY)
    MESSAGE(SEND_ERROR "FlyCapture1 library not found.")
  ENDIF(FlyCapture1_LIBRARY)

  IF(NOT FlyCapture1_INCLUDE_DIR)
    MESSAGE(SEND_ERROR "FlyCapture1 include dir not found.")
  ENDIF(NOT FlyCapture1_INCLUDE_DIR)

  IF(FlyCapture1_LIBRARIES AND FlyCapture1_INCLUDE_DIR)
    SET(FlyCapture1_FOUND TRUE)
    
    SET( FlyCapture1_HEADER 
      "${FlyCapture1_INCLUDE_DIR}/PGRFlyCapture.h"
      "${FlyCapture1_INCLUDE_DIR}/PGRFlyCapturePlus.h"
      "${FlyCapture1_INCLUDE_DIR}/pgrerror.h"
      "${FlyCapture1_INCLUDE_DIR}/pgrcameragui.h"
      )
    #MESSAGE("DBG FlyCapture1_HEADER=${FlyCapture1_HEADER}")
    SOURCE_GROUP(FlyCapture1394 FILES ${FlyCapture1_HEADER})
    
    # extract link directory
    IF    (FlyCapture1_LIBRARY)
      GET_FILENAME_COMPONENT(FlyCapture1_LINK_DIRECTORIES ${FlyCapture1_LIBRARY} PATH)
    ENDIF (FlyCapture1_LIBRARY)

  ELSE(FlyCapture1_LIBRARIES AND FlyCapture1_INCLUDE_DIR)
    SET(FlyCapture1_FOUND FALSE)
  ENDIF(FlyCapture1_LIBRARIES AND FlyCapture1_INCLUDE_DIR)

  MARK_AS_ADVANCED(
    FlyCapture1_INCLUDE_DIR
    FlyCapture1_LIBRARY
    FlyCapture1_LIBRARIES
    FlyCapture1_GUI_LIBRARY
    )

ENDIF (WIN32)
