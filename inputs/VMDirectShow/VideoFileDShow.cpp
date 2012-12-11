#include "StdAfx.h"


#include <iostream>
#include <math.h>
#include "assert.h"
#include "VideoFileDShow.h"
#include "DirectShowHelpers.h"

using namespace std;
using namespace VideoMan;

VideoFileDShow::VideoFileDShow(void)
{
	paused = true;
	LastSampleTime = 0;
}

VideoFileDShow::~VideoFileDShow(void)
{
	stopMedia();
	callback = NULL;
	SAFE_RELEASE( mediaSample );
}


bool VideoFileDShow::loadVideoFile( const char *file, VMInputFormat *aFormat )
{	
	if ( file == NULL )
		return false;
	// Initialize DirectShow and query for needed interfaces
    HRESULT hr = initDirectShow();	
    if ( FAILED( hr ) )
    {
		freeDirectShow();
		cerr << "Failed to initialize DirectShow!" << endl;
		return false;
    }

	const string name = file;
	hr = prepareMedia( name, aFormat );
    if (FAILED(hr))
    {
		freeDirectShow();     
		cerr << "Failed preparing media! " << endl;		
		cerr << "fileName: " << name << endl;
        return false;
    }
	copyStringToChar( name, &identification.fileName );	
	copyStringToChar( "DSHOW_VIDEO_FILE", &identification.identifier );	
	//Set the time format
	//Always use TIME_FORMAT_MEDIA_TIME,problems with TIME_FORMAT_FRAME
	//In getPosition check format.timeFormat and update the output
	hr = pMS->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );

	LONGLONG duration;	
	hr = pMS->GetDuration( &duration );		
	videoLengthSeconds = referenceTime2seconds( duration);	
	videoLengthFrames = static_cast<int>( floor( static_cast<double>( duration ) / avgTimePerFrame ) );	
	
	/*hr = pMC->Run();
	if ( FAILED( hr ) )
	{
		cerr << "Failed running file " << name << endl;
		freeDirectShow();
		return false;
	}*/

	frameCaptured = false;

	if ( format.width<=0 || format.height<=0 )
	{
		freeDirectShow();
		return false;
	}
	if (format.fps<=0)
		format.fps=30;

	if ( aFormat != NULL )	
		*aFormat = format;

	return true;
}

HRESULT VideoFileDShow::prepareMedia( const std::string &name, VMInputFormat *aFormat  )
{
    HRESULT hr = S_OK;

	if ( aFormat )
		format = *aFormat;
  	
	/*** Add the source filter ***/
	//std::string filterName = "CLSID_CAVIFileSynth";	
	if ( name.find( "http://" ) != std::string::npos )	
	{
		//CLSID_AsyncReader //CLSID_WMAsfReader //CLSID_CAVIFileSynth
		if ( FAILED(hr = CoCreateInstance(CLSID_WMAsfReader, NULL, CLSCTX_INPROC_SERVER, 
				IID_IBaseFilter, (void**)&(videoSource))))
			return hr;	
		
		IFileSourceFilter *pFileSource = NULL;
		if ( FAILED(videoSource->QueryInterface(IID_IFileSourceFilter,(void**)&pFileSource))) 
			return hr;
		if ( FAILED( hr = pGB->AddFilter(videoSource, L"File Reader")))
		{
			SAFE_RELEASE( pFileSource );
			return hr;
		}
		if ( FAILED( hr = pFileSource->Load( (LPCWSTR)bstr_t(name.c_str()), NULL ) ) )
		{
			SAFE_RELEASE( pFileSource );
			return hr;
		}
		SAFE_RELEASE( pFileSource );
	}
	else
	{
		if ( FAILED( hr = pGB->AddSourceFilter( (LPCWSTR)bstr_t(name.c_str()), NULL, &videoSource ) ) )
			return hr;
	}	

	/*** Get the output pin of the video source filter ***/
	IPin *sourceOutPin = NULL;
	if ( FAILED( hr = getPin(videoSource, PINDIR_OUTPUT, MEDIATYPE_Video, &sourceOutPin ) ) )
	{
		if ( FAILED( hr = getPin(videoSource, PINDIR_OUTPUT, MEDIATYPE_Stream, &sourceOutPin ) ) )
		{
			//If the video output pin was not found, take the first pin
			if ( FAILED(hr = getPin(videoSource,PINDIR_OUTPUT,1, &sourceOutPin)))
				return hr;
		}
	}

	/*** Add the NULL renderer ***/
	IBaseFilter *pVideoRenderer = NULL;
	if ( FAILED( hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&(pVideoRenderer) ) ) )
		return hr;
	if ( FAILED ( hr = pGB->AddFilter(pVideoRenderer, L"NULL Video Renderer") ) )
	{
		SAFE_RELEASE( pVideoRenderer );
		return hr;	
	}

	/*** Add the sample grabber ***/	
	if ( FAILED( hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&pSG) ) )
		return hr;	
	if ( FAILED( hr = pSG->QueryInterface(IID_ISampleGrabber,(void**)&sampleGrabber)) )
		return(hr);

	/*** Set sampleGrabber format ***/
	AM_MEDIA_TYPE mt;
	ZeroMemory((PVOID)&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.formattype = GUID_NULL;
	if ( aFormat != NULL )	
		mt.subtype = translatePIXEL_FORMAT( aFormat->getPixelFormatOut() );
	else
		mt.subtype = translatePIXEL_FORMAT( RGB24 );
	if ( FAILED( hr = sampleGrabber->SetMediaType(&mt) ) )
		return hr;
	if ( FAILED( hr = pGB->AddFilter(pSG, L"Sample Grabber") ) )
		return hr;
	freeMediaType(mt);
	
	/*** Get the input and output pins of the samplegrabber ****/
	IPin *sampleGrabberPinIn = NULL;
	if ( FAILED( hr = getPin( pSG, PINDIR_INPUT, 1, &sampleGrabberPinIn ) ) ) 
		return(hr);	
	IPin *sampleGrabberPinOut = NULL;
	if ( FAILED( hr = getPin( pSG, PINDIR_OUTPUT, 1, &sampleGrabberPinOut ) ) ) 	
		return(hr);
	
	/*** Get the input pin of the renderer ****/
	IPin *rendererPin = NULL;
	if ( FAILED ( hr = getPin( pVideoRenderer, PINDIR_INPUT, 1, &rendererPin ) ) )
		return(hr);

	//Conect the splitter and the grabber
//	if ( FAILED( hr = autoConnectFilters(videoSource,1,pSG,1,pGB) )	 )
//		return hr;
	//Conect the grabber and the renderer
//	if ( FAILED( hr = autoConnectFilters(pSG,1,pVideoRenderer,1,pGB) ) )
//		return hr;	
	//Conect VideoSource and the splitter
	
	/*** Connect the source filter and the grabber ***/
	if( FAILED( hr = pGB->Connect( sourceOutPin, sampleGrabberPinIn ) ) )
		return hr;
	SAFE_RELEASE( sampleGrabberPinIn );
	SAFE_RELEASE( sourceOutPin );
	
	/*** Connect the grabber and the renderer ***/
	if( FAILED( hr = pGB->Connect( sampleGrabberPinOut, rendererPin ) ) )
		return hr;
	SAFE_RELEASE( sampleGrabberPinOut );
	
	/*** Render the audio ***/
	if ( aFormat == NULL || aFormat->renderAudio )
	{		
		/** Para poner el audio, primero hay que ver si el videoSource tiene pin de salida de audio.
		Si no lo tiene hay que añadir el splitter de un tipo concreto ( Al conectar se mete automaticamente)
		Habría que coger el splitter y render la salida de audio**/		
		/** Al conectar, el splitter se mete automaticamente, ya no hace falta meterlo **/
		/*CComPtr <IBaseFilter> pStreamSplitter = NULL;
		//CLSID_MPEG1Splitter
		if ( FAILED(hr = CoCreateInstance(CLSID_AviSplitter, NULL, CLSCTX_INPROC_SERVER, 
				IID_IBaseFilter, (void**)&(pStreamSplitter)))) 
			return hr;
		if ( FAILED(hr = pGB->AddFilter(pStreamSplitter, L"Stream Splitter"))) 
			return hr;

		//Conect VideoSource and the splitter
		if ( FAILED( hr = connectFilters(videoSource,1,pStreamSplitter,1)))
			return hr;	*/

		IPin *AudioOutPin = NULL;
		if ( !FAILED( hr = getPin( videoSource, PINDIR_OUTPUT, MEDIATYPE_Audio, &AudioOutPin ) ) )			 
		{
			//The video source has audio output pin
			hr = pGB->Render(AudioOutPin);
			SAFE_RELEASE( AudioOutPin );
			format.renderAudio = true;
		}
		else
		{
			/** The video source has not audio output pin.
			Find the splitter and render its audio output pin **/
			IBaseFilter *pStreamSplitter = NULL;
			if ( !FAILED( findFilter( pGB, L"Splitter", &pStreamSplitter ) ) )				
			{
				IPin *AudioOutPin = NULL;
				if ( !FAILED( hr = getPin( pStreamSplitter, PINDIR_OUTPUT, MEDIATYPE_Audio, &AudioOutPin ) ) )					
				{
					IPin* pConnectedPin = NULL;
					if( AudioOutPin->ConnectedTo( &pConnectedPin ) == VFW_E_NOT_CONNECTED )
					{
						hr = pGB->Render(AudioOutPin);
					SAFE_RELEASE( pConnectedPin );
					SAFE_RELEASE( AudioOutPin );
						format.renderAudio = true;
					}
					if ( pConnectedPin != NULL ) 
						pConnectedPin->Release();
				}
				SAFE_RELEASE( pStreamSplitter );
			}
		}
		//Connect the splitter audio pin	
		/*IPin *pPin;
		CComPtr<IEnumPins> EnumPins;
		ULONG		fetched;
		PIN_INFO	pinfo;

		pStreamSplitter->EnumPins(&EnumPins);
		EnumPins->Reset();
		EnumPins->Next(1, &pPin, &fetched);
		pPin->QueryPinInfo(&pinfo);

		if(fetched > 0) do
		{
			if(pinfo.dir == PINDIR_OUTPUT)
			{
				IPin* pConnectedPin = NULL;
				if(pPin->ConnectedTo(&pConnectedPin) == VFW_E_NOT_CONNECTED)
					hr = pGB->Render(pPin);
				if(pConnectedPin != NULL) pConnectedPin->Release();
			}
			pPin->Release();
			EnumPins->Next(1, &pPin, &fetched);
			pPin->QueryPinInfo(&pinfo);

		} while(fetched > 0);*/
	}	

	#ifdef _DEBUG
		hr = addToRot( pGB, &dwRegisterROT);
	#endif

	AM_MEDIA_TYPE mediaType;
	ZeroMemory((PVOID)&mediaType, sizeof(AM_MEDIA_TYPE));
	rendererPin->ConnectionMediaType(&mediaType);
	SAFE_RELEASE( rendererPin );

	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) mediaType.pbFormat;

	FILTER_INFO filter_info;
	videoSource->QueryFilterInfo(&filter_info);
	SAFE_RELEASE( filter_info.pGraph ); 

	GUID subtype  = mediaType.subtype;
	format.width  = pvi->bmiHeader.biWidth;
	format.height = pvi->bmiHeader.biHeight;
	format.fps = referenceTime2fps( pvi->AvgTimePerFrame );
 	format.setPixelFormat( UNKNOWN, translateMEDIASUBTYPE( mediaType.subtype ) );
	avgTimePerFrame = static_cast<double>( pvi->AvgTimePerFrame );
	if ( avgTimePerFrame <= 0 )
		avgTimePerFrame = seconds2referenceTime( 0.03 ); //30fps
	freeMediaType(mediaType);

	if ( aFormat != NULL  && !aFormat->clock )
	{
		IMediaFilter *pMediaFilter = NULL;
		hr = pGB->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
		if ( !FAILED(hr) )
		{			
			hr = pMediaFilter->SetSyncSource(NULL);			
		}
		SAFE_RELEASE( pMediaFilter );
		hr = videoSource->SetSyncSource(NULL);
		hr = pVideoRenderer->SetSyncSource(NULL);		
		hr = pSG->SetSyncSource(NULL);
	}
	SAFE_RELEASE( pVideoRenderer );

	IMediaEvent *pMediaEvent = NULL;
	hr = pGB->QueryInterface(IID_IMediaEvent, (void**)&pMediaEvent);
	if ( !FAILED(hr) )
	{
		long event;
		pMediaEvent->WaitForCompletion( INFINITE, &event );
		//if ( EC_COMPLETE == event )
		//	int a = 0;
		SAFE_RELEASE( pMediaEvent );
	}

	hr = EnableMemoryBuffer();
	if ( aFormat != NULL )
		dropFrames = aFormat->dropFrames;

	//pMS->SetRate(2);
	
    return hr;
}

char *VideoFileDShow::getFrame( bool wait)
{
/*	m_CSec.Lock();
	if ( mb.size() > 0 )
	{
		mb.front().blocked = true;
		m_CSec.Unlock();
		mb.front().media_sample->GetPointer((BYTE**)&pixelBuffer);
		frameCaptured = true;
		return pixelBuffer;
	}
	m_CSec.Unlock();
	return NULL;*/
	/*if ( mediaSample != NULL )
	{
		mediaSample->GetPointer((BYTE**)&pixelBuffer);
		frameCaptured = true;
		return pixelBuffer;
	}	
	return NULL;*/
	return pixelBuffer;
}

void VideoFileDShow::releaseFrame()
{
	//using sampleCB
	if ( mediaSample != NULL )
	{
		pixelBuffer = NULL;		
		mediaSample->Release();
		mediaSample = NULL;
		frameCaptured = false;		
		if ( !dropFrames && !paused )
			pMC->Run();
	}
	
	//using BufferCB
	/*if ( pixelBuffer != NULL )
	{
		frameCaptured = false;
		pixelBuffer = NULL;
		if ( !dropFrames && !paused )
			pMC->Run();
	}*/
}


void VideoFileDShow::play()
{
	HRESULT hr;
	if ( pMC != NULL )
	{
		paused = !SUCCEEDED( hr = pMC->Run() );
		assert( SUCCEEDED( hr ) && "Failed running video file" );
		//if ( FAILED( hr ) )	
		//	cerr << "Failed running file " << identification.fileName << endl;
		//else
		//	paused = false;
	}
}


void VideoFileDShow::pause()
{
	if ( !paused )
		paused = ( pMC != NULL ) && SUCCEEDED( pMC->Pause() );
}


void VideoFileDShow::stop()
{
	stopMedia();
}


void VideoFileDShow::goToFrame( int frame )
{
	FILTER_STATE state;
	pMC->GetState( 1000, (OAFilterState*)&state );		
	bool alreadyPaused = paused;
	pause();

	/*double time2 = ( (double)seconds2referenceTime( LastSampleTime )  / avgTimePerFrame );
	time2 *= avgTimePerFrame;
	LONGLONG kk = avgTimePerFrame * frame + avgTimePerFrame;
	cout << seconds2referenceTime(LastSampleTime) <<  " " << kk << endl;
	pMS->SetPositions(&kk,AM_SEEKING_NoPositioning ,
 				NULL, AM_SEEKING_NoPositioning);
	return;*/

	//Change to time format frames
	//if ( format.timeFormat == SECONDS )
	//	pMS->SetTimeFormat( &TIME_FORMAT_FRAME );
	
	//LONGLONG time = static_cast<LONGLONG>( frame );
	//Tranform from frames to referenceTime
	LONGLONG time = static_cast<LONGLONG>( avgTimePerFrame * frame + avgTimePerFrame * 0.5 );
	//cout << "goto " << frame << " " << referenceTime2seconds(time) << endl;
	if ( pMS != NULL)
		pMS->SetPositions(&time,AM_SEEKING_AbsolutePositioning ,
				NULL, AM_SEEKING_NoPositioning);

	//Return to time format SECONDS
	//if ( format.timeFormat == SECONDS )
		//pMS->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );

	
	if ( !alreadyPaused )//if ( state == State_Paused || state == State_Stopped )		
		play();
}


void VideoFileDShow::goToMilisecond(  double milisecond )
{
	FILTER_STATE state;
	pMC->GetState( 1000, (OAFilterState*)&state );		
	bool alreadyPaused = paused;
	pause();

	//Change to time format seconds	
	//if ( format.timeFormat == FRAMES )
		//pMS->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );

	LONGLONG time = seconds2referenceTime( milisecond * 0.001 );	
	if ( pMS != NULL)
		pMS->SetPositions( &time, AM_SEEKING_AbsolutePositioning ,
				NULL, AM_SEEKING_NoPositioning );
	//Return to time format frames
	//if ( format.timeFormat == FRAMES )
		//pMS->SetTimeFormat( &TIME_FORMAT_FRAME );	
	
	
	LONGLONG position, stopTime;
	pMS->GetPositions(&position, &stopTime );
	//cout << "goto " << time << " " << position << endl;

	if ( !alreadyPaused )//if ( state == State_Paused || state == State_Stopped )		
		play();
}


int VideoFileDShow::getLengthFrames()
{
	return videoLengthFrames;
}

double VideoFileDShow::getLengthSeconds()
{
	return videoLengthSeconds;
}

double VideoFileDShow::getPositionSeconds()
{
	//cout << "get " << seconds2referenceTime( LastSampleTime ) << endl;
	//return LastSampleTime;
	LONGLONG position, stopTime;
	if ( pMS != NULL)
	{
		pMS->GetPositions(&position, &stopTime );
		const double time = referenceTime2seconds( position );		
		return time;
	}
	return 0.0;
}

int VideoFileDShow::getPositionFrames()
{
	LONGLONG position, stopTime;
	if ( pMS != NULL)
	{
		pMS->GetPositions(&position, &stopTime );
		const int time = static_cast<int>( floor( static_cast<double>( position )  / avgTimePerFrame ) );
		//cout << "get " << position << " " << time << endl;
		//const int time2 = static_cast<int>( ( (double)seconds2referenceTime( LastSampleTime )  / avgTimePerFrame ) );
		//cout << "get2 " << seconds2referenceTime( LastSampleTime ) << " " << time2 << endl;
		return time;
	}
	return 0;

	const int time = static_cast<int>( ( (double)seconds2referenceTime( LastSampleTime )  / avgTimePerFrame ) );
	cout << "get " << seconds2referenceTime( LastSampleTime ) << " " << time << endl;
	return time;
}

bool VideoFileDShow::supportFrameCallback()
{
	return true;
}

void VideoFileDShow::setFrameCallback( getFrameCallback theCallback, void *data )
{
	bool alreadyPaused = paused;
	pause();
	callback = theCallback;
	frameCallbackData = data;
	releaseFrame();	//avoid problems wth already captured frames
	if ( !alreadyPaused )
		play();	
}

HRESULT WINAPI VideoFileDShow::SampleCB( double SampleTime, IMediaSample *pSample )
{
	//return(E_NOTIMPL);
	/*while( be.media_sample != NULL )
	{
	}
		
	pSample->AddRef();
	be.media_sample = pSample;		
	LONGLONG time, endT;
	pSample->GetMediaTime( &time, &endT );
	std::cout << "nuevo sample " << static_cast<double>( time  ) << std::endl;
	return(S_OK);
*/
	if ( !dropFrames )
	{
		pMC->Pause();
	}
	
	if ( mediaSample == NULL )
	{
		//const int frame = static_cast<int>( ( (double)seconds2referenceTime( SampleTime )  / avgTimePerFrame ) );
		//cout << "mediaSample " << SampleTime <<endl;
		LastSampleTime = SampleTime;
		pSample->AddRef();
		mediaSample = pSample;

		mediaSample->GetPointer((BYTE**)&pixelBuffer);
		frameCaptured = true;
		if ( callback )
			(*callback)( pixelBuffer, inputID, SampleTime, frameCallbackData );

		//LONGLONG time, endT;
		//pSample->GetMediaTime( &time, &endT );
		return(S_OK);		
	}
	/*else
	{
		cout << "drop " << SampleTime <<endl;
		//pSample->Release();
	}*/
	
	return(S_FALSE);
	/*if ( dropFrames )
	{
		if ( mb.size() < 2 )
		{
		//	m_CSec.Unlock();
			MemoryBufferEntry mb_entry;	
			mb_entry.media_sample = pSample;
			mb_entry.blocked = false;
			pSample->AddRef();
			mb.push_back( mb_entry );
		}
		else
		{
			pSample->AddRef();
			pSample->Release();
		}
	}
	else
	{
		//m_CSec.Unlock();
		MemoryBufferEntry mb_entry;	
		mb_entry.media_sample = pSample;
		mb_entry.blocked = false;
		pSample->AddRef();

		mb.push_back( mb_entry );
	}*/
	
	return(S_FALSE);
}

HRESULT WINAPI VideoFileDShow::BufferCB(double sampleTimeSec, BYTE* bufferPtr, long bufferLength)
{
	if ( !dropFrames )
		pMC->Pause();
	if ( pixelBuffer == NULL )
	{
		LastSampleTime = sampleTimeSec;
		pixelBuffer = (char*)bufferPtr;
		frameCaptured = true;
		if ( callback )
			(*callback)( pixelBuffer, inputID, sampleTimeSec, frameCallbackData );
	}
	return(S_OK);
}
