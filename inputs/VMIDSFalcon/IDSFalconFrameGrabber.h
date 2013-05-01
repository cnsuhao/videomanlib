#pragma once

#ifdef linux
	#include <pthread.h>
#endif

#include "VideoManInputFormat.h"
#include "falcon.h"
#include "VideoInput.h"

#include <vector>

void copyStringToChar( const std::string &src, char **dst );

void freeChar( char **src );


class IDSFalconFrameGrabber :
	public VideoManPrivate::VideoInput
{
public:
	IDSFalconFrameGrabber(void);
	virtual ~IDSFalconFrameGrabber(void);

	bool initBoard( const unsigned long &hids, VideoMan::VMInputFormat *aFormat );

	char *getFrame( bool wait = false);

	inline void releaseFrame();

	void showPropertyPage();

	void play();

	void pause();

	void stop();

	static void getAvailableDevices( VideoMan::VMInputIdentification **_deviceList, int &numDevices  );

	void setFrameCallback( VideoManPrivate::VideoInput::getFrameCallback theCallback, void *data );

	bool supportFrameCallback()
	{
		#ifdef WIN32
			return true;
		#endif
		return false;
	}

	typedef struct
	{
		HIDS           hf;

		#ifdef WIN32
			HANDLE         hEvent;
			HANDLE         hThread;
		#endif
		#ifdef linux
			pthread_cond_t hEvent;
			pthread_t hThread;
		#endif

		bool           bKill;
		int            nXSize;
		int            nYSize;
		getFrameCallback callback;
		std::vector<char *> imageMem;
		std::vector<int> imageMemID;
		int inputID;
		void *frameCallbackData;
		int *lastSeqIndex;
		int numRepeatLastNum;
		char **pixelBuffer;

	} IDS_SWITCH_THREAD;


private:
	HIDS m_hBoard; //board indentifier
	std::vector<char *> m_pcImageMem; //image
	std::vector<int> m_nImageMemID;
	std::vector<int> m_nSeqNumId;
	int m_lastSeqIndex;
	int colorMode;

	IDS_SWITCH_THREAD m_info;

	int colorModeFromPixelFormat( VideoMan::VMPixelFormat depth );
	VideoMan::VMPixelFormat pixelFormatFromColorMode( int colorMode );
	int m_lastNum;
	int m_numRepeatLastNum;
};
