#include "StdAfx.h"

#include <iostream>
#include <vector>
#include <assert.h>

#include "CaptureDeviceDShow.h"
#include "DirectShowHelpers.h"

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

CaptureDeviceDShow::CaptureDeviceDShow(void)
{	
	hFPropThread = NULL;
	videoProperties = NULL;
	videoControl = NULL;
	cameraControl = NULL;
	captureDlgs = NULL;
	crossbar = NULL;
}

CaptureDeviceDShow::~CaptureDeviceDShow(void)
{
	stopMedia();
	callback = NULL;
	DWORD exitCode = 1;	
	if ( hFPropThread )
	{
		GetExitCodeThread( hFPropThread, &exitCode);
		TerminateThread( hFPropThread, exitCode);		
		CloseHandle( hFPropThread );
		hFPropThread = NULL;
	}	
	SAFE_RELEASE( videoProperties );
	SAFE_RELEASE( videoControl );
	SAFE_RELEASE( cameraControl );
	SAFE_RELEASE( captureDlgs );
	if ( crossbar )
	{
		delete crossbar;
		crossbar = NULL;
	}
	SAFE_RELEASE( mediaSample );
} 

bool CaptureDeviceDShow::initCamera( const char *friendlyName, const char *devicePath, VMInputFormat *aFormat )
{
    // Initialize DirectShow and query for needed interfaces
    HRESULT hr = initDirectShow();
    if ( FAILED( hr ) )
    {
		cerr << "Failed to initialize DirectShow!" << endl;
		freeDirectShow();
		return false;
    }

	std::string foundFriendlyName;
	if ( friendlyName != NULL )
		foundFriendlyName = friendlyName;
	std::string foundDevicePath;
	if ( devicePath != NULL )
		foundDevicePath = devicePath;
	hr = prepareMedia( foundFriendlyName, foundDevicePath, aFormat );
	copyStringToChar( foundFriendlyName, &identification.friendlyName );
	copyStringToChar( foundDevicePath, &identification.uniqueName );
	copyStringToChar( "DSHOW_CAPTURE_DEVICE", &identification.identifier );
    if (FAILED(hr))
    {
		cerr << "Failed preparing media:" << endl;
		if ( identification.friendlyName )
			cerr << "-friendlyName: " << identification.friendlyName << endl;
		if ( identification.uniqueName )
			cerr << "-devicePath: " << identification.uniqueName << endl;		
        freeDirectShow();     
        return false;
    }

	hr = pMC->Run();
	if ( FAILED( hr ) )
	{
		cerr << "Failed running camera" << endl;
		//freeDirectShow();
		//return false;
	}

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

BOOL CaptureDeviceDShow::CreateCrossbar()
{
	IPin        *pP = 0;
    IEnumPins   *pins=0;
    ULONG        n;
    PIN_INFO     pinInfo;
    BOOL         Found = FALSE;
	BOOL crossbarCreated = FALSE;
    IKsPropertySet *pKs=0;
    GUID guid;
    DWORD dw;
    BOOL fMatch = FALSE;
		
	if( SUCCEEDED( videoSource->EnumPins( &pins ) ) )
	{
		while(!Found && (S_OK == pins->Next(1, &pP, &n)))
		{
			if(S_OK == pP->QueryPinInfo(&pinInfo))
			{
				if(pinInfo.dir == PINDIR_INPUT)
				{
					// is this pin an ANALOGVIDEOIN input pin?
					if(pP->QueryInterface(IID_IKsPropertySet,
						(void **)&pKs) == S_OK)
					{
						if(pKs->Get(AMPROPSETID_Pin,
							AMPROPERTY_PIN_CATEGORY, NULL, 0,
							&guid, sizeof(GUID), &dw) == S_OK)
						{
							if(guid == PIN_CATEGORY_ANALOGVIDEOIN)
								fMatch = TRUE;
						}
						SAFE_RELEASE( pKs );						
					}
					if(fMatch)
                    {
                        HRESULT hrCreate=S_OK;
                        crossbar = new CCrossbar(pP, &hrCreate);
                        if ( !crossbar || FAILED( hrCreate ) )
                            break;

						crossbarCreated = TRUE;
						LONG NumberOfVideoInputs;
                        HRESULT hr = crossbar->GetInputCount(&NumberOfVideoInputs);
                        Found = TRUE;
                    }
                }
				SAFE_RELEASE( pinInfo.pFilter );
            }
			SAFE_RELEASE( pP );            
        }
		SAFE_RELEASE( pins );
    }
	if ( fMatch )
		return crossbarCreated;
	return TRUE;
}

HRESULT CaptureDeviceDShow::prepareMedia( std::string &friendlyName, std::string &devicePath, VMInputFormat *aFormat  )
{
    HRESULT hr = E_FAIL;
	
	if ( aFormat )
		format = *aFormat;

	//Find capture device and add video input source filter
	hr = findCaptureDevice( &videoSource, friendlyName, devicePath );
	if ( SUCCEEDED( hr ) )
	{
		if ( FAILED(hr = pGB->AddFilter(videoSource, _bstr_t(friendlyName.c_str()) )))//L"Video Source")))
			return hr;
	}
	else
		return E_FAIL;

	//Get capture dialog interface
	hr = pCGB->FindInterface(&PIN_CATEGORY_CAPTURE,
						  &MEDIATYPE_Video, videoSource,
						  IID_IAMVfwCaptureDialogs, (void **)&captureDlgs);

	//Create the crossbar if necessary
	if ( !CreateCrossbar() )
	{
		cerr << "Failed creating crossbar" << endl;
		return E_FAIL;
	}

	//Get source out pin
	outPinVideoSource = NULL;
	if ( FAILED( hr = getPin( videoSource, PINDIR_OUTPUT, 1, &outPinVideoSource ) ) ) 
		return hr;

	//Get source stream config interface
	IAMStreamConfig *pStreamConfig = NULL;
	hr = pCGB->FindInterface(&PIN_CATEGORY_CAPTURE,NULL,videoSource,IID_IAMStreamConfig,(void**)&pStreamConfig);
	if (FAILED(hr))		
		return hr;

	//Get IAMVideoProcAmp interface
	if ( videoSource )
		videoSource->QueryInterface( IID_IAMVideoProcAmp,  (void **)&videoProperties );	

	//Get IAMVideoControl interface
	if ( videoSource )
		hr = videoSource->QueryInterface( IID_IAMVideoControl,  (void **)&videoControl );		
	
	//Get IAMCameraControl interface
	if ( videoSource )
		videoSource->QueryInterface( IID_IAMCameraControl,  (void **)&cameraControl );	

	//Setup input format
	AM_MEDIA_TYPE inputMediaType;
	ZeroMemory((PVOID)&inputMediaType, sizeof(AM_MEDIA_TYPE));
	//Find the first media type
	//findOneMediaType( outPinVideoSource, &mediaType );
	bool formatSet = false;
	if ( aFormat != NULL && !aFormat->showDlg )
	{
		hr = findMediaType( outPinVideoSource, aFormat, &inputMediaType );
		if ( SUCCEEDED( hr ) )
			hr = pStreamConfig->SetFormat( &inputMediaType );
		//If media type not found or pStreamConfig->SetFormat failed
		if ( SUCCEEDED( hr ) )
			formatSet = true;
		else
			cerr << "Incorrect input format" << endl;
	}
	if ( !formatSet )
	{
		std::wstring widestring(friendlyName.begin(), friendlyName.end());
		hr = displayPinProperties(outPinVideoSource, widestring );
		if ( FAILED( hr ) )
		{
			bool hasDialogs = false;
			//If pin properties failed, show capture dialog format
			if ( captureDlgs )
			{
				hr = captureDlgs->HasDialog( VfwCaptureDialog_Format );
				if ( SUCCEEDED( hr ) )
				{
					captureDlgs->ShowDialog( VfwCaptureDialog_Format, NULL );
					hasDialogs = true;
				}
			}
			//If capture dialog format failed, show filter properties
			if ( !hasDialogs )
			{
				displayFilterProperties( videoSource, NULL );
			}

		}
		//if ( FAILED( hr = displayPinProperties(outPinVideoSource, widestring ) ) ) 
		//	return(hr);
		//If cancel EXIT???
	}
	/*else
	{
		hr = findMediaType( outPinVideoSource, aFormat, &inputMediaType );
		if ( SUCCEEDED( hr ) )		
			hr = pStreamConfig->SetFormat( &inputMediaType );
		//If media type not found or pStreamConfig->SetFormat failed
		if ( FAILED( hr ) )
		{
			cerr << "Incorrect input format" << endl;
			std::wstring widestring(friendlyName.begin(), friendlyName.end());
			if ( FAILED( hr = displayPinProperties(outPinVideoSource, widestring ) ) ) 
				return(hr);
		}
	}*/	
	SAFE_RELEASE( pStreamConfig );
	SAFE_RELEASE( outPinVideoSource );

	//Create NULL Renderer
	IBaseFilter *pVideoRenderer = NULL;
	if ( FAILED( hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&(pVideoRenderer) ) ) )
		return hr;
	if ( FAILED ( hr = pGB->AddFilter(pVideoRenderer, L"NULL Video Renderer") ) )
		return hr;
	
	//Create sample grabber filter
	if ( FAILED( hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&pSG) ) )
		return hr;	
	if ( FAILED( hr = pSG->QueryInterface(IID_ISampleGrabber,(void**)&sampleGrabber)) )
		return(hr);
	
	//Set sampleGrabber ouput format
	AM_MEDIA_TYPE outputMediaType;
	ZeroMemory(&outputMediaType,sizeof(AM_MEDIA_TYPE));
	outputMediaType.majortype = MEDIATYPE_Video;
	outputMediaType.formattype = GUID_NULL;
	if ( aFormat != NULL )//&& aFormat->getPixelFormatOut() != UNKNOWN )	
		outputMediaType.subtype = translatePIXEL_FORMAT( aFormat->getPixelFormatOut() );
	else
		outputMediaType.subtype = translatePIXEL_FORMAT( RGB24 );
	if ( FAILED( hr = sampleGrabber->SetMediaType(&outputMediaType) ) )
		return hr;

	//Add sampple grabber filter
	if ( FAILED( hr = pGB->AddFilter(pSG, L"Sample Grabber") ) )
		return hr;

	//Conect the source and the grabber
	if ( FAILED( hr = autoConnectFilters(videoSource,1,pSG,1,pGB) )	 )
		return hr;
	//Conect the grabber and the renderer
	if ( FAILED( hr = autoConnectFilters(pSG,1,pVideoRenderer,1,pGB) ) )
		return hr;
	
	#ifdef _DEBUG
		hr = addToRot( pGB, &dwRegisterROT );
	#endif

	//Get the final media type from the renderer input pin	
	IPin *rendererPin = NULL;
	if(FAILED(hr = getPin(pVideoRenderer,PINDIR_INPUT,1, &rendererPin))) return(hr);	
	rendererPin->ConnectionMediaType(&outputMediaType);
	SAFE_RELEASE( rendererPin );
	SAFE_RELEASE( pVideoRenderer );
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) outputMediaType.pbFormat;
	FILTER_INFO filter_info;
	videoSource->QueryFilterInfo(&filter_info);
	SAFE_RELEASE( filter_info.pGraph );
	GUID subtype  = outputMediaType.subtype;
	format.width  = pvi->bmiHeader.biWidth;
	format.height = pvi->bmiHeader.biHeight;
	format.fps = referenceTime2fps( pvi->AvgTimePerFrame );
 	format.setPixelFormat( UNKNOWN, translateMEDIASUBTYPE( outputMediaType.subtype ) );
	freeMediaType(outputMediaType);	

	hr = EnableMemoryBuffer();
	if ( aFormat != NULL )
		dropFrames = aFormat->dropFrames;
	
	//pMS->SetRate(2);

    return hr;
}

char *CaptureDeviceDShow::getFrame( bool wait)
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


void CaptureDeviceDShow::releaseFrame()
{
	/*if (frameCaptured)
	{
		pixelBuffer = NULL;
		mb.front().media_sample->Release();
		mb.front().blocked = false;
		mb.pop_front();
		frameCaptured = false;
	}*/
	//using SampleCB
	if ( mediaSample != NULL )
	{
		pixelBuffer = NULL;		
		mediaSample->Release();
		mediaSample = NULL;
		frameCaptured = false;		
		if ( !dropFrames )
			play();
	}
	//using BufferCB
	/*if ( pixelBuffer != NULL )
	{
		frameCaptured = false;
		pixelBuffer = NULL;
		if ( !dropFrames )
			pMC->Run();
	}*/
}


void CaptureDeviceDShow::play()
{
	if ( pMC != NULL )
		pMC->Run();	
}


void CaptureDeviceDShow::pause()
{
	if ( pMC != NULL )
		pMC->Pause();
}


void CaptureDeviceDShow::stop()
{
	stopMedia();
}

IBaseFilter *CaptureDeviceDShow::getVideoSourceFilter()
{
	return videoSource;
}

std::string CaptureDeviceDShow::getFriendlyName()
{
	return identification.friendlyName;
}

template<class ClassType>
unsigned long WINAPI displayFilterPropertiesThread(void *parameters)
{
	ClassType *device = static_cast<ClassType*>(parameters);
	IBaseFilter *pFilter = device->getVideoSourceFilter();
	ISpecifyPropertyPages *pProp;
	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if (SUCCEEDED(hr)) 
	{
		std::string friendlyName = device->getFriendlyName();
		std::wstring widestring(friendlyName.begin(), friendlyName.end());

		// Show the page. 
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		if (FAILED(hr))
		{
			SAFE_RELEASE( pProp );
			return hr;
		}
		OleCreatePropertyFrame(
			NULL,                   // Parent window
			0, 0,                   // (Reserved)
			widestring.c_str(),     // Caption for the dialog box
			1,                      // Number of objects (just the filter)
			(IUnknown **)&pFilter,  // Array of object pointers. 
			caGUID.cElems,          // Number of property pages
			caGUID.pElems,          // Array of property page CLSIDs
			0,                      // Locale identifier
			0, NULL);               // Reserved
		CoTaskMemFree(caGUID.pElems);
	}
	SAFE_RELEASE( pProp );
	return(hr);
}

void CaptureDeviceDShow::showPropertyPage()
{
	if ( hFPropThread )
	{
		DWORD exitCode = 1;	
		TerminateThread( hFPropThread, exitCode);	
		GetExitCodeThread( hFPropThread, &exitCode);
		CloseHandle( hFPropThread );
		hFPropThread = NULL;
	}

	DWORD dwThreadID;
	//hFPropThread = CreateThread(NULL,0,showFilterPropertiesThread,(LPVOID)videoSource,0,&dwThreadID);
	hFPropThread = CreateThread( NULL,0,displayFilterPropertiesThread<CaptureDeviceDShow>, (LPVOID)this, 0, &dwThreadID );	
}

void CaptureDeviceDShow::showCaptureDialogDisplay()
{
	if ( captureDlgs )
	{
		stop();
		HRESULT hr = captureDlgs->HasDialog( VfwCaptureDialog_Display );
		if ( captureDlgs && SUCCEEDED( hr ) )
			captureDlgs->ShowDialog( VfwCaptureDialog_Display, NULL );
		play();
	}
}

void CaptureDeviceDShow::showCaptureDialogSource()
{
	if ( captureDlgs )
	{
		stop();
		HRESULT hr = captureDlgs->HasDialog( VfwCaptureDialog_Source );
		if ( SUCCEEDED( hr ) )
			captureDlgs->ShowDialog( VfwCaptureDialog_Source, NULL );
		play();
	}
}

void CaptureDeviceDShow::showCaptureDialogFormat()
{
	if ( captureDlgs )
	{
		stop();
		HRESULT hr = captureDlgs->HasDialog( VfwCaptureDialog_Format );
		if ( SUCCEEDED( hr ) )
			captureDlgs->ShowDialog( VfwCaptureDialog_Format, NULL );
		play();
	}
}

void CaptureDeviceDShow::getAvailableDevices( VMInputIdentification **deviceList2, int &numDevices  )
{
	vector<VMInputIdentification> deviceList;
	numDevices = 0;
	*deviceList2 = NULL;

	//Create a device enumerator
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, 
								IID_ICreateDevEnum,(void **)&pSysDevEnum);
	if (FAILED(hr))
		return;

	// Get an enumerator class for the video inpout category
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);	
	if (hr == S_OK)
	{
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK))
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{				
				VARIANT varName;				
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);				
				if (SUCCEEDED(hr))
				{					
					std::string friendlyName;					
					int ret = WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)varName.pcVal, -1, NULL, 0, NULL, NULL );
					std::vector<char> friendlyNameAux( ret );					
					WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)varName.pcVal, -1, &friendlyNameAux[0], ret, NULL, NULL );
					friendlyName = &friendlyNameAux[0];
					hr = pPropBag->Read(L"DevicePath", &varName, 0);
					std::string devicePath;
					if (SUCCEEDED(hr))
					{						
						int ret = WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)varName.pcVal, -1, NULL, 0, NULL, NULL );
						std::vector<char> deviceAux( ret );						
						WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)varName.pcVal, -1, &deviceAux[0], ret, NULL, NULL );
						devicePath = &deviceAux[0];
					}
					VMInputIdentification device;
					device.fileName = NULL;					
					copyStringToChar( devicePath, &device.uniqueName );
					copyStringToChar( friendlyName, &device.friendlyName );
					copyStringToChar( "DSHOW_CAPTURE_DEVICE", &device.identifier );					
					deviceList.push_back( device );
				}
				VariantClear(&varName);
				SAFE_RELEASE(pPropBag);
			}
			SAFE_RELEASE(pMoniker);
		}	
	}
	SAFE_RELEASE(pEnumCat);
	SAFE_RELEASE(pSysDevEnum);

	if ( deviceList.size() > 0 )
	{
		numDevices = deviceList.size();
		*deviceList2 = new VMInputIdentification[deviceList.size()];
		for ( size_t d = 0; d < deviceList.size(); ++d )
			(*deviceList2)[d] = deviceList[d];
	}

	return;
}

bool CaptureDeviceDShow::supportVideoControl()
{
	return ( videoControl != NULL );
}

bool CaptureDeviceDShow::getMode( long &mode )
{
	if ( videoControl )
	{
		HRESULT hr = videoControl->GetMode( outPinVideoSource, &mode );
	}
	return false;
}

bool CaptureDeviceDShow::supportImageProperties()
{
	return ( videoProperties != NULL );
}

void CaptureDeviceDShow::setImageProperty( VideoProcAmpProperty imageProp, const long &value, const bool &flagAuto )
{
	HRESULT hr;
	if ( videoProperties )
	{		
		hr = videoProperties->Set( imageProp, value, flagAuto ? VideoProcAmp_Flags_Auto: VideoProcAmp_Flags_Manual );
	}
}

bool CaptureDeviceDShow::getImageProperty( VideoProcAmpProperty imageProp, long &value, bool &flagAuto )
{
	if ( videoProperties )
	{
		long flags;
		HRESULT hr = videoProperties->Get( imageProp, &value, &flags );
		flagAuto = ( flags == VideoProcAmp_Flags_Auto );
		if ( hr == S_OK )
			return true;
	}
	return false;
}

bool CaptureDeviceDShow::getImagePropertyRange( VideoProcAmpProperty imageProp, long &min, long &max, long &steppingDelta, long &defaultVal, bool &flagAuto )
{
	if ( videoProperties )	
	{		
		long flags;
		HRESULT hr = videoProperties->GetRange( imageProp, &min, &max, &steppingDelta, &defaultVal, &flags );
		flagAuto = ( flags == VideoProcAmp_Flags_Auto );
		if ( hr == S_OK )
			return true;
	}
	return false;
}

void CaptureDeviceDShow::setCameraControl( CameraControlProperty camControl, const long &value, const bool &flagAuto )
{
	HRESULT hr;
	if ( cameraControl )
		hr = cameraControl->Set( camControl, value, flagAuto ? CameraControl_Flags_Auto : CameraControl_Flags_Manual );
}

bool CaptureDeviceDShow::getCameraControl( CameraControlProperty camControl, long &value, bool &flagAuto )
{
	HRESULT hr;
	if ( cameraControl )			
	{
		long flags;
		hr = cameraControl->Get( camControl, &value, &flags );
		flagAuto = ( flags == CameraControl_Flags_Auto );
		if ( hr == S_OK )
			return true;
	}
	return false;
}

bool CaptureDeviceDShow::getCameraControlRange( CameraControlProperty camControl, long &min, long &max, long &steppingDelta, long &defaultVal, bool &flagAuto )
{
	HRESULT hr;
	if ( cameraControl )			
	{
		long flags;
		hr = cameraControl->GetRange( camControl, &min, &max, &steppingDelta, &defaultVal, &flags );
		flagAuto = flags == CameraControl_Flags_Auto;
		if ( hr == S_OK )
			return true;
	}
	return false;
}

bool CaptureDeviceDShow::supportCameraControl()
{
	return ( cameraControl != NULL );
}

int CaptureDeviceDShow::getInputChannelsCount()
{
	if ( crossbar )
	{
		LONG count;
		if ( SUCCEEDED( crossbar->GetInputCount( &count ) ))
			return static_cast<int>( count );
	}
	return 0;
}

VideoManInputController *CaptureDeviceDShow::getController()
{
	return (VideoManInputController*)( controller );
}

bool CaptureDeviceDShow::linkController( VideoManInputController *acontroller )
{	
	assert( acontroller!= NULL && "linkController: Invalid controller" );
	if ( acontroller->setInput( this ) )
	{
		controller = acontroller;
		return true;
	}
	return false;
}

bool CaptureDeviceDShow::setInputChannel( int channel )
{
	if ( crossbar )
	{
		if ( SUCCEEDED( crossbar->SetInputIndex( (LONG)channel ) ) )
			return true;
	}
	return false;
}

bool CaptureDeviceDShow::supportFrameCallback()
{
	return true;
}

void CaptureDeviceDShow::setFrameCallback( getFrameCallback theCallback, void *data )
{	
	pause();
	callback = theCallback;
	frameCallbackData = data;
	releaseFrame();	//avoid problems wth already captured frames
	play();	
}

HRESULT WINAPI CaptureDeviceDShow::SampleCB( double SampleTime, IMediaSample *pSample )
{
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

HRESULT WINAPI CaptureDeviceDShow::BufferCB(double sampleTimeSec, BYTE* bufferPtr, long bufferLength)
{
	if ( pixelBuffer == NULL )
	{
		pixelBuffer = (char*)bufferPtr;
		frameCaptured = true;
		if ( callback )
			(*callback)( pixelBuffer, inputID, sampleTimeSec, frameCallbackData );
	}
	return(S_OK);
}