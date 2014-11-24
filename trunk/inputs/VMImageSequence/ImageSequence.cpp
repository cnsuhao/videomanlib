#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated

#include <iostream>
#include <string>
#include <list>

#include "ImageSequence.h"

#include "highgui.h"

using namespace std;
using namespace VideoMan;

ImgSeq::ImgSeq(void)
{
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
	#endif	
	
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
			img = cv::imread( (dirPath + "/" + fileName).c_str(), CV_LOAD_IMAGE_ANYCOLOR );
			if ( !img.empty() )
			{				
				format.width = img.cols;
				format.height = img.rows;				
				if ( img.channels() == 3 && img.type() == CV_8UC3 )
					format.setPixelFormat( VM_BGR24, VM_BGR24 );
				else if ( img.channels() == 3 && img.type() == CV_8UC3 )
					format.setPixelFormat( VM_RGB24, VM_RGB24 );
				else if ( img.channels() == 1 && img.type() == CV_8UC1 )
					format.setPixelFormat( VM_GREY8, VM_GREY8 );
				else if ( img.channels() == 1 && img.type() == CV_16UC1 )
					format.setPixelFormat( VM_GREY16, VM_GREY16 );
				else
					return false;
				format.align = 1;
				///if ( img->origin == 0 )
				///	cvFlip( img, img, 0 );
				cv::flip( img, img, 0 );
			}
		}
	}while( img.empty() && FindNextFile( hFind, &fd) );	

	if ( !img.empty() ) 
	{
		pixelBuffer = (char*)img.data;
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
				string filePath= dirPath + "/" + fileName;
				cv::Mat img2 = cv::imread( filePath, CV_LOAD_IMAGE_ANYCOLOR );			
				if ( !img2.empty() )
				{
					if ( img.cols == img2.cols && img.rows == img2.rows && img.depth() == img2.depth() && img.channels() == img2.channels() )
					{		
						/*CRITICAL_SECTION csNewEvent;
						InitializeCriticalSection(&csNewEvent);
						EnterCriticalSection(&csNewEvent);
						pixelBuffer = NULL;
						LeaveCriticalSection(&csNewEvent);
						DeleteCriticalSection(&csNewEvent);*/
						img = img2;
						//if ( img->origin == 0 )
							//cvFlip( img, img, 0 );
						cv::flip( img, img, 0 );
						pixelBuffer = (char*)img.data;
						newImage = true;
					}
					//else
						//cvReleaseImage( &img2 );
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


int alphasort2( const struct dirent **d1,
               const struct dirent **d2)
{
	std::string a = (*d1)->d_name;
	std::string b = (*d2)->d_name;
	if ( a == b )
	return 0;
	std::list<string> list;
	list.push_back( a );
	list.push_back( b );
	list.sort();	
	if ( list.front() == a )
		return 1;	
   return -1;
}
			   
bool ImgSeq::initSequence( const char *adirPath, VMInputFormat *aFormat )
{
	numImage = 0;
	dirPath = adirPath;	
	struct stat stbuf;
	char buf[PATH_MAX];
	if(!(dir = opendir(dirPath.c_str())))
	{
		perror("opendir()");
		return false;
	}
	//while( img.empty() && ( dent = readdir( dir ) ) )	
	numImage = scandir( dirPath.c_str(), &dent, NULL, alphasort2 );	
	while( img.empty() && --numImage >= 0 )	
	{
		std::string fileName = dent[numImage]->d_name;			 		
		img = cv::imread( (dirPath + "/" + fileName).c_str(), CV_LOAD_IMAGE_ANYCOLOR );
		if ( !img.empty() )
		{		
			format.width = img.cols;
			format.height = img.rows;
			if ( img.channels() == 3 && img.type() == CV_8UC3 )
				format.setPixelFormat( VM_BGR24, VM_BGR24 );
			else if ( img.channels() == 3 && img.type() == CV_8UC3 )
				format.setPixelFormat( VM_RGB24, VM_RGB24 );
			else if ( img.channels() == 1 && img.type() == CV_8UC1 )
				format.setPixelFormat( VM_GREY8, VM_GREY8 );
			else if ( img.channels() == 1 && img.type() == CV_16UC1 )
				format.setPixelFormat( VM_GREY16, VM_GREY16 );
			else
				return false;
			//if ( img->origin == 0 )
//				cvFlip( img, img, 0 );

		}		
	}
	if ( !img.empty() ) 
	{
		pixelBuffer = (char*)img.data;
		if ( aFormat )
			*aFormat = format;
		
		const string identifier = "IMAGE_SEQUENCE";
		identification.identifier = new char[identifier.length() + 1];
		strcpy( identification.identifier, identifier.c_str() );
		
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
	bool newImage = false;			
	while( !newImage && --numImage >= 0 )
	{
		std::string fileName = dent[numImage]->d_name;			 		
		cv::Mat img2 = cv::imread( (dirPath + "/" + fileName).c_str(), CV_LOAD_IMAGE_ANYCOLOR );					
		{
			if ( img.cols == img2.cols && img.rows == img2.rows && img.depth() == img2.depth() && img.channels() == img2.channels() )
			{
				img = img2;
				//if ( img->origin == 0 )
					//cvFlip( img, img, 0 );
				pixelBuffer = (char*)img.data;
				newImage = true;
			}
			//else
				//cvReleaseImage( &img2 );
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
