#ifndef VIDEOINPUT_H
#define VIDEOINPUT_H

#include "VideoManInputFormat.h"
#include "VideoManInputController.h"

namespace VideoManPrivate
{
class VideoInput
{

public:

	//format must be completed when the input is initialized
	VideoInput( void )
	{
		pixelBuffer = NULL;
		callback = NULL;
		roiMode = false;
	}

	virtual ~VideoInput( void )
	{
		if ( identification.uniqueName )
			delete identification.uniqueName;
		identification.uniqueName = NULL;
		if ( identification.friendlyName )
			delete identification.friendlyName;
		identification.friendlyName = NULL;
		if ( identification.identifier )
			delete identification.identifier;
		identification.identifier = NULL;
		if ( identification.fileName )
			delete identification.fileName;
		identification.fileName = NULL;
	};

	/** \brief  Get the format of the video input
	*/
	VideoMan::VMInputFormat getVideoManInputFormat(){ return format; }

	/** \brief  Get a new frame
		\param wait [in] if it must wait for next sample or not
		\return the frame of the video
	*/
	virtual char *getFrame( bool wait = false) = 0;

	virtual VideoManPrivate::VideoManInputController *getController(){ return NULL; }

	virtual bool linkController( VideoManPrivate::VideoManInputController *controller ){return false;}

	virtual void releaseFrame(){}

	char* pixelBuffer;	//The last captured frame

	VideoMan::VMInputIdentification getIdentification()
	{
		return identification;
	}

	typedef void (*getFrameCallback)( char *pixelBuffer, size_t input, double timestamp, void *data );

	virtual void setFrameCallback( getFrameCallback theCallback, void *data )
	{
		callback = theCallback;
		frameCallbackData = data;
	}

	virtual bool supportFrameCallback()
	{
		return false;
	}

	/** VIDEO FILE METHODS **/
	virtual void play(){}

	virtual void pause(){}

	virtual void stop(){}

	virtual double getLengthSeconds(){return 0;}

	virtual int getLengthFrames(){return 0;}

	virtual double getPositionSeconds(){return 0;}

	virtual int getPositionFrames(){return 0;}

	virtual void goToFrame( int frame ){}

	virtual void goToMilisecond( double milisecond ){}

	/** CAPTURE DEVICES METHODS **/
	virtual void showPropertyPage(){}

	virtual double getTimeStamp(){return 0;}

	/** USER INPUT METHODS **/
	virtual void setImage( char *image ){}

	void setInputID( size_t input ){inputID = input;}

	bool isRoiModeEnabled(){ return roiMode; }

	void getImageRoi( int &x, int &y, int &width, int &height )
	{
		x = xRoi;
		y = yRoi;
		width = wRoi;
		height = hRoi;
	}

protected:

	VideoMan::VMInputFormat format;	//The internal format of the frames
	VideoMan::VMInputIdentification identification; //The information about the input (must be released by the input module)

	bool frameCaptured; //if a frame was already captured

	getFrameCallback callback;

	/** VIDEO FILES MEMBERS **/
	int videoLengthFrames;

	double videoLengthSeconds;

	size_t inputID;

	void *frameCallbackData;

	bool roiMode;
	int xRoi, yRoi, wRoi, hRoi;

};
};
#endif
