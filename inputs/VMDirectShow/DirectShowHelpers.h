#include "VideoManInputFormat.h"
#include <string>
//#include "atlbase.h"
#include <dshow.h>

#define SAFE_RELEASE(x) { if(x) { (x)->Release(); (x) = NULL; }}

void copyStringToChar( const std::string &src, char **dst );
HRESULT addToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
void removeFromRot(DWORD pdwRegister);

HRESULT getPin(IBaseFilter *flt, PIN_DIRECTION dir, int number, IPin **pRetPin);
HRESULT getPin(IBaseFilter *flt, PIN_DIRECTION dir, const GUID &mediaType, IPin **pRetPin);
HRESULT connectFilters(IBaseFilter *filter_out, int out_pin_nr,
					   IBaseFilter *in_filter, int in_pin_nr);
HRESULT autoConnectFilters(IBaseFilter *filter_out, int out_pin_nr, IBaseFilter *in_filter,
											  int in_pin_nr, IGraphBuilder *pGraphBuilder);
VideoMan::VMPixelFormat translateMEDIASUBTYPE( GUID format );
GUID translatePIXEL_FORMAT( VideoMan::VMPixelFormat );
double referenceTime2fps( REFERENCE_TIME AvgTimePerFrame );
double referenceTime2seconds( REFERENCE_TIME time );
REFERENCE_TIME seconds2referenceTime( double seconds );
REFERENCE_TIME fps2referenceTime( double fps );
HRESULT displayPinProperties(IPin *pSrcPin, const std::wstring &friendlyName, HWND hWnd = NULL);
HRESULT displayFilterProperties(IBaseFilter *pFilter, HWND hWnd = NULL);
DWORD WINAPI showFilterPropertiesThread(LPVOID lpParameter);

// Func Round
// coded by Simon Hughes ( The Code Project )
// If num_digits is 0, then number is rounded to the nearest integer.
// Examples
//        ROUND(2.15, 1)        equals 2.2
//        ROUND(2.149, 1)        equals 2.1
//        ROUND(-1.475, 2)    equals -1.48
double  roundDecimals( const double &number, const int num_digits = 2);

HRESULT findFilter( IGraphBuilder *pGB, const std::wstring &name, IBaseFilter **filter );
bool checkSupportMediaType( IPin *Pin, const GUID &mediaType );
void freeMediaType(__inout AM_MEDIA_TYPE& mt);
void deleteMediaType(__inout_opt AM_MEDIA_TYPE *pmt);
HRESULT copyMediaType(__out AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource);