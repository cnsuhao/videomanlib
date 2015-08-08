#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated

#include <iostream>
#include <string>

#include "ImageFile.h"

#include "highgui.h"

using namespace std;
using namespace VideoMan;

ImageFile::ImageFile(void)
{	
}

ImageFile::~ImageFile(void)
{
}

bool ImageFile::initInput( const char *imagePath, VMInputFormat *aFormat )
{
	if ( !imagePath )
		return false;

	m_imagePath = imagePath;
	m_img = cv::imread( m_imagePath, CV_LOAD_IMAGE_COLOR );
	if ( !m_img.empty() )
	{
		format.width = m_img.cols;
		format.height = m_img.rows;
		const int channels = m_img.channels();
		const int depth = m_img.depth();
		//string channelSeq = m_img.channelSeq;		
		/*if ( img->nChannels == 3 && img->depth == 8 && channelSeq == "BGR" )
			format.setPixelFormat( BGR24, BGR24 );
		else if ( img->nChannels == 3 && img->depth == 8  )
			format.setPixelFormat( RGB24, RGB24 );
		else if ( img->nChannels == 1 && img->depth == 8)
			format.setPixelFormat( GREY8, GREY8 );
		else if ( img->nChannels == 1 && img->depth == 16)
			format.setPixelFormat( GREY16, GREY16 );
		else
			return false;*/
		format.setPixelFormat( VM_RGB24, VM_RGB24 );
		/*if ( channels == 1 && depth == IPL_DEPTH_8U && depth == IPL_DEPTH_8S )
			format.setPixelFormat( GREY8, GREY8 );
		if ( channels == 1 && depth == IPL_DEPTH_16U && depth == IPL_DEPTH_16S )
			format.setPixelFormat( GREY16, GREY16 );
		if ( channels == 3 && depth == IPL_DEPTH_8U && depth == IPL_DEPTH_8S )
			format.setPixelFormat( RGB24, RGB24 );
		if ( channels == 4 && depth == IPL_DEPTH_8U && depth == IPL_DEPTH_8S )
			format.setPixelFormat( RGB32, RGB32 );
		else
			return false;*/

			
		//if ( m_img.origin() == 0 )
		//	cvFlip( img, img, 0 );
		
		pixelBuffer = (char*)m_img.data;
		if ( aFormat )
			*aFormat = format;

		const string identifier = "IMAGE_FILE";
		identification.identifier = new char[identifier.length() + 1];
		strcpy( identification.identifier, identifier.c_str() );
		return true;
	}
	return false;
}

void ImageFile::releaseFrame( )
{
}

char *ImageFile::getFrame( bool wait)
{
	return pixelBuffer;
}

int ImageFile::getLengthFrames()
{
	return 0;
}

double ImageFile::getLengthSeconds()
{
	return 0.0;
}