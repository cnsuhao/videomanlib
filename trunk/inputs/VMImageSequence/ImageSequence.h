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

class ImgSeq :
	public VideoManPrivate::VideoInput
{
public:
	ImgSeq(void);
	virtual ~ImgSeq(void);

	bool initSequence( const char *adirPath, VideoMan::VMInputFormat *aFormat = NULL );
		
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
	
	IplImage *img;
	
	std::string dirPath;

	#ifdef WIN32
		HANDLE hFind;
		HANDLE threadHandle;
	#endif

	#ifdef linux
		DIR *dir;
		pthread_t thread;
	#endif
};

#endif