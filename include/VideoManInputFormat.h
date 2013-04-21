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

/** \brief Identification of a video input
*/
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

/** \brief Pixel Formats
*/
enum VMPixelFormat { VM_RGB24, VM_RGB32, VM_BGR24, VM_BGR32, VM_YUV422, VM_YUV411, VM_IYUV, VM_GREY8, VM_GREY16, VM_RAW8, VM_RAW16, VM_UNKNOWN};

/** \brief Format of a video input
*/
class VIDEOMAN_API VMInputFormat
{
public:
	VMInputFormat(void);
	VMInputFormat(VMInputFormat const &format);
	virtual ~VMInputFormat(void);
	VMInputFormat &operator=( VMInputFormat const &format );

	bool SetFormat( int awidth, int aheight, double afps, VMPixelFormat apixelFormatIn, VMPixelFormat apixelFormatOut );
	bool setPixelFormat( VMPixelFormat apixelFormatIn, VMPixelFormat apixelFormatOut );
	VMPixelFormat getPixelFormatIn() const;
	VMPixelFormat getPixelFormatOut() const;
	bool validFormat();	

	int nChannels;  //Number of channels
	int depth;		//bits per channel
	int width;		//resolution width
	int height;		//resolution height
	int align;		//Memory buffer alignment(1,2,4,8)
	double fps;		//frames per second

	bool showDlg;	//(capture device) Show init dialog to select format

	bool clock; //(video file) Use clock to reproduce the video with its corresponding framerate or not
	bool renderAudio; //(video file) Render the audio channel	

private:
	VMPixelFormat formatIn;
	VMPixelFormat formatOut;
};
};
#endif