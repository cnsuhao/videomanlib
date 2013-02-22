#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated

#include <iostream>
#include <string>

#include "ImageSequence.h"

#include "highgui.h"

using namespace std;
using namespace VideoMan;

ImgSeq::ImgSeq(void)
{
	img = NULL;
}

ImgSeq::~ImgSeq(void)
{
	#ifdef WIN32		
		FindClose( hFind);
		/*if ( threadHandle != NULL )
		{
			TerminateThread(threadHandle, 0);
			CloseHandle( threadHandle );
		}*/
	#endif
	#ifdef linux
		closedir( dir );
	#endif
	cvReleaseImage(&img);
	
}

#ifdef WIN32
bool ImgSeq::initSequence( const char *adirPath, VMInputFormat *aFormat )
{
	if ( !adirPath )
		return false;
	WIN32_FIND_DATA fd;
	DWORD dwAttr = FILE_ATTRIBUTE_ARCHIVE;	
	dirPath = adirPath;
	string path = dirPath + "\\*.*";
	hFind = FindFirstFile( path.c_str(), &fd);
	if(hFind == INVALID_HANDLE_VALUE )
	{
		FindClose( hFind);
		return false;
	}
	do
	{
		if ( fd.dwFileAttributes & dwAttr )
		{
			string fileName = fd.cFileName;			 
			if ( img = cvLoadImage( (dirPath + "/" + fileName).c_str() ) )
			{
				format.width = img->width;
				format.height = img->height;
				string channelSeq = img->channelSeq;
				if ( img->nChannels == 3 && img->depth == 8 && channelSeq == "BGR" )
					format.setPixelFormat( BGR24, BGR24 );
				else if ( img->nChannels == 3 && img->depth == 8  )
					format.setPixelFormat( RGB24, RGB24 );
				else if ( img->nChannels == 1 && img->depth == 8)
					format.setPixelFormat( GREY8, GREY8 );
				else if ( img->nChannels == 1 && img->depth == 16)
					format.setPixelFormat( GREY16, GREY16 );
				else
					return false;
				if ( img->origin == 0 )
					cvFlip( img, img, 0 );
			}
		}
	}while( img == NULL && FindNextFile( hFind, &fd) );	

	if ( img ) 
	{
		pixelBuffer = img->imageData;
		if ( aFormat )
			*aFormat = format;

		const string identifier = "IMAGE_SEQUENCE";
		identification.identifier = new char[identifier.length() + 1];
		strcpy( identification.identifier, identifier.c_str() );

		return true;
	}
	return false;
}

unsigned long WINAPI run(void *parameters)
{
	ImgSeq *pObject = static_cast<ImgSeq*>(parameters);
	pObject->readNextImage();
	return 0;
}

void ImgSeq::readNextImage()
{
	if(hFind != INVALID_HANDLE_VALUE )
	{
		WIN32_FIND_DATA fd;
		DWORD dwAttr = FILE_ATTRIBUTE_ARCHIVE;
		bool newImage = false;
		while(  !newImage && FindNextFile( hFind, &fd) )
		{
			//if ( fd.dwFileAttributes & dwAttr )
			{
				string fileName = fd.cFileName;			 
				IplImage *img2;
				if ( img2 = cvLoadImage( (dirPath + "/" + fileName).c_str() ) )
				{
					if ( img->width == img2->width && img->height == img2->height && img->depth == img2->depth && img->nChannels == img2->nChannels )
					{		
						/*CRITICAL_SECTION csNewEvent;
						InitializeCriticalSection(&csNewEvent);
						EnterCriticalSection(&csNewEvent);
						pixelBuffer = NULL;
						LeaveCriticalSection(&csNewEvent);
						DeleteCriticalSection(&csNewEvent);*/
						cvReleaseImage( &img );
						img = img2;
						if ( img->origin == 0 )
							cvFlip( img, img, 0 );
						pixelBuffer = img->imageData;
						newImage = true;
					}
					else
						cvReleaseImage( &img2 );
				}
			}
		}
	}	
}

void ImgSeq::releaseFrame( )
{	
	if ( pixelBuffer == NULL )
		return;
	pixelBuffer = NULL;
	unsigned long threadID;	
	threadHandle = CreateThread(NULL, 0, run, this, 0, &threadID); // Creates and launches the thread		
}

#endif

#ifdef linux
bool ImgSeq::initSequence( const char *adirPath, VMInputFormat *aFormat )
{
	dirPath = adirPath;
	struct dirent *dent;
	struct stat stbuf;
	char buf[PATH_MAX];
	if(!(dir = opendir(dirPath.c_str())))
	{
		perror("opendir()");
		return false;
	}
	while( img == NULL && ( dent = readdir( dir ) ) )
	{
		std::string fileName = dent->d_name;			 
		if ( img = cvLoadImage( (dirPath + "/" + fileName).c_str() ) )
		{
			format.width = img->width;
			format.height = img->height;
			if ( img->nChannels == 3 && img->depth == 8 )
				format.setPixelFormat( RGB24, RGB24 );
			else if ( img->nChannels == 1 && img->depth == 8)
				format.setPixelFormat( GREY8, GREY8 );
			else if ( img->nChannels == 1 && img->depth == 16)
				format.setPixelFormat( GREY16, GREY16 );
			else
				return false;			
		}
	}
	if ( img ) 
	{
		pixelBuffer = img->imageData;
		if ( aFormat )
			*aFormat = format;
		return true;
	}
	return false;
}

void *run(void *parameters)
{
	ImgSeq *pObject = static_cast<ImgSeq*>(parameters);
	pObject->readNextImage();
}

void ImgSeq::readNextImage()
{
	if( !dir )			
		return;
	struct dirent *dent;
	bool newImage = false;
	while( !newImage && ( dent = readdir( dir ) ) )
	{
		std::string fileName = dent->d_name;			 
		IplImage *img2;
		if ( img2 = cvLoadImage( (dirPath + "/" + fileName).c_str() ) )
		{
			if ( img->width == img2->width && img->height == img2->height && img->depth == img2->depth && img->nChannels == img2->nChannels )
			{
				cvReleaseImage( &img );
				img = img2;
				pixelBuffer = img->imageData;
				newImage = true;
			}
			else
				cvReleaseImage( &img2 );
		}
	}
	pthread_exit(NULL);
}

void ImgSeq::releaseFrame( )
{
	if ( pixelBuffer == NULL )
		return;
	pixelBuffer = NULL;
	pthread_create( &thread, NULL, run, (void*)this );
}
#endif

char *ImgSeq::getFrame( bool wait)
{
	return pixelBuffer;
}

int ImgSeq::getLengthFrames()
{
	return 0;
}
	
double ImgSeq::getLengthSeconds()
{
	return 0.0;
}
