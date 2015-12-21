#ifndef VIDEOINPUTDSHOW_H
#define VIDEOINPUTDSHOW_H

#include "VideoInput.h"

#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__

#include "Qedit.h"
#include <streams.h>
//#include <objbase.h>
//#include <atlcomcli.h>	// CComPtr
//#include <atlbase.h>
//#include <process.h>
#include <comutil.h>	// _bstr_t
#include <deque>

class VideoInputDShow :
	public ISampleGrabberCB
{
public:
	VideoInputDShow(void);
	virtual ~VideoInputDShow(void);



//// ISampleGrabberCB interface methods -----------------
public:
	//IUnknown interface
	HRESULT WINAPI QueryInterface( REFIID iid, void** ppvObject );
	ULONG WINAPI AddRef();
	ULONG WINAPI Release();

	// ISampleGrabberCB interfaces
	virtual HRESULT WINAPI SampleCB(double SampleTime, IMediaSample *pSample) = 0;
	virtual HRESULT WINAPI BufferCB(double sampleTimeSec, BYTE* bufferPtr, long bufferLength) = 0;

private:
	unsigned long	fRefCount;
///// ---------------------------------------------------

protected:

	HRESULT initDirectShow(void);
	HRESULT freeDirectShow(void);
	HRESULT stopMedia();
	HRESULT EnableMemoryBuffer(unsigned int _maxConcurrentClients = 1,
                              unsigned int _allocatorBuffersPerClient = 2);
	HRESULT findMediaType(IPin *pin, VideoMan::VMInputFormat *requestedFormat, AM_MEDIA_TYPE *requestedMT );
	HRESULT findOneMediaType(IPin *pin, AM_MEDIA_TYPE *requestedMT );
	HRESULT findCaptureDevice( IBaseFilter ** ppSrcFilter, std::string &friendlyName, std::string &devicePath );
	
	IGraphBuilder *pGB;
	ICaptureGraphBuilder2 *pCGB;
	IMediaSeeking *pMS;
	IMediaControl *pMC;
	//IMediaPosition *pMP 	
	//IBasicVideo   *pBV;
	IBaseFilter   *pSG;
	ISampleGrabber *sampleGrabber;
	IBaseFilter *videoSource;

	#ifndef NDEBUG
		DWORD dwRegisterROT;
	#endif

	IMediaSample* mediaSample;
	IMediaSample* currentMediaSample;
};
#endif