#pragma once
#include "VideoInput.h"
#include "VideoManInputFormat.h"
#include "tisgrabber.h"


class TISinput :
	public VideoManPrivate::VideoInput
{
public:
	TISinput(void);
	virtual ~TISinput(void);

	bool initInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );

	char *getFrame( bool wait = false);

	void releaseFrame( );

	static void getAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices  );

	bool setImageROI( int x, int y, int width, int height );
	bool setImageROIpos( int x, int y );

	bool linkController( VideoManPrivate::VideoManInputController *acontroller );

	void getExposure( double &shutterTime );

	bool setExposure( double shutterTime );

	void getFrameRate( double &frameRate );

	bool setFrameRate( double frameRate );	

	void getTimeStamp( char* buffer );

	void getTimeStamp( unsigned long long* timeStamp );	

	void getPixelClock( unsigned int &pixelClock );

	bool setPixelClock( unsigned int pixelClock );

	bool setMirrorUpDown( bool enable );	

	bool setMirrorLeftRight( bool enable );

	bool setAutoGain( bool enable );

	bool setAutoShutter( bool enable );

	bool setHardwareGamma( bool enable );
	
	bool getHardwareGamma();

	void forceTrigger();

private:

	int colorModeFromPixelFormat( VideoMan::VMPixelFormat pixelFormat );
	VideoMan::VMPixelFormat pixelFormatFromColorMode( int colorMode );

	VideoManPrivate::VideoManInputController *controller;
	char* lastPixelBuffer;	//The last captured frame
    HGRABBER m_hGrabber;
};
