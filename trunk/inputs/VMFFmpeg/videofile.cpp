#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated

#include "videofile.h"
#include <iostream>
//#include <math.h>
//#include <cmath>

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;
extern "C" {
#include <libavutil\time.h>	
#include <libavutil\rational.h>	
#include <libavutil\avutil.h>	
}


void copyStringToChar( const std::string &src, char **dst )
{
	if ( src.length() > 0 )
	{
		*dst = new char[src.length() + 1];
		strcpy( *dst, src.c_str() );
	}
	else
		*dst = NULL;
}

VideoFile::VideoFile(void)
{
	m_avFormatContext = NULL;
	m_codecContext = NULL;
	m_swsContext = NULL;
	m_videoStream = -1;
	m_frame = NULL;
	m_scaledFrame = NULL;
	m_paused = true;
	m_lastFrameIndex = m_lastFramePos = 0;
}

VideoFile::~VideoFile(void)
{
	av_free(m_frame);
	av_free(m_scaledFrame);	
	avcodec_close(m_codecContext);	
	avformat_close_input(&m_avFormatContext);
}

AVPixelFormat get_format( struct AVCodecContext *s, const enum AVPixelFormat *fmt )
{
	for( int i = 0; fmt[i] != AV_PIX_FMT_NONE; ++i )
	{
		enum AVPixelFormat format = fmt[i];
		int a = 0;
	}
	return fmt[0];
}

bool VideoFile::initInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *aformat )
{
	if ( !device.fileName )
		return false;
	av_register_all();

	if(avformat_open_input(&m_avFormatContext, device.fileName, NULL, NULL)!=0)
		return false; // Couldn't open file
	if(avformat_find_stream_info(m_avFormatContext, NULL)<0)
		return false; // Couldn't find stream information
	
	//find video stream
	for(int i=0; i<m_avFormatContext->nb_streams; i++)
	{       
		if((m_avFormatContext->streams[i])->codec->codec_type==AVMEDIA_TYPE_VIDEO)
		{
			m_videoStream=i;
			break;
		}
	}
	if (m_videoStream == -1)
		return false; // Didn't find a video stream
	AVCodecContext *codecContext = m_avFormatContext->streams[m_videoStream]->codec;

	
	//m_codecContext->get_format = get_format;

	//find decoder
	AVCodec *codec=avcodec_find_decoder(codecContext->codec_id);
	if(codec==NULL)
		return false; // Codec not found

	m_codecContext = avcodec_alloc_context3(codec);
	if(avcodec_copy_context( m_codecContext, codecContext) != 0) {
	  fprintf(stderr, "Couldn't copy codec context");
	  return -1; // Error copying codec context
	}
	avcodec_close(codecContext);	
	if(avcodec_open2(m_codecContext, codec, NULL) < 0 )
		return false; // Could not open codec
	m_frame=av_frame_alloc();
	if (!m_frame) 
	{
        fprintf(stderr, "Could not allocate video m_frame\n");
        return false;
    }

	int w = m_codecContext->width;
	int h = m_codecContext->height;
	m_durationMicroSeconds = m_avFormatContext->duration;
	m_durationSeconds = (double)m_durationMicroSeconds / (double)AV_TIME_BASE;
	AVRational rational = m_codecContext->time_base;
	m_frameRate = av_q2d( m_avFormatContext->streams[m_videoStream]->avg_frame_rate );
	m_durationFrames = 1E-6 * m_durationMicroSeconds * m_frameRate;

	m_scaledFrame = av_frame_alloc();	
	m_dstPixelFormat = AV_PIX_FMT_RGB24;
	m_swsContext = sws_getContext( w, h,  m_codecContext->pix_fmt, w, h, m_dstPixelFormat, SWS_BICUBIC, NULL, NULL, NULL);
	if( m_swsContext == NULL )
	{
		fprintf(stderr, "Cannot initialize the conversion context!\n");
		return false;
	}

	uint8_t *buffer = NULL;
	int numBytes;
	// Determine required buffer size and allocate buffer
	numBytes=avpicture_get_size(AV_PIX_FMT_RGB24, m_codecContext->width,m_codecContext->height);
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	avpicture_fill((AVPicture *)m_scaledFrame, buffer, AV_PIX_FMT_RGB24,m_codecContext->width, m_codecContext->height);
	
	format.SetFormat( w, h, m_frameRate, VM_RGB24, VM_RGB24 ); 
	if ( aformat != NULL )
		*aformat = format;

	copyStringToChar( device.fileName, &identification.fileName );	
	copyStringToChar( "FFMPEG_VIDEO_FILE", &identification.identifier );	

//	while( !getFrame(false) );
	
}


char *VideoFile::getFrame( bool wait )
{
	if ( m_paused )
		return pixelBuffer;
	if ( pixelBuffer )
	{
		//cout << floor(m_lastFrameIndex + 0.5) << " " << " " << m_lastFramePos << " " << (double)av_gettime() / 1000000.0  << endl;			
		return pixelBuffer;
	}
	AVPacket packet;
	while ( pixelBuffer == NULL )
	{
		if (av_read_frame(m_avFormatContext, &packet) < 0)
		{
			return NULL;
		}
		if(packet.stream_index==m_videoStream) {
		// Decode video m_frame   
			int gotPicture;
			avcodec_decode_video2(m_codecContext, m_frame, &gotPicture, &packet);
			// Did we get a video m_frame?
			if(gotPicture) 
			{			 
				if (m_codecContext->pix_fmt != m_dstPixelFormat)
				{                       
					if (m_codecContext->pix_fmt != m_dstPixelFormat )            
					{
						m_lastFramePos = av_frame_get_best_effort_timestamp( m_frame ) * av_q2d(m_avFormatContext->streams[m_videoStream]->time_base);			
						m_lastFrameIndex = floor( m_lastFramePos * av_q2d( m_avFormatContext->streams[m_videoStream]->avg_frame_rate ) + 0.5 );
				//cout << floor(m_lastFrameIndex + 0.5) << " " << packet.dts << " " << packet.pts << " " << m_lastFramePos << " " << (double)av_gettime() / 1000000.0  << endl;			

						 sws_scale( m_swsContext, m_frame->data, 
								  m_frame->linesize, 0, 
								  m_codecContext->height, 
								  m_scaledFrame->data, m_scaledFrame->linesize);              
						 pixelBuffer = (char*)m_scaledFrame->data[0];
						 av_free_packet(&packet);
						 return pixelBuffer;
					}
				}
			}
		}
		av_free_packet(&packet);
	}
	return NULL;
}

void VideoFile::releaseFrame()
{
	pixelBuffer = NULL;

}

void VideoFile::goToMilisecond(  double milisecond )
{
	bool wasPaused = m_paused;
	m_paused = false;
	//(int64_t)(pos * AV_TIME_BASE)
	
	//av_seek_frame( m_avFormatContext, m_videoStream, 0, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD );	
	if ( milisecond < 0 )
		milisecond = 0;	
	double second = 0.001 * milisecond;
	if ( second > m_durationSeconds )
		second = m_durationSeconds;
	const int64_t pos = (int64_t)( second * AV_TIME_BASE);
	/*
	5631 5797 5.464
	5672 5756 5.506 KF
	3.670
	*/
	
	AVRational rational = {1, AV_TIME_BASE};
	int64_t seek_target =  av_rescale_q( pos, rational, m_avFormatContext->streams[m_videoStream]->time_base);		
	//cout << "seek_target " << seek_target << " " << m_avFormatContext->streams[m_videoStream]->start_time << endl;
	int ret = av_seek_frame( m_avFormatContext, m_videoStream, seek_target, AVSEEK_FLAG_BACKWARD  );
	//avformat_seek_file( m_avFormatContext, m_videoStream, seek_target,seek_target, seek_target, AVSEEK_FLAG_FRAME );
	avcodec_flush_buffers( m_codecContext );	
	do
	{
		releaseFrame();
		if ( !getFrame() )
			break;
	}while( !pixelBuffer || fabs( m_lastFramePos - second ) >= ( 0.5 / m_frameRate ) );
	//cout << "seek_target2 " << m_lastFramePos << endl;
	m_paused = wasPaused;
}

void VideoFile::goToFrame( int frame )
{
	bool wasPaused = m_paused;
	m_paused = false;
	//(int64_t)(pos * AV_TIME_BASE)	
	//av_seek_frame( m_avFormatContext, m_videoStream, 0, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD );
	double time = frame / m_frameRate;
	int64_t pos = (int64_t)(time * AV_TIME_BASE);	
	
	AVRational rational = {1, AV_TIME_BASE};
	int64_t seek_target =  av_rescale_q( pos, rational, m_avFormatContext->streams[m_videoStream]->time_base);		
	//cout << "seek_target " << seek_target << " " << m_avFormatContext->streams[m_videoStream]->start_time << endl;
	int ret = av_seek_frame( m_avFormatContext, m_videoStream, seek_target, AVSEEK_FLAG_BACKWARD  );
	//avformat_seek_file( m_avFormatContext, m_videoStream, seek_target,seek_target, seek_target, AVSEEK_FLAG_FRAME );
	avcodec_flush_buffers( m_codecContext );	
	do
	{
		releaseFrame();
		if ( !getFrame() )
			break;
	}while( !pixelBuffer || fabs( m_lastFramePos - time ) >= ( 0.5 / m_frameRate ) );
	//cout << "seek_target2 " << m_lastFramePos << endl;	
	m_paused = wasPaused;

}

double VideoFile::getPositionSeconds()
{
	return m_lastFramePos;	
}

int VideoFile::getPositionFrames()
{
	return m_lastFrameIndex;	
}

void VideoFile::pause()
{
	m_paused = true;
}

void VideoFile::play()
{
	m_paused = false;
}

double VideoFile::getLengthSeconds()
{
	return m_durationSeconds;
}

int VideoFile::getLengthFrames()
{
	return m_durationFrames;
}
