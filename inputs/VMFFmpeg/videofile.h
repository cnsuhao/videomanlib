#pragma once
#include "VideoInput.h"
#include "VideoManInputFormat.h"

extern "C" {
#define __STDC_CONSTANT_MACROS
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"	
};

class VideoFile :
	public VideoManPrivate::VideoInput
{
public:
	VideoFile(void);
	virtual ~VideoFile(void);

	bool initInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );

	char *getFrame( bool wait = false);

	void releaseFrame();

	void goToFrame( int frame );

	void goToMilisecond(  double milisecond );

	double getPositionSeconds();
	
	int getPositionFrames();	
	
	void play();

	void pause();

	double getLengthSeconds();

	int getLengthFrames();



private:
		
	void seek( double seekSecond );

	AVFormatContext *m_avFormatContext;
	AVCodecContext *m_codecContext;
	SwsContext *m_swsContext;
	AVFrame *m_frame;
	AVFrame *m_scaledFrame;
	int m_videoStream;
	AVPixelFormat m_dstPixelFormat;
	int m_lastFrameIndex;
	double m_lastFramePos;
	double m_frameRate;
	bool m_paused;	
	int64_t m_durationMicroSeconds;
	double m_durationSeconds;
	int m_durationFrames;
};
