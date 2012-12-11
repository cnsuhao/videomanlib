#ifndef PGR_CAMERA2
#define PGR_CAMERA2

//#include <windows.h>

#include <string>
#include <vector>

//=============================================================================
// PGR Includes
//=============================================================================
#include "VideoInput.h"

#include "FlyCapture2Defs.h"
#include "FlyCapture2GUI.h"
#include "Camera.h"
#include "Image.h"


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

	bool supportFrameCallback()
	{
		return true;
	}

	bool initCamera( unsigned long aSerialNumber, VideoMan::VMInputFormat *aFormat = NULL );
	
	/** \brief  Get the a new frame			
		\return the frame of the video
	*/
	inline char *getFrame( bool wait );

	void releaseFrame();

	void showPropertyPage();

	VideoManPrivate::VideoManInputController *getController();

	bool linkController( VideoManPrivate::VideoManInputController *controller );
	
	// Start grabbing 
	void startGrabRecord();
	
	int getNumberOfLostFrames();
	
	//bool setRegister( unsigned long reg, unsigned long value );
	//bool setRegisterBroadcast( unsigned long reg, unsigned long value );

	//bool start(unsigned long serialNumber, VMInputFormat *format);

	static void getAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices );

	void setFrameCallback( VideoManPrivate::VideoInput::getFrameCallback theCallback, void *data );

	friend class PointGreyController;
	friend void frameCallback( FlyCapture2::Image* pImage, const void* pCallbackData );

protected: 
	FlyCapture2::Camera cam;

	bool setImageROI( int x, int y, int width, int height, int videoMode );

	bool getROIUnits( int &Hmax, int &Vmax, int &Hunit, int &Vunit, int &HPosUnit, int &VPosUnit, int videoMode );

	bool moveImageROI( int x, int y );

	bool resetImageROI();

	bool setGainControl(bool autoGain);

	bool getGainControl();

	bool setExposureControl( bool autoExp );

	bool setSharpnessControlValue(float value);
	
	bool setSharpnessControl( bool autoSharp );

	bool setShutterTime( float shutterTime );
	
	bool setFrameRate( float frameRate );

	bool setTrigger( bool triggerOn, int source, int mode );

	bool fireSoftwareTrigger( bool broadcast );

	bool setStrobeOutput();

	void stop();

private:

	bool resolveFormat( FlyCapture2::VideoMode videoMode, FlyCapture2::FrameRate frameRate, VideoMan::VMInputFormat &format );

	VideoMan::VMPixelFormat resolvePixelFormat( FlyCapture2::PixelFormat );
	
	FlyCapture2::VideoMode buildVideoMode( VideoMan::VMInputFormat format );

	unsigned int getRegisterValue(unsigned long _register );

	bool resetCamera( );

	int buildColorCoding( VideoMan::VMPixelFormat pixelFormat );


/*	void processAvi( std::string &rawFileName, std::string &outputFileName );

	void initializeConvertedImage();	

	VMPixelFormat resolveColorCodingID( int ID );

	

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
	
	//PGRController *controller;

	unsigned int act; //index of the actual locked frame
	bool started;  //If the camera has benn started or not
	
	unsigned int uiBusIndex; //The index on the bus of the camera
	bool convertForTheRenderer; //If we must convert each frame*/
	FlyCapture2::CameraControlDlg *dlg;
	FlyCapture2::CameraInfo *camInfo;
	//HANDLE showControlDlgThread;

	FlyCapture2::Image* lastImage;
	FlyCapture2::Image rawImage; 
	FlyCapture2::Image convertedImage;
	FlyCapture2::Image convertedImageCopy;

	unsigned long serialNumber; //The serial number of the camera

	FlyCapture2::VideoMode videoMode;
	FlyCapture2::FrameRate frameRate;

	VideoManPrivate::VideoManInputController *controller;

	//int width, height;
	//int xOffset,yOffset;
	//int xROI, yROI, wROI, hROI;

};
#endif