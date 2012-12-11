#ifndef VIDEOMANINPUTFORMAT_H
#define VIDEOMANINPUTFORMAT_H

#ifdef WIN32
#ifdef VideoMan_EXPORTS
#define VIDEOMAN_API __declspec(dllexport)
#else
#define VIDEOMAN_API __declspec(dllimport)
#endif
#endif
#ifdef linux
#define VIDEOMAN_API 
#endif

namespace VideoMan
{
struct VMInputIdentification
{
	char* fileName;
	char* friendlyName;
	char* uniqueName;
	char* identifier;

	VMInputIdentification()
	{
		fileName = 0;
		friendlyName = 0;
		uniqueName = 0;		
		identifier = 0;
	}
};

enum VMPixelFormat { RGB24, RGB32, BGR24, BGR32, YUV422, YUV411, IYUV, GREY8, GREY16, RAW8, UNKNOWN};

class VIDEOMAN_API VMInputFormat
{
public:
	VMInputFormat(void);
	VMInputFormat(VMInputFormat const &format);
	virtual ~VMInputFormat(void);
	VMInputFormat &operator=( VMInputFormat const &format );

	bool SetFormat( int awidth, int aheight, double afps, VMPixelFormat apixelFormatIn, VMPixelFormat apixelFormatOut );
	bool setPixelFormat( VMPixelFormat apixelFormatIn, VMPixelFormat apixelFormatOut );
	VMPixelFormat getPixelFormatIn();
	VMPixelFormat getPixelFormatOut();
	bool validFormat();	

	int nChannels;  //Number of channels
	int depth;		//bits per channel
	int width;		//resolution width
	int height;		//resolution height
	int align;		//Memory buffer alignment(1,2,4,8)
	double fps;		//frames per second

	bool showDlg;	//(capture device) Show init dialog to select format
	
	bool dropFrames; //drop frames while playing or not	
	bool clock; //(video file) Use clock to reproduce the video with its corresponding framerate or not
	bool renderAudio; //(video file) Render the audio channel	

private:
	VMPixelFormat formatIn;
	VMPixelFormat formatOut;
};
};
#endif