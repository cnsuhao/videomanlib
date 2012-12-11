#ifndef VIDEOFILEDSHOW_H
#define VIDEOFILEDSHOW_H

#include "VideoManInputFormat.h"
#include "VideoInputDShow.h"

class VideoFileDShow :
	public VideoManPrivate::VideoInput, VideoInputDShow
{
public:
	VideoFileDShow(void);
	virtual ~VideoFileDShow(void);

	/** \brief  Add a video input from a video file
		\if aFormat==NULL -->> RGB24
		\param name [in] the name of the video file
		\param format [in/out] The preferred format of the video input, the resolution and fps will be taken from the video file
		\return false if something fails
	*/
	bool loadVideoFile( const char *file, VideoMan::VMInputFormat *aFormat = NULL );

	char *getFrame( bool wait = false);

	void releaseFrame();

	void play();
	
	void pause();
	
	void stop();

	int getLengthFrames();
	
	double getLengthSeconds();
	
	void goToFrame( int frame );

	void goToMilisecond(  double milisecond );

	int getPositionFrames();
	
	double getPositionSeconds();

	bool supportFrameCallback();

	void setFrameCallback( getFrameCallback theCallback, void *data );
	
	HRESULT WINAPI SampleCB(double SampleTime, IMediaSample *pSample);
	HRESULT WINAPI BufferCB(double sampleTimeSec, BYTE* bufferPtr, long bufferLength);


private:
	HRESULT prepareMedia( const std::string &name, VideoMan::VMInputFormat *aFormat );	
	bool paused;
	double LastSampleTime;
	double avgTimePerFrame;
};

#endif