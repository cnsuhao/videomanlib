#pragma once
#include "VideoInput.h"
#include "VideoManInputFormat.h"
#include <include/uEye.h>


class IDSuEye :
	public VideoManPrivate::VideoInput
{
public:
	IDSuEye(void);
	virtual ~IDSuEye(void);

	bool initInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );
		
	char *getFrame( bool wait = false);

	void releaseFrame( );
	
	static void getAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices  );	

	bool setImageROI( int x, int y, int width, int height );
	bool setImageROIpos( int x, int y );

	bool linkController( VideoManPrivate::VideoManInputController *acontroller );

private:

	int colorModeFromPixelFormat( VideoMan::VMPixelFormat pixelFormat );
	VideoMan::VMPixelFormat pixelFormatFromColorMode( int colorMode );

	VideoManPrivate::VideoManInputController *controller;

	HIDS m_hCam;
	int m_lMemoryId[2];		// camera memory - buffer ID
	char*	m_pcImageMemory[2];	// camera memory - pointer to buffer
	SENSORINFO m_sInfo;			// sensor information struct
	
	INT		m_nColorMode;		// Y8/RGB16/RGB24/REG32
	INT		m_nBitsPerPixel;	// number of bits needed store one pixel
	INT		m_nSizeX;			// width of image
	INT		m_nSizeY;			// height of image
	INT		m_nPosX;			// left offset of image
	INT		m_nPosY;			// right offset of image
};
