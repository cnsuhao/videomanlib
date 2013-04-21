#include "StdAfx.h"

#include "DirectShowHelpers.h"

#include <sstream>
#include <iostream>
#include <objbase.h>
#include <comutil.h>
#include <process.h>
#include <math.h> 

using namespace VideoMan;

void copyStringToChar( const std::string &src, char **dst )
{
	if ( src.length() > 0 )
	{
		*dst = new char[src.length() + 1];
		strcpy( *dst, src.c_str() );
	}
	else
		*dst = NULL;
}

// Make a lowercase copy of s: 
void lowerCase( std::wstring &s )
{ 
	for( size_t i = 0; i < s.length(); ++i ) 
		s[i] = tolower( s[i] );
}

HRESULT addToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
// see Microsoft DirectX SDK 9.0 "Loading a Graph From an External Process" 
{
	IMoniker *pMoniker = NULL;
	IRunningObjectTable *pROT = NULL;
	if (FAILED(GetRunningObjectTable(0, &pROT))) {
		return E_FAIL;
	}	
	std::ostringstream text;	
	text << "FilterGraph " << static_cast<unsigned long>((DWORD_PTR)pUnkGraph) << "pid " << GetCurrentProcessId();
	HRESULT hr = CreateItemMoniker(L"!", _bstr_t(text.str().c_str()), &pMoniker);
	if (SUCCEEDED(hr))
		hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);	
	SAFE_RELEASE( pMoniker );
	SAFE_RELEASE( pROT );
	return hr;
}

void removeFromRot(DWORD pdwRegister)
{
	IRunningObjectTable *pROT = NULL;
	if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
		pROT->Revoke(pdwRegister);
	}
	SAFE_RELEASE( pROT );
}

	

HRESULT getPin(IBaseFilter *flt, PIN_DIRECTION dir, const GUID &mediaType, IPin **pRetPin )
{	
	IPin* Pin = NULL;
	IEnumPins*	EnumPins = NULL;
	ULONG		fetched;
	PIN_INFO	pinfo;
	
	flt->EnumPins(&EnumPins);
	EnumPins->Reset();
	EnumPins->Next(1, &Pin, &fetched);
	//pinfo.pFilter->Release();

	//std::wstring ltype = type;
	//lowerCase( ltype );
	while( fetched > 0 )
	{
		Pin->QueryPinInfo(&pinfo);
		SAFE_RELEASE( pinfo.pFilter );
		if( pinfo.dir == dir && checkSupportMediaType( Pin, mediaType ) )
		{
			SAFE_RELEASE( EnumPins );
			*pRetPin = Pin;				
			return S_OK;
		}
		SAFE_RELEASE( Pin );
		EnumPins->Next(1, &Pin, &fetched);
	}
	SAFE_RELEASE( Pin );
	SAFE_RELEASE( pinfo.pFilter );
	SAFE_RELEASE( EnumPins );
	return E_FAIL;
}

HRESULT getPin(IBaseFilter *flt, PIN_DIRECTION dir, int number, IPin **pRetPin )
{
	int n=0;
	IPin* Pin = NULL;
	IEnumPins*	EnumPins = NULL;
	ULONG		fetched;
	PIN_INFO	pinfo;

	flt->EnumPins(&EnumPins);
	EnumPins->Reset();
	EnumPins->Next(1, &Pin, &fetched);
	Pin->QueryPinInfo(&pinfo);
	//pinfo.pFilter->Release();

	do
	{
		// the pFilter member has an outstanding ref count -> release it, we do not use it anyways!
		SAFE_RELEASE( pinfo.pFilter );
		if(pinfo.dir == dir)
		{
			n++;
			if(n==number) 
			{
				SAFE_RELEASE( EnumPins );
				*pRetPin = Pin;
				return S_OK;
			}
			else
			{
				SAFE_RELEASE( Pin );
				EnumPins->Next(1, &Pin, &fetched);
				if(fetched == 0) // no more pins
				{
					SAFE_RELEASE( EnumPins );
					pRetPin = NULL;
					return(E_FAIL);
				}
				Pin->QueryPinInfo(&pinfo);
			}
		}
		else //if (pinfo.dir != dir)
		{
			SAFE_RELEASE( Pin );
			EnumPins->Next(1, &Pin, &fetched);
			if(fetched == 0) // no more pins
			{
				SAFE_RELEASE( EnumPins );
				pRetPin = NULL;
				return(E_FAIL);
			}
			Pin->QueryPinInfo(&pinfo);
			//pinfo.pFilter->Release();
		}
	} while(Pin != NULL);

	SAFE_RELEASE( EnumPins );
	return E_FAIL;
}

HRESULT connectFilters(IBaseFilter *filter_out, int out_pin_nr, 
					   IBaseFilter *in_filter, int in_pin_nr)
{
	HRESULT hr;
	IPin *OutPin = NULL;
	IPin *InPin = NULL;
	if(FAILED(hr = getPin(filter_out,PINDIR_OUTPUT,out_pin_nr, &OutPin))) return(hr);
	if(FAILED(hr = getPin(in_filter,PINDIR_INPUT,in_pin_nr, &InPin))) 
	{
		SAFE_RELEASE( OutPin );
		return(hr);
	}
	if(OutPin == NULL || InPin== NULL) return(E_FAIL);
	hr = OutPin->Connect(InPin,NULL);
	SAFE_RELEASE( OutPin );
	SAFE_RELEASE( InPin );
	if ( FAILED(hr) ) 
		return(E_FAIL);
	else 
		return(S_OK);
}

HRESULT autoConnectFilters(IBaseFilter *filter_out, int out_pin_nr, IBaseFilter *in_filter,
											  int in_pin_nr, IGraphBuilder *pGraphBuilder)
{
	HRESULT hr;
	IPin *OutPin  = NULL;
	IPin *InPin  = NULL;
	if(FAILED(hr = getPin(filter_out,PINDIR_OUTPUT,out_pin_nr, &OutPin))) return(hr);
	if(FAILED(hr = getPin(in_filter,PINDIR_INPUT,in_pin_nr, &InPin))) 
	{
		SAFE_RELEASE( OutPin );
		return(hr);
	}
	if(OutPin == NULL || InPin== NULL) return(E_FAIL);
	hr = pGraphBuilder->Connect(OutPin,InPin);
	SAFE_RELEASE( OutPin );
	SAFE_RELEASE( InPin );
	if ( FAILED( hr ) ) 
		return(E_FAIL);
	else 
		return(S_OK);
}

VMPixelFormat translateMEDIASUBTYPE( GUID format )
{	
	if(format == MEDIASUBTYPE_UYVY)   return(VM_YUV411);
	else if(format == MEDIASUBTYPE_YUY2)   return(VM_YUV422);
	//if(format == MEDIASUBTYPE_RGB565) return(PIXELFORMAT_RGB565);
	//if(format == MEDIASUBTYPE_RGB555) return(PIXELFORMAT_RGB555);
	else if(format == MEDIASUBTYPE_RGB8) return(VM_GREY8);	
	else if(format == MEDIASUBTYPE_RGB24)  return(VM_BGR24);
	else if(format == MEDIASUBTYPE_RGB32)  return(VM_BGR32);
	return VM_UNKNOWN;
}

GUID translatePIXEL_FORMAT( VMPixelFormat format )
{
	switch ( format )
	{
		case VM_YUV411:	return(MEDIASUBTYPE_Y411);	
		case VM_YUV422:	return(MEDIASUBTYPE_UYVY);
		case VM_IYUV:		return(MEDIASUBTYPE_IYUV);
		case VM_GREY8:		return(MEDIASUBTYPE_RGB8);	
		case VM_GREY16:	return(MEDIASUBTYPE_RGB8);	
		case VM_BGR24:		return(MEDIASUBTYPE_RGB24);
		case VM_RGB24:		return(MEDIASUBTYPE_RGB24);
		case VM_BGR32:		return(MEDIASUBTYPE_RGB32);
		case VM_RGB32:		return(MEDIASUBTYPE_RGB32);
		default:		return GUID_NULL;
	}
}

double referenceTime2fps( REFERENCE_TIME AvgTimePerFrame )
{ 
	double fps = 10000000.0 / static_cast<double>( AvgTimePerFrame );
	return( roundDecimals( fps ) ); 
}

double referenceTime2seconds( REFERENCE_TIME time )
{ 
	double refTime = static_cast<double>( time ) / 10000000.0;
	return( roundDecimals( refTime ) ); 
}

REFERENCE_TIME seconds2referenceTime( double seconds )
{ 
	return (REFERENCE_TIME)( seconds * 10000000.0 ); 
}

REFERENCE_TIME fps2referenceTime( double fps )
{
	return((REFERENCE_TIME)( 10000000.0 / fps ) );
}

HRESULT displayPinProperties(IPin *pSrcPin, const std::wstring &friendlyName, HWND hWnd)
{
	ISpecifyPropertyPages *pPages = NULL;

	HRESULT hr = pSrcPin->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pPages);
	if (SUCCEEDED(hr))
	{
		CAUUID caGUID;
		pPages->GetPages(&caGUID);

		hr = OleCreatePropertyFrame(
			NULL,
			0,
			0,
			friendlyName.c_str(),//L"Property Sheet",
			1,
			(IUnknown **)&(pSrcPin),
			caGUID.cElems,
			caGUID.pElems,
			0,
			0,
			NULL);
		CoTaskMemFree(caGUID.pElems);
	}
	else 
	{
		SAFE_RELEASE( pPages );
		return(hr);
	}
	
	SAFE_RELEASE( pPages );

	return(S_OK);
}

HRESULT displayFilterProperties(IBaseFilter *pFilter, HWND hWnd)
{
	ISpecifyPropertyPages *pProp = NULL;
	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if (SUCCEEDED(hr)) 
	{	
		// Get the filter's name and IUnknown pointer.
		FILTER_INFO FilterInfo;
		hr = pFilter->QueryFilterInfo(&FilterInfo); 
		if (FAILED(hr))
			return hr;

		// Show the page. 
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		if (FAILED(hr))
			return hr;
		OleCreatePropertyFrame(
			NULL,                   // Parent window
			0, 0,                   // (Reserved)
			FilterInfo.achName,     // Caption for the dialog box
			1,                      // Number of objects (just the filter)
			(IUnknown **)&pFilter,  // Array of object pointers. 
			caGUID.cElems,          // Number of property pages
			caGUID.pElems,          // Array of property page CLSIDs
			0,                      // Locale identifier
			0, NULL);               // Reserved

		// Clean up.		
		SAFE_RELEASE( FilterInfo.pGraph ); 
		CoTaskMemFree(caGUID.pElems);
	}
	SAFE_RELEASE( pProp );
	return(hr);
}


DWORD WINAPI showFilterPropertiesThread(LPVOID lpParameter)
{
	return(displayFilterProperties((IBaseFilter*)lpParameter));
}


double roundDecimals(const double &number, const int num_digits)
{
    static const double doBase = 10.0;
    double doComplete5, doComplete5i;
    
    doComplete5 = number * pow(doBase, (double) (num_digits + 1));
    
    if(number < 0.0)
        doComplete5 -= 5.0;
    else
        doComplete5 += 5.0;
    
    doComplete5 /= doBase;
    modf(doComplete5, &doComplete5i);
    
    return doComplete5i / pow(doBase, (double) num_digits);
}

HRESULT findFilter( IGraphBuilder *pGB, const std::wstring &name, IBaseFilter **filter )
{
	//CComPtr <IBaseFilter> pStreamSplitter = NULL;
	//pGB->FindFilterByName( L"Splitter", &pStreamSplitter );
	//pGB->EnumFilters()

	//hr = pGB->Render(OutPin);

	IBaseFilter *pFilter = NULL;
	IEnumFilters *EnumFilters = NULL;
	ULONG		fetched;
	FILTER_INFO	finfo;

	pGB->EnumFilters(&EnumFilters);
	EnumFilters->Reset();
	EnumFilters->Next(1, &pFilter, &fetched);
	pFilter->QueryFilterInfo(&finfo);

	std::wstring lname = name;
	lowerCase( lname );

	while(fetched > 0)
	{
		SAFE_RELEASE( finfo.pGraph );
		std::wstring fname = finfo.achName;
		lowerCase( fname );
		if ( fname.find( lname ) != std::string::npos )
		{
			*filter = pFilter;			
			SAFE_RELEASE( EnumFilters );
            return S_OK;
		}		
		EnumFilters->Next(1, &pFilter, &fetched);
		pFilter->QueryFilterInfo(&finfo);
	}
	SAFE_RELEASE( finfo.pGraph );
	SAFE_RELEASE( EnumFilters );
	return E_FAIL;
}

bool checkSupportMediaType( IPin *Pin, const GUID &mediaType )
{
	IEnumMediaTypes *enumMTypes = NULL;
	Pin->EnumMediaTypes( &enumMTypes );
	ULONG fetched;
	
	AM_MEDIA_TYPE *pmType = NULL;	
	enumMTypes->Reset();
	enumMTypes->Next( 1, &pmType, &fetched);
	while( fetched > 0 )
	{
		if ( pmType->majortype == mediaType )		
		{
			SAFE_RELEASE( enumMTypes );
			return true;
		}
		enumMTypes->Next( 1, &pmType, &fetched);
	}
	SAFE_RELEASE( enumMTypes );
	return false;
}

void freeMediaType(__inout AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
	{
        CoTaskMemFree((PVOID)mt.pbFormat);        
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
	{
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

void deleteMediaType(__inout_opt AM_MEDIA_TYPE *pmt)
{
    if (pmt == NULL)
        return;
    freeMediaType(*pmt);
    CoTaskMemFree((PVOID)pmt);
}

HRESULT copyMediaType(__out AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource)
{
    //  We'll leak if we copy onto one that already exists - there's one
    //  case we can check like that - copying to itself.
    if ( pmtSource == pmtTarget)
		return E_FAIL;
    *pmtTarget = *pmtSource;
    if (pmtSource->cbFormat != 0) {
        if ( pmtSource->pbFormat == NULL)
			return E_FAIL;
        pmtTarget->pbFormat = (PBYTE)CoTaskMemAlloc(pmtSource->cbFormat);
        if (pmtTarget->pbFormat == NULL) {
            pmtTarget->cbFormat = 0;
            return E_OUTOFMEMORY;
        } else {
            CopyMemory((PVOID)pmtTarget->pbFormat, (PVOID)pmtSource->pbFormat,
                       pmtTarget->cbFormat);
        }
    }
    if (pmtTarget->pUnk != NULL) {
        pmtTarget->pUnk->AddRef();
    }

    return S_OK;
}