INCLUDE_DIRECTORIES(${VideoMan_SOURCE_DIR}/inputs)

IF(WIN32)

option(BUILD_INPUT_VMDirectShow "Build VMDirectShow input module for input devices and video files using Microsoft DirectShow" ON)
IF(BUILD_INPUT_VMDirectShow)	
SUBDIRS(
 VMDirectShow
 )
ENDIF(BUILD_INPUT_VMDirectShow)

option(BUILD_INPUT_VMPointGrey "Build VMPointGrey input module for PointGrey cameras" OFF)
IF(BUILD_INPUT_VMPointGrey)
SUBDIRS(
VMPointGrey
)
ENDIF(BUILD_INPUT_VMPointGrey)

option(BUILD_INPUT_VMOpenNI "Build OpenNI input module for Kinect sensor" OFF)
IF(BUILD_INPUT_VMOpenNI)
SUBDIRS(
VMOpenNI
)
ENDIF(BUILD_INPUT_VMOpenNI)

ENDIF(WIN32)

IF(UNIX)
option(BUILD_INPUT_VMDC1394 "Build VMDC1394 input module for IEEE1394 cameras" OFF)
IF(BUILD_INPUT_VMDC1394)
SUBDIRS(
VMDC1394
)
ENDIF(BUILD_INPUT_VMDC1394)

ENDIF(UNIX)

option(BUILD_INPUT_VMImageSequence "Build VMImageSequence input module for image file sequences" OFF)
IF (BUILD_INPUT_VMImageSequence)
SUBDIRS(
 VMImageSequence
)
ENDIF(BUILD_INPUT_VMImageSequence)

option(BUILD_INPUT_VMImageFile "Build VMImageFile input module for a single image file" OFF)
IF (BUILD_INPUT_VMImageFile)
SUBDIRS(
 VMImageFile
)
ENDIF(BUILD_INPUT_VMImageFile)

option(BUILD_INPUT_VMHighgui "Build VMHighgui input module for input devices and video files using OpenCV-Highgui" OFF)
IF(BUILD_INPUT_VMHighgui)	
SUBDIRS(
VMHighgui
)
ENDIF(BUILD_INPUT_VMHighgui)

option(BUILD_INPUT_VMPointGrey2 "Build VMPointGrey2 input module for PointGrey cameras (using FlyCapture2)" OFF)
IF(BUILD_INPUT_VMPointGrey2)
SUBDIRS(
VMPointGrey2
)
ENDIF(BUILD_INPUT_VMPointGrey2)

option(BUILD_INPUT_VMIDSuEye "Build VMIDSuEye input module for IDS uEye cameras" OFF)
IF(BUILD_INPUT_VMIDSuEye)
SUBDIRS(
VMIDSuEye
)
ENDIF(BUILD_INPUT_VMIDSuEye)

option(BUILD_INPUT_VMIDSFalcon "Build VMIDSFalcon input module for IDS frame-grabbers" OFF)
IF(BUILD_INPUT_VMIDSFalcon)
SUBDIRS(
VMIDSFalcon
)
ENDIF(BUILD_INPUT_VMIDSFalcon)


