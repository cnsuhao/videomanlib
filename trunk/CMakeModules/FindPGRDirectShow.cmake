#
# Try to find Point Grey Direct Show interface
# available from: www.ptgrey.com
#
# Once run this will define: 
#
# PGRDirectShow_FOUND
# PGRDirectShow_INCLUDE_DIR
#
# Javier Barandiaran 05/2011.
# based on Find1394FlyCapture.cmake by:
# Jan Woetzel 08/2004.
# www.mip.informatik.uni-kiel.de/~jw
# --------------------------------


  FIND_PATH(PGRDirectShow_INCLUDE_DIR PGRDirectShow.h
    # $ENV{FlyCapture2_HOME}/include
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Point Grey Research, Inc.\\PGRFlyCapture;InstallDir]/include"
    # "$ENV{ProgramFiles}/Point Grey Research/include"
	"$ENV{ProgramFiles}/Point Grey Research/FlyCapture2/include"
    "$ENV{EXTRA}/include"
	  # $ENV{1394FlyCapture_HOME}/include
    )
  #MESSAGE("DBG PGRDirectShow_INCLUDE_DIR=${PGRDirectShow_INCLUDE_DIR}")

  # --------------------------------

  IF(NOT PGRDirectShow_INCLUDE_DIR)
    MESSAGE(SEND_ERROR "Point Grey Direct Show interface include dir not found.")
  ENDIF(NOT PGRDirectShow_INCLUDE_DIR)

  IF(PGRDirectShow_INCLUDE_DIR)
    SET(PGRDirectShow_FOUND TRUE)
  ENDIF(PGRDirectShow_INCLUDE_DIR)

  MARK_AS_ADVANCED(
    PGRDirectShow_INCLUDE_DIR
    )


