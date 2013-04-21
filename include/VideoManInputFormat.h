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

/** \brief A structure to contain the identification of a video input.

	It is needed for creating a video input with VideoManControl::addVideoInput.
	In order to indentify a video input you need to know the module that will control it, for example VMDirectShow.
	Each module support at least one identifier, for example VMDirectShow supports "DSHOW_VIDEO_FILE" and "DSHOW_CAPTURE_DEVICE".
	The field identifier contains the identifier of the input module, for example "DSHOW_VIDEO_FILE". 
	- If you want to create a video input from a video file you need to fill the field fileName with its path "C:/videos/video.avi".
	- If you want to create it from a capture device you need to know its friendly name, for example "Logitec webcam", and its unique name.
	What this name represents depends on the input module; it could be the serial number, the device path or just an id. The unique name identifies
	cameras with the same friendly name.	
	
	You can get the list of available devices with VideoManControl::getAvailableDevices. Also you can get the list of supported identifier with VideoManControl::getSupportedIdentifiers.	
*/
struct VMInputIdentification
{
	/** Path of a video file */
	char* fileName; 
	/** Friendly name of a capture device */
	char* friendlyName; 
	/** Unique name of a capture device */
	char* uniqueName; 
	/** Identifier of the videoman module that controls the input */
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

/** \brief A class to contain the format of a video input

	When you create a video input with VideoManControl::addVideoInput you can indicate some parameters, like resolution, frame rate, 
	if you want to reproduce sound in case of a video file, etc... If want to choose the format from a dialog you can use the field showDlg.
	
	You can get the format of an initialized video input with VideoManControl::getFormat
*/
class VIDEOMAN_API VMInputFormat
{
public:
	VMInputFormat(void);
	VMInputFormat(VMInputFormat const &format);
	virtual ~VMInputFormat(void);
	VMInputFormat &operator=( VMInputFormat const &format );

	/** \brief Configure a format with resolution, frame rate and pixel formats
		\param awidth [in] Resolution width
		\param aheight [in] Resolution wiheightdth
		\param afps [in] Frame rate
		\param afps [in] Frame rate
		\param apixelFormat [in] deprecated 
		\param apixelFormatOut [in] The pixel format of the image
	*/
	bool SetFormat( int awidth, int aheight, double afps, VMPixelFormat apixelFormat, VMPixelFormat apixelFormatOut );

	/** \brief Configure the pixel format
		\param apixelFormatIn [in] deprecated 
		\param apixelFormatOut [in] The pixel format of the image
	*/
	bool setPixelFormat( VMPixelFormat apixelFormatIn, VMPixelFormat apixelFormatOut );

	/** \brief Get pixel format (deprecated)
		\return the pixel format		
	*/
	VMPixelFormat getPixelFormatIn() const;

	/** \brief Get pixel format of the image
		\return the pixel format		
	*/
	VMPixelFormat getPixelFormatOut() const;

	/** \brief Check is the format is valid
		\return True if the format is valid
	*/
	bool validFormat();	

	//! \name Image format
	//@{
	/** Number of channels per pixel */
	int nChannels;
	/** Number of bits per channel */
	int depth;
	/** Resolution width */
	int width;
	/** Resolution height */
	int height;
	/** Memory buffer alignment(1,2,4,8) */
	int align;
	/** frames per second */
	double fps;
	//@}

	//! \name For capture devices
	//@{
	/** Show init dialog to select format */
	bool showDlg;
	//@}

	//! \name For video files
	//@{
	/**  Use clock to reproduce the video with its native framerate or as fast as possible */
	bool clock;
	/**  Reproduce the audio channel */
	bool renderAudio; 
	//@}

private:
	VMPixelFormat formatIn;
	VMPixelFormat formatOut;
};
};
#endif