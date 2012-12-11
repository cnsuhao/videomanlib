#ifndef PGR_CAMERA2
#define PGR_CAMERA2

//#include <windows.h>

#include <string>
#include <vector>

//=============================================================================
// PGR Includes
//=============================================================================
#include "VideoInput.h"

#include "XnCppWrapper.h"

#define KINECT_PROJECT_NAME "VideoMan"
#define KINECT_FILE_NAME    "$RCSfile: KINECTCamera.h,v $"
#define KINECT_FILE_VERSION "$Revision: 1.2 $"

#define BUFFERS_NUM 200
#define RGB_CAMERA 0
#define DEPTH_CAMERA 1
#define DEPTH_CAMERA_COLOR 2
#define IR_CAMERA 3



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

void copyStringToChar( const std::string &src, char **dst );


class KinectCamera : 
	public VideoManPrivate::VideoInput
{
public:
	KinectCamera(void);
	virtual ~KinectCamera(void);

	bool supportFrameCallback()
	{
		return false;
	}

	bool initCamera( std::string source, std::string xmlFile  );
	bool initCamera( std::string source );

/* funciones virtuales de VideoInput */

	/** \brief  Get the a new frame			
		\return the frame of the video
	*/
	inline char *getFrame( bool wait );

	void releaseFrame();

	void showPropertyPage();

	VideoManPrivate::VideoManInputController *getController();

	bool linkController( VideoManPrivate::VideoManInputController *controller );

	void setFrameCallback( VideoManPrivate::VideoInput::getFrameCallback theCallback );

/* ******** */

	void raw2depth();
	char* depth2rgb(const XnDepthPixel* Xn_disparity);
	void getFormat( VideoMan::VMInputFormat &_format );


protected: 
	xn::Context __context;
	
	xn::DepthGenerator __depth; 
	xn::ImageGenerator __image;
	xn::IRGenerator __ir;
	
	xn::DepthMetaData __depthMD;
	xn::ImageMetaData __imageMD;
	xn::IRMetaData __irMD;

	char *__data;

	XnMapOutputMode __mapMode; 
	int __source;

	unsigned short depth[10000];




	



private:


	
	
	unsigned long serialNumber; //The serial number of the camera

	VideoManPrivate::VideoManInputController *controller;


};
#endif