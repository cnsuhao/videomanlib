# serach for The Imaging Source SDK  and set the following variables:
# ImagingSource_FOUND
# ImagingSource_INCLUDE_DIR
# ImagingSource_LIBRARIES

IF(WIN32)  
	FIND_PATH(ImagingSource_SDK include/tisgrabber.h
	$ENV{ProgramFiles}/IC Imaging Control 3.3/TIS Grabber DLL/	 )    
ELSE(WIN32)	    
	FIND_PATH(ImagingSource_SDK "include/tisgrabber.h" "/usr/")	
ENDIF(WIN32) 

IF(ImagingSource_SDK)	
	SET(ImagingSource_INCLUDE_DIR "${ImagingSource_SDK}/include")
	IF(WIN32)
		SET(ImagingSource_LIBRARIES "${ImagingSource_SDK}/bin/win32/tisgrabber.lib")   
	ELSE(WIN32)
		SET(ImagingSource_LIBRARIES "${ImagingSource_SDK}/bin/libueye_api.so")
	ENDIF(WIN32)
	SET(ImagingSource_FOUND TRUE)
ELSE(ImagingSource_SDK)	
	SET(ImagingSource_FOUND FALSE)
ENDIF(ImagingSource_SDK)
 

 