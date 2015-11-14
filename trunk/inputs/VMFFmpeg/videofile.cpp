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
	m_lastFrameIndex = 0;
	m_lastFramePos = 0.0;
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
	for(unsigned int i=0; i<m_avFormatContext->nb_streams; i++)
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
	  return false; // Error copying codec context
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
	if ( m_avFormatContext->duration != AV_NOPTS_VALUE )
		m_durationMicroSeconds = m_avFormatContext->duration;
	else
		m_durationMicroSeconds = m_avFormatContext->streams[m_videoStream]->duration;
	m_durationSeconds = (double)m_durationMicroSeconds / (double)AV_TIME_BASE;
	AVRational rational = m_codecContext->time_base;
	m_frameRate = av_q2d( av_guess_frame_rate( m_avFormatContext, m_avFormatContext->streams[m_videoStream], NULL ) );//m_avFormatContext->streams[m_videoStream]->avg_frame_rate );
	m_durationFrames = static_cast<int>( floor( 1E-6 * m_durationMicroSeconds * m_frameRate + 0.5 ) );
	if ( m_avFormatContext->streams[m_videoStream]->nb_frames > 0 )
		m_durationFrames = m_avFormatContext->streams[m_videoStream]->nb_frames;
	
	m_scaledFrame = av_frame_alloc();	
	m_dstPixelFormat = AV_PIX_FMT_RGB24;
	//Change pix format because of the warning "deprecated pixel format used, make sure you did set range correctlly"
		AVPixelFormat pixFormat;
	switch (m_codecContext->pix_fmt) {
	case AV_PIX_FMT_YUVJ420P :
		pixFormat = AV_PIX_FMT_YUV420P;
		break;
	case AV_PIX_FMT_YUVJ422P  :
		pixFormat = AV_PIX_FMT_YUV422P;
		break;
	case AV_PIX_FMT_YUVJ444P   :
		pixFormat = AV_PIX_FMT_YUV444P;
		break;
	case AV_PIX_FMT_YUVJ440P :
		pixFormat = AV_PIX_FMT_YUV440P;
	default:
		pixFormat = m_codecContext->pix_fmt;
		break;
	}
	m_swsContext = sws_getContext( w, h,  pixFormat, w, h, m_dstPixelFormat, SWS_BICUBIC, NULL, NULL, NULL);
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
	return true;
}

char *VideoFile::getFrame( bool wait )
{
	if ( m_paused || pixelBuffer )
		return pixelBuffer;
	AVPacket packet;
	while ( pixelBuffer == NULL )
	{
		if ( av_read_frame(m_avFormatContext, &packet) < 0 )
			return NULL;
		if ( packet.stream_index == m_videoStream )
		{
			// Decode frame   
			int gotPicture = 0 ;
			avcodec_decode_video2(m_codecContext, m_frame, &gotPicture, &packet);			
			if ( gotPicture && m_codecContext->pix_fmt != m_dstPixelFormat ) 
			{
				if ( m_avFormatContext->streams[m_videoStream]->start_time != AV_NOPTS_VALUE )
					m_lastFramePos = ( av_frame_get_best_effort_timestamp( m_frame ) - m_avFormatContext->streams[m_videoStream]->start_time ) * av_q2d(m_avFormatContext->streams[m_videoStream]->time_base);			
				else
					m_lastFramePos = ( av_frame_get_best_effort_timestamp( m_frame ) ) * av_q2d(m_avFormatContext->streams[m_videoStream]->time_base);			
				m_lastFrameIndex = static_cast<int>( floor( m_lastFramePos * m_frameRate + 0.5 ) );
				//cout << m_lastFrameIndex << " " << av_frame_get_best_effort_timestamp( m_frame ) << " " << packet.dts * av_q2d(m_avFormatContext->streams[m_videoStream]->time_base) << " " << packet.pts * av_q2d(m_avFormatContext->streams[m_videoStream]->time_base) << " " << m_lastFramePos << " " << (double)av_gettime() / 1000000.0  << endl;			
				//cout << m_lastFrameIndex << " " << m_lastFramePos << endl;
				sws_scale( m_swsContext, m_frame->data, m_frame->linesize, 0, m_codecContext->height,  m_scaledFrame->data, m_scaledFrame->linesize );
				pixelBuffer = (char*)m_scaledFrame->data[0];
				av_free_packet(&packet);
				return pixelBuffer;
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

void VideoFile::seek(  double seekSecond )
{
	if ( seekSecond < 0 )
		seekSecond = 0;	
	if ( seekSecond > m_durationSeconds )
		seekSecond = m_durationSeconds;	
	AVRational rational = {1, 1};
	int64_t seek_target =  av_rescale_q( seekSecond, rational, m_avFormatContext->streams[m_videoStream]->time_base);
	//Seek to the closest keyframe previous to seektime
	if ( av_seek_frame( m_avFormatContext, m_videoStream, seek_target, AVSEEK_FLAG_BACKWARD  ) >= 0 )
	{
		avcodec_flush_buffers( m_codecContext );	
		//decode frames until reach seekSecond
		do
		{
			releaseFrame();
			if ( !getFrame() )
				break;
			if ( m_lastFramePos > seekSecond )
				break;
		}while( !pixelBuffer || fabs( m_lastFramePos - seekSecond ) >= ( 0.5 / m_frameRate ) );	
	}
}

void VideoFile::goToMilisecond(  double seekMilisecond )
{
	bool wasPaused = m_paused;
	m_paused = false;
	const double seekSecond = 0.001 * seekMilisecond;	
	seek( seekSecond );
	m_paused = wasPaused;
}

void VideoFile::goToFrame( int frame )
{
	bool wasPaused = m_paused;
	m_paused = false;	
	const double seekSecond = frame / m_frameRate;
	seek( seekSecond );
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
