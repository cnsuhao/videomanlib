#include "StdAfx.h"

#include "VideoInputDShow.h"
#include "DirectShowHelpers.h"
#include <iostream>
#include <vector>

using namespace std;
using namespace VideoMan;

VideoInputDShow::VideoInputDShow(void)
{
	pGB = NULL;
	pCGB = NULL;
	pMS = NULL;
	pMC = NULL;
	pSG = NULL;
	sampleGrabber = NULL;
	videoSource = NULL;
	mediaSample = NULL;
}

VideoInputDShow::~VideoInputDShow(void)
{
	#ifdef _DEBUG
		removeFromRot(dwRegisterROT);
		dwRegisterROT = 0;
	#endif
	freeDirectShow();
}

HRESULT VideoInputDShow::initDirectShow(void)
{
	CoInitialize(NULL);

    HRESULT hr = S_OK;	

    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder,
                        (void **)&pGB);	
	if ( SUCCEEDED( hr ) )
	hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
		IID_ICaptureGraphBuilder2, (void **) &pCGB);	

	pCGB->SetFiltergraph( pGB );

    if ( SUCCEEDED( hr ) )
		hr = pGB->QueryInterface(IID_IMediaControl,  (void **)&pMC);
    if ( SUCCEEDED( hr ) )
		hr = pGB->QueryInterface(IID_IMediaSeeking,  (void **)&pMS);

	if ( SUCCEEDED( hr ) )
		return S_OK;
	else
	{
		freeDirectShow();
		return(hr);
	}
}

HRESULT VideoInputDShow::freeDirectShow(void)
{
    HRESULT hr=S_OK;

    //stopMedia();

    SAFE_RELEASE(pMC);
    SAFE_RELEASE(pMS);
	SAFE_RELEASE(pSG);
	SAFE_RELEASE(sampleGrabber);
	SAFE_RELEASE(videoSource);
	SAFE_RELEASE(pCGB);
	SAFE_RELEASE(pGB);

    return hr;
}

HRESULT VideoInputDShow::stopMedia()
{
    HRESULT hr = S_OK;

    if ( pMC != NULL )
	{
		hr = pMC->StopWhenReady();
		if (FAILED(hr)) 
		{
			cerr << "Failed in Stop()!" << endl;
			return hr;
		}
	}
    return E_FAIL;
}

HRESULT VideoInputDShow::EnableMemoryBuffer(unsigned int _maxConcurrentClients,
						   unsigned int _allocatorBuffersPerClient)
{
	HRESULT hr;
    
	if(_allocatorBuffersPerClient < 1)
		_allocatorBuffersPerClient = 1;
	if(_maxConcurrentClients <= 0)
		_maxConcurrentClients = 1;

	int currentAllocatorBuffers = _maxConcurrentClients * _allocatorBuffersPerClient;

	// ------
	IPin *sgPin = NULL;
	if(FAILED(hr=getPin(pSG,PINDIR_INPUT,1,&sgPin))) return(hr); 
	IMemAllocator *pAllocator = NULL;

	IMemInputPin *sgmiPin = NULL; 
	if ( FAILED( hr = sgPin->QueryInterface(IID_IMemInputPin, (void**)&sgmiPin) ) )
		return(hr);
	SAFE_RELEASE( sgPin );
	if( FAILED(hr = sgmiPin->GetAllocator(&pAllocator)))
		return(hr);
	if( FAILED(hr = pAllocator->Decommit()))
		return(hr);
	ALLOCATOR_PROPERTIES requestedProperties;
	ALLOCATOR_PROPERTIES actualProperties;
	pAllocator->GetProperties(&requestedProperties);
	if(requestedProperties.cBuffers != currentAllocatorBuffers)
		requestedProperties.cBuffers = currentAllocatorBuffers;
	hr = pAllocator->SetProperties(&requestedProperties,&actualProperties);
	SAFE_RELEASE( sgmiPin );
	SAFE_RELEASE( pAllocator );
	//E_UNEXPECTED Internal DIB decoder
	if ( hr != E_UNEXPECTED && FAILED( hr ) )
		return(hr);

	sampleGrabber->SetBufferSamples(FALSE);
	sampleGrabber->SetOneShot(FALSE);
	
	ISampleGrabberCB *custom_sgCB;
	if ( FAILED( hr = QueryInterface( IID_ISampleGrabberCB, (void**)&custom_sgCB ) ) )	
		return(hr);
	hr = sampleGrabber->SetCallback( custom_sgCB, 0 ); //(0:sampleCB, 1:BufferCB)
	SAFE_RELEASE( custom_sgCB );	
	return hr;
}

HRESULT VideoInputDShow::findMediaType(IPin *pin, VMInputFormat *requestedFormat, AM_MEDIA_TYPE *requestedMT ) 
{
	HRESULT hr;
	IEnumMediaTypes *enum_mt;
	pin->EnumMediaTypes(&enum_mt);
	enum_mt->Reset();
	AM_MEDIA_TYPE *mediaType = NULL;
	GUID requestedSubtype;
	requestedSubtype =GUID_NULL;//;GUID_NULL;//translatePIXEL_FORMAT( requestedFormat->pixelFormat );
	requestedSubtype = translatePIXEL_FORMAT( requestedFormat->getPixelFormatIn() );
	while( S_OK == ( hr = enum_mt->Next( 1, &mediaType, NULL ) ) )
	{
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) mediaType->pbFormat;
		if ( requestedFormat->fps > 0 )
			pvi->AvgTimePerFrame = fps2referenceTime( requestedFormat->fps );
		copyMediaType( requestedMT, mediaType );
		
		if( ((mediaType->subtype == requestedSubtype) || (requestedSubtype == GUID_NULL)) &&
			(pvi->bmiHeader.biHeight == requestedFormat->height) &&
			(pvi->bmiHeader.biWidth  == requestedFormat->width) )			
		{
			deleteMediaType(mediaType);
			SAFE_RELEASE( enum_mt );
			return(S_OK);
		}
		else deleteMediaType(mediaType);
	}
	SAFE_RELEASE( enum_mt );
	return(E_FAIL);
}

HRESULT VideoInputDShow::findOneMediaType(IPin *pin, AM_MEDIA_TYPE *requestedMT ) 
{
	HRESULT hr;
	IEnumMediaTypes *enum_mt;
	pin->EnumMediaTypes(&enum_mt);
	enum_mt->Reset();
	AM_MEDIA_TYPE *mediaType = NULL;
	GUID requestedSubtype;
	requestedSubtype =GUID_NULL;
	if ( S_OK == ( hr = enum_mt->Next( 1, &mediaType, NULL ) ) )
	{
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) mediaType->pbFormat;		
		pvi->AvgTimePerFrame = fps2referenceTime( 30.0 );
		copyMediaType( requestedMT, mediaType );
		deleteMediaType(mediaType);
		SAFE_RELEASE( enum_mt );
		return(S_OK);		
	}
	SAFE_RELEASE( enum_mt );
	return(E_FAIL);
}


HRESULT VideoInputDShow::findCaptureDevice( IBaseFilter ** ppSrcFilter, std::string &friendlyName, std::string &devicePath )
{
	//Create a device enumerator
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, 
								IID_ICreateDevEnum,(void **)&pSysDevEnum);
	if (FAILED(hr))
		return hr;

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
					std::string fName;
					std::string dPath;
					int ret = WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)varName.pcVal, -1, NULL, 0, NULL, NULL );
					std::vector<char> friendlyNameAux( ret );					
					WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)varName.pcVal, -1, &friendlyNameAux[0], ret, NULL, NULL );					
					fName = &friendlyNameAux[0];
					//hr = pPropBag->Read(L"Description", &varName, 0);
					hr = pPropBag->Read(L"DevicePath", &varName, 0);					
					if ( SUCCEEDED( hr ) )
					{					
						int ret = WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)varName.pcVal, -1, NULL, 0, NULL, NULL );
						std::vector<char> deviceAux( ret );						
						WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)varName.pcVal, -1, &deviceAux[0], ret, NULL, NULL );						
						dPath = &deviceAux[0];
					}
					if ( ( friendlyName.empty() && devicePath.empty() ) ||
							( friendlyName.empty() && !devicePath.empty() && devicePath == dPath ) ||
							( !friendlyName.empty() && devicePath.empty() && friendlyName == fName ) ||
							( !friendlyName.empty() && !devicePath.empty() && friendlyName == fName && devicePath == dPath) )
					{
						friendlyName = fName;
						devicePath = dPath;
						// Bind Moniker to a filter object
						hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)ppSrcFilter);
						SAFE_RELEASE(pPropBag);
						SAFE_RELEASE(pMoniker);
						SAFE_RELEASE(pEnumCat);
						SAFE_RELEASE(pSysDevEnum);
						return hr;
					}					

					/*Parse the devicepath
						"\\?\1394#sony&dfw-v_500_2.30#780c02002460008#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\global"
					*/
					/*std::string aux;
					size_t ind = devicePath.find( '?' );
					aux.resize( devicePath.length() - ind - 2 );
					devicePath.copy( &aux[0], devicePath.length() - ind - 2, ind + 2 );
					ind = aux.find( '#' );
					std::string type;
					type.resize(10);
					aux.copy( &type[0], ind, 0 );
					type.resize( ind );
					if ( type == "usb" || type == "USB" )
					{
						//devicePath = devicePath.Left( devicePath.ReverseFind( '#' ) );
						std::string pathAux;
						pathAux.resize( devicePath.length() );
						ind = devicePath.rfind( '#' );						
						devicePath.resize( ind );
						//devicePath.copy( &pathAux[0], ind, 0);
						ind = devicePath.rfind( '#' ); //second #
						devicePath.erase( devicePath.begin(), devicePath.begin() + ind + 1 );
						
						if ( name.empty() )							
						{
							name = friendlyName;
							// Bind Moniker to a filter object
							hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)ppSrcFilter);
							SAFE_RELEASE(pPropBag);
							SAFE_RELEASE(pMoniker);
							SAFE_RELEASE(pEnumCat);
							SAFE_RELEASE(pSysDevEnum);
							return hr;
						}
						else if ( devicePath == name )
						{
							// Bind Moniker to a filter object
							hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)ppSrcFilter);
							SAFE_RELEASE(pPropBag);
							SAFE_RELEASE(pMoniker);
							SAFE_RELEASE(pEnumCat);
							SAFE_RELEASE(pSysDevEnum);
							return hr;
						}
					}
					else if ( type == "1394" )
					{
						std::string pathAux;
						pathAux.resize( devicePath.length() );
						ind = devicePath.rfind( '#' );
						devicePath.resize( ind );
						ind = devicePath.rfind( '#' ); //second #
						devicePath.erase( devicePath.begin(), devicePath.begin() + ind + 1 );							
						if ( name.empty() )
						{
							name = devicePath;
							// Bind Moniker to a filter object
							hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)ppSrcFilter);
							SAFE_RELEASE(pPropBag);
							SAFE_RELEASE(pMoniker);
							SAFE_RELEASE(pEnumCat);
							SAFE_RELEASE(pSysDevEnum);
							return hr;
						}
						else if ( devicePath == name )
						{
							// Bind Moniker to a filter object
							hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)ppSrcFilter);
							SAFE_RELEASE(pPropBag);
							SAFE_RELEASE(pMoniker);
							SAFE_RELEASE(pEnumCat);
							SAFE_RELEASE(pSysDevEnum);
							return hr;
						}							
					}*/						
				}
				VariantClear(&varName);
				SAFE_RELEASE(pPropBag);
			}
			SAFE_RELEASE(pMoniker);
		}
		SAFE_RELEASE(pEnumCat);
	}
	SAFE_RELEASE(pSysDevEnum);
	return E_FAIL;
}


//// ISampleGrabberCB interface methods -----------------

HRESULT WINAPI VideoInputDShow::QueryInterface(REFIID iid, void** ppvObject )
{
	// Return requested interface
	if (IID_IUnknown == iid)
	{
		*ppvObject = dynamic_cast<IUnknown*>( this );
	}
	else if (IID_ISampleGrabberCB == iid)
	{
		// Sample grabber callback object
		*ppvObject = dynamic_cast<ISampleGrabberCB*>( this );
	}
	else
	{     
		// No interface for requested iid - return error.
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	// inc reference count
	this->AddRef();
	return S_OK;
}

ULONG WINAPI VideoInputDShow::AddRef()
{
	return(fRefCount++);
}

ULONG WINAPI VideoInputDShow::Release()
{
	return (fRefCount--);
}


