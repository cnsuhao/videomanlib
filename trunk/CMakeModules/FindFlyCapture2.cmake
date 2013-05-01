#
# Try to find Pont Grey FlyCapture v2 camera driver library. 
# available from: www.ptgrey.com
#
# Once run this will define: 
#
# FlyCapture2_FOUND
# FlyCapture2_INCLUDE_DIR
# FlyCapture2_LIBRARIES
# FlyCapture2_GUI_LIBRARY . additional library if you want to use teh (MFC) PTG Gui controls.
# FlyCapture2_HEADER  This is a hack to allow command completion in MSVS
#
#  (FlyCapture2_LINK_DIRECTORIES: not yet...)
#
# Javier Barandiaran 08/2010.
# based on Find1394FlyCapture.cmake by:
# Jan Woetzel 08/2004.
# www.mip.informatik.uni-kiel.de/~jw
# --------------------------------


  FIND_PATH(FlyCapture2_INCLUDE_DIR FlyCapture2.h
    # $ENV{FlyCapture2_HOME}/include
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Point Grey Research, Inc.\\PGRFlyCapture;InstallDir]/include"
    "$ENV{ProgramFiles}/Point Grey Research/FlyCapture2/include"
    "$ENV{EXTRA}/include"
    /usr/include/flycapture/
    )
  #MESSAGE("DBG FlyCapture2_INCLUDE_DIR=${FlyCapture2_INCLUDE_DIR}")

  FIND_LIBRARY(FlyCapture2_LIBRARY
    NAMES FlyCapture2 libflycapture.so
    PATHS 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Point Grey Research, Inc.\\PGRFlyCapture;InstallDir]/lib"
    "$ENV{ProgramFiles}/Point Grey Research/FlyCapture2/lib"
    "$ENV{EXTRA}/lib"
    /usr/lib
    )
  #MESSAGE("DBG FlyCapture2_LIBRARY=${FlyCapture2_LIBRARY}")


  FIND_LIBRARY(FlyCapture2_GUI_LIBRARY
    NAMES FlyCapture2GUI libflycapturegui.so
    PATHS 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Point Grey Research, Inc.\\PGRFlyCapture;InstallDir]/lib"
    "$ENV{ProgramFiles}/Point Grey Research/FlyCapture2/lib"
    "$ENV{EXTRA}/lib"
    /usr/lib
    )
  #MESSAGE("DBG FlyCapture2_GUI_LIBRARY=${FlyCapture2_GUI_LIBRARY}")

  
  # --------------------------------

  IF(FlyCapture2_LIBRARY)
    SET(FlyCapture2_LIBRARIES ${FlyCapture2_LIBRARY})
  ELSE(FlyCapture2_LIBRARY)
    MESSAGE(SEND_ERROR "FlyCapture2 library not found.")
  ENDIF(FlyCapture2_LIBRARY)

  IF(NOT FlyCapture2_INCLUDE_DIR)
    MESSAGE(SEND_ERROR "FlyCapture2 include dir not found.")
  ENDIF(NOT FlyCapture2_INCLUDE_DIR)

  IF(FlyCapture2_LIBRARIES AND FlyCapture2_INCLUDE_DIR)
    SET(FlyCapture2_FOUND TRUE)
    
    SET( FlyCapture2_HEADER 
      "${FlyCapture2_INCLUDE_DIR}/FlyCapture2.h"      
      "${FlyCapture2_INCLUDE_DIR}/Error.h"
      "${FlyCapture2_INCLUDE_DIR}/FlyCapture2GUI.h"
      )
    #MESSAGE("DBG FlyCapture2_HEADER=${FlyCapture2_HEADER}")
    SOURCE_GROUP(FlyCapture1394 FILES ${FlyCapture2_HEADER})
    
    # extract link directory
    IF    (FlyCapture2_LIBRARY)
      GET_FILENAME_COMPONENT(FlyCapture2_LINK_DIRECTORIES ${FlyCapture2_LIBRARY} PATH)
    ENDIF (FlyCapture2_LIBRARY)

  ELSE(FlyCapture2_LIBRARIES AND FlyCapture2_INCLUDE_DIR)
    SET(FlyCapture2_FOUND FALSE)
  ENDIF(FlyCapture2_LIBRARIES AND FlyCapture2_INCLUDE_DIR)

  MARK_AS_ADVANCED(
    FlyCapture2_INCLUDE_DIR
    FlyCapture2_LIBRARY
    FlyCapture2_LIBRARIES
    FlyCapture2_GUI_LIBRARY
    )


