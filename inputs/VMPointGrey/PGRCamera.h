#pragma once
#include <windows.h>

#include <string>
#include <vector>

//=============================================================================
// PGR Includes
//=============================================================================
#include "VideoInput.h"

#include "pgrflycapture.h"
#include "pgrflycaptureplus.h"
#include "pgrcameragui.h"


#define PGR_PROJECT_NAME "VideoMan"
#define PGR_FILE_NAME    "$RCSfile: PGRCamera.h,v $"
#define PGR_FILE_VERSION "$Revision: 1.2 $"

#define BUFFERS_NUM 200

//friend class PGRController;

/*
	SUPPORTED FORMATS:
	800x600:		
	640X320:
	1024x768:
	1200x960:
	1600x1200:
		-RGB24
		-GREY16
		-GREY8
		-YUV422
		-YUV411 problems!
		-RAW8
	320X240:
		-YUV422
	FORMAT7(custom mode)
*/

class PGRCamera : 
	public VideoManPrivate::VideoInput
{
public:
	PGRCamera(void);
	virtual ~PGRCamera(void);

	bool initCamera( unsigned long aSerialNumber, VideoMan::VMInputFormat *aFormat = NULL );
	
			
	/** \brief  Get the a new frame			
		\return the frame of the video
	*/
	inline char *getFrame( bool wait );

	void showPropertyPage();

	// Start grabbing 
	void startGrabRecord();
	
	int getNumberOfLostFrames();
	
	bool setRegister( unsigned long reg, unsigned long value );
	bool setRegisterBroadcast( unsigned long reg, unsigned long value );

	FlyCaptureContext &getContext();

	//bool start(unsigned long serialNumber, VMInputFormat *format);

	static void getAvailableDevices(  VideoMan::VMInputIdentification **deviceList, int &numDevices );

	VideoManPrivate::VideoManInputController *getController();

	bool linkController( VideoManPrivate::VideoManInputController *controller );

	void printInfo();

	bool setImageROI( int x, int y, int width, int height, int videoMode );

	bool moveImageROI( int x, int y );

	bool getROIUnits( int &Hmax, int &Vmax, int &Hunit, int &Vunit, int &HPosUnit, int &VPosUnit, int videoMode );

	bool resetImageROI();

	bool setGainControl(bool autoGain);

	bool getGainControl();

	bool setExposureControl( bool autoExp );

	bool setSharpnessControlValue(float value);
	
	bool setSharpnessControl( bool autoSharp );

	bool setShutterTime( float shutterTime );

private:

	// Helper code to handle a FlyCapture error.
	bool checkCaptureError( FlyCaptureError error, std::string message );
	// Helper code to handle a Gui error.
	bool checkGuiError( CameraGUIError error, std::string message );
	
	bool resolveFormat( FlyCaptureVideoMode videoMode, FlyCaptureFrameRate frameRate, VideoMan::VMInputFormat &format );

	void processAvi( std::string &rawFileName, std::string &outputFileName );

	void initializeConvertedImage();	

	VideoMan::VMPixelFormat resolveColorCodingID( int ID );

	int buildColorCoding( VideoMan::VMPixelFormat pixelFormat );

	FlyCaptureVideoMode buildVideoMode( VideoMan::VMInputFormat format );

    FlyCaptureContext context;  
	CameraGUIContext  guiContext;

	FlyCaptureImage imageG;
	FlyCaptureImagePlus image;
	FlyCaptureImage imageConverted;
		
	FlyCaptureVideoMode videoMode;
	FlyCaptureFrameRate frameRate;

	//Variables for record videos
	bool recording; //If we are recording images	
	int lastRecordedFrame; //The sequence id of the last recorded frame
	int lostFrames; //The number of frames that we have lost during the record process
	int numRecordedFrames; //The number of recorded frames
	
	VideoManPrivate::VideoManInputController *controller;
	
	bool started;  //If the camera has benn started or not
	unsigned long serialNumber; //The serial number of the camera
	unsigned int uiBusIndex; //The index on the bus of the camera
	bool convertForTheRenderer; //If we must convert each frame

	unsigned long colorCoding;
	//int width, height;
	//int xOffset,yOffset;
	//int xROI, yROI, wROI, hROI;
};
