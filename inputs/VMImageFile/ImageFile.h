#ifndef IMAGESEQUENCE_H
#define IMAGESEQUENCE_H

#include "VideoInput.h"
#include "VideoManInputFormat.h"
#include "cxcore.h"

#ifdef WIN32
	#include <windows.h>
#endif
#ifdef linux
	#include <stddef.h>
	#include <sys/stat.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <dirent.h>
#endif

class ImageFile :
	public VideoManPrivate::VideoInput
{
public:
	ImageFile(void);
	virtual ~ImageFile(void);

	bool initInput( const char *imagePath, VideoMan::VMInputFormat *aFormat = NULL );
		
	char *getFrame( bool wait = false);

	void releaseFrame( );

	int getLengthFrames();
	
	double getLengthSeconds();	

	void readNextImage();


	double getPositionSeconds(){return 0.0;}
	
	int getPositionFrames(){return 0;}

	void goToFrame( int frame ){};
	
	void goToMilisecond( double milisecond ){};

private:
	
	cv::Mat m_img;
	std::string m_imagePath;
};

#endif