INCLUDE_DIRECTORIES(${VideoMan_SOURCE_DIR}/inputs)

IF(WIN32)

option(VIDEOMAN_BUILD_INPUT_VMDirectShow "Build VMDirectShow input module for input devices and video files using Microsoft DirectShow" ON)
IF(VIDEOMAN_BUILD_INPUT_VMDirectShow)	
SUBDIRS(
 VMDirectShow
 )
ENDIF(VIDEOMAN_BUILD_INPUT_VMDirectShow)

#option(VIDEOMAN_BUILD_INPUT_VMPointGrey "Build VMPointGrey input module for PointGrey cameras" OFF)
#IF(VIDEOMAN_BUILD_INPUT_VMPointGrey)
#SUBDIRS(
#VMPointGrey
#)
#ENDIF(VIDEOMAN_BUILD_INPUT_VMPointGrey)

option(VIDEOMAN_BUILD_INPUT_VMOpenNI "Build OpenNI input module for Kinect sensor" OFF)
IF(VIDEOMAN_BUILD_INPUT_VMOpenNI)
SUBDIRS(
VMOpenNI
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMOpenNI)

#option(VIDEOMAN_BUILD_INPUT_VMGenicam "Build Genicam input module for GenICam cameras" OFF)
IF(VIDEOMAN_BUILD_INPUT_VMGenicam)
SUBDIRS( VMGenicam )
ENDIF(VIDEOMAN_BUILD_INPUT_VMGenicam)


ENDIF(WIN32)

IF(UNIX)
option(VIDEOMAN_BUILD_INPUT_VMDC1394 "Build VMDC1394 input module for IEEE1394 cameras" OFF)
IF(VIDEOMAN_BUILD_INPUT_VMDC1394)
SUBDIRS(
VMDC1394
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMDC1394)

ENDIF(UNIX)

option(VIDEOMAN_BUILD_INPUT_VMImageSequence "Build VMImageSequence input module for image file sequences" OFF)
IF (VIDEOMAN_BUILD_INPUT_VMImageSequence)
SUBDIRS(
 VMImageSequence
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMImageSequence)

option(VIDEOMAN_BUILD_INPUT_VMImageFile "Build VMImageFile input module for a single image file" OFF)
IF (VIDEOMAN_BUILD_INPUT_VMImageFile)
SUBDIRS(
 VMImageFile
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMImageFile)

option(VIDEOMAN_BUILD_INPUT_VMHighgui "Build VMHighgui input module for input devices and video files using OpenCV-Highgui" ON)
IF(VIDEOMAN_BUILD_INPUT_VMHighgui)	
SUBDIRS(
VMHighgui
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMHighgui)

option(VIDEOMAN_BUILD_INPUT_VMPointGrey2 "Build VMPointGrey2 input module for PointGrey cameras (using FlyCapture2)" OFF)
IF(VIDEOMAN_BUILD_INPUT_VMPointGrey2)
SUBDIRS(
VMPointGrey2
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMPointGrey2)

option(VIDEOMAN_BUILD_INPUT_VMIDSuEye "Build VMIDSuEye input module for IDS uEye cameras" OFF)
IF(VIDEOMAN_BUILD_INPUT_VMIDSuEye)
SUBDIRS(
VMIDSuEye
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMIDSuEye)

option(VIDEOMAN_BUILD_INPUT_VMIDSFalcon "Build VMIDSFalcon input module for IDS frame-grabbers" OFF)
IF(VIDEOMAN_BUILD_INPUT_VMIDSFalcon)
SUBDIRS(
VMIDSFalcon
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMIDSFalcon)

option(VIDEOMAN_BUILD_INPUT_VMImagingSource "Build VMImagingSource input module for The Imaging Source cameras and frame-grabbers" OFF)
IF(VIDEOMAN_BUILD_INPUT_VMImagingSource)
SUBDIRS(
VMImagingSource
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMImagingSource)

option(VIDEOMAN_BUILD_INPUT_VMFFmpeg "Build VMFFmpeg input module for video files using FFmpeg libraries" OFF)
IF(VIDEOMAN_BUILD_INPUT_VMFFmpeg)
SUBDIRS(
VMFFmpeg
)
ENDIF(VIDEOMAN_BUILD_INPUT_VMFFmpeg)