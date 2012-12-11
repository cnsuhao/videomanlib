//------------------------------------------------------------------------------
// File: Crossbar.cpp
//
// Desc: A class for controlling video crossbars. 
//
//       This class creates a single object which encapsulates all connected
//       crossbars, enumerates all unique inputs which can be reached from
//       a given starting pin, and automatically routes audio when a video
//       source is selected.
//
//       The class supports an arbitrarily complex graph of crossbars, 
//       which can be cascaded and disjoint, that is not all inputs need 
//       to traverse the same set of crossbars.
//
//       Given a starting input pin (typically the analog video input to
//       the capture filter), the class recursively traces upstream 
//       searching for all viable inputs.  An input is considered viable if
//       it is a video pin and is either:
//
//           - unconnected 
//           - connects to a filter which does not support IAMCrossbar 
//
//       Methods:
//
//       CCrossbar (IPin *pPin);             
//       ~CCrossbar();
//
//       HRESULT GetInputCount (LONG *pCount);
//       HRESULT GetInputType  (LONG Index, LONG * PhysicalType);
//       HRESULT GetInputName  (LONG Index, TCHAR * pName, LONG NameSize);
//       HRESULT SetInputIndex (LONG Index);
//       HRESULT GetInputIndex (LONG *Index);
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>

#include "crossbar.h"
#include <strsafe.h>
#include <assert.h>

//------------------------------------------------------------------------------
// Name: CCrossbar::CCrossbar()
// Desc: Constructor for the CCrossbar class
//------------------------------------------------------------------------------
CCrossbar::CCrossbar(
        IPin *pStartingInputPin,
        HRESULT *phr
    ) 
    : m_pStartingPin (pStartingInputPin)
    , m_CurrentRoutingIndex (0)
    , m_RoutingList (NULL)

{
    HRESULT hr;
    assert(phr);
	    

    assert (pStartingInputPin != NULL);

    // Init everything to zero
    ZeroMemory (&m_RoutingRoot, sizeof (m_RoutingRoot));	    
	    
    hr = BuildRoutingList(pStartingInputPin, &m_RoutingRoot, 0 /* Depth */);

    // Return an error/success code from the constructor        
    if (phr)
        *phr = hr;
}


//------------------------------------------------------------------------------
// Name: CCrossbar::CCrossbar()
// Desc: Destructor for the CCrossbar class
//------------------------------------------------------------------------------
CCrossbar::~CCrossbar()
{   
    HRESULT hr = DestroyRoutingList ();
}


//
// This function is called recursively, every time a new crossbar is
// entered as we search upstream.
//
// Return values:
//
//  S_OK -    Returned on final exit after recursive search if at least
//            one routing is possible
//  S_FALSE - Normal return indicating we've reached the end of a 
//            recursive search, so save the current path
//  E_FAIL -  Unable to route anything

HRESULT CCrossbar::BuildRoutingList (
   IPin     *pStartingInputPin,
   CRouting *pRouting,
   int       Depth
   )
{
    HRESULT  hr;
    LONG     InputIndexRelated, OutputIndexRelated;
    LONG     InputPhysicalType, OutputPhysicalType;
    LONG     Inputs, Outputs, InputIndex, OutputIndex;

    IPin    *pPin=0;
    IPin    *pStartingOutputPin=0;
    PIN_INFO pinInfo;
    CRouting RoutingNext;
    IAMCrossbar *pXbar=0;


    assert (pStartingInputPin != NULL);
    assert (pRouting != NULL);

    if (!pStartingInputPin || !pRouting)
        return E_POINTER;

    //
    // If the pin isn't connected, then it's a terminal pin
    //

    hr = pStartingInputPin->ConnectedTo (&pStartingOutputPin);
    if (hr != S_OK)
        return (Depth == 0) ? E_FAIL : S_FALSE;

    //
    // It is connected, so now find out if the filter supports 
    // IAMCrossbar
    //

    if (S_OK == pStartingOutputPin->QueryPinInfo(&pinInfo)) 
    {
        assert (pinInfo.dir == PINDIR_OUTPUT);

        hr = pinInfo.pFilter->QueryInterface(IID_IAMCrossbar, (void **)&pXbar);
        if (hr == S_OK) 
        {
            assert (S_OK == pXbar->get_PinCounts(&Outputs, &Inputs));

            assert (S_OK == GetCrossbarIndexFromIPin (
                                    pXbar,
                                    &OutputIndex,
                                    FALSE,   // Input ?
                                    pStartingOutputPin));

            assert (S_OK == pXbar->get_CrossbarPinInfo(
                                    FALSE, // Input ?
                                    OutputIndex,
                                    &OutputIndexRelated,
                                    &OutputPhysicalType));

            //
            // for all input pins
            //

            for (InputIndex = 0; InputIndex < Inputs; InputIndex++) 
            {
                assert (S_OK == pXbar->get_CrossbarPinInfo(
                                        TRUE, // Input?
                                        InputIndex,
                                        &InputIndexRelated,
                                        &InputPhysicalType));

                //
                // Is the pin a video pin?
                //
                if (InputPhysicalType < PhysConn_Audio_Tuner) 
                {
                    //
                    // Can we route it?
                    //
                    if (S_OK == pXbar->CanRoute(OutputIndex, InputIndex)) 
                    {

                        assert (S_OK == GetCrossbarIPinAtIndex (
                                        pXbar,
                                        InputIndex,
                                        TRUE,   // Input
                                        &pPin));

                        //
                        // We've found a route through this crossbar
                        // so save our state before recusively searching
                        // again.
                        //
                        ZeroMemory (&RoutingNext, sizeof (RoutingNext));

                        // doubly linked list
                        RoutingNext.pRightRouting = pRouting;
                        pRouting->pLeftRouting = &RoutingNext;

                        pRouting->pXbar = pXbar;
                        pRouting->VideoInputIndex = InputIndex;
                        pRouting->VideoOutputIndex = OutputIndex;
                        pRouting->AudioInputIndex = InputIndexRelated;
                        pRouting->AudioOutputIndex = OutputIndexRelated;
                        pRouting->InputPhysicalType = InputPhysicalType;
                        pRouting->OutputPhysicalType = OutputPhysicalType;
                        pRouting->Depth = Depth;

                        hr = BuildRoutingList (pPin, &RoutingNext, Depth + 1);
                        
                        if (hr == S_FALSE) 
                        {
                            pRouting->pLeftRouting = NULL;
                            SaveRouting (pRouting);
                        }
                    } // if we can route

                } // if its a video pin

            } // for all input pins

            pXbar->Release();
        }
        else 
        {
            // The filter doesn't support IAMCrossbar, so this
            // is a terminal pin
            pinInfo.pFilter->Release();
            pStartingOutputPin->Release ();

            return (Depth == 0) ? E_FAIL : S_FALSE;
        }

        pinInfo.pFilter->Release();
    }

    pStartingOutputPin->Release ();

    return S_OK;
}

//
// Make a copy of the current routing, and AddRef the IAMCrossbar
// interfaces.
//

HRESULT CCrossbar::SaveRouting (CRouting *pRoutingNew)
{
    int Depth = pRoutingNew->Depth + 1;
    CRouting *pr=0;
    CRouting *pCurrent = pRoutingNew;

    if (!pRoutingNew )
        return E_POINTER;


    pr = new CRouting[Depth];
    if (pr == NULL)
        return E_FAIL;

    m_RoutingList.push_back(pr);

    for (int j = 0; j < Depth; j++, pr++) 
    {
        *pr = *pCurrent;
        assert (pCurrent->pXbar != NULL);

        //
        // We're holding onto this interface, so AddRef
        //
        pCurrent->pXbar->AddRef();
        pCurrent = pCurrent->pRightRouting;

        //
        // Pointers were stack based during recursive search, so update them
        // in the allocated array
        //
        pr->pLeftRouting  = pr - 1;
        pr->pRightRouting = pr + 1;

        if (j == 0) {                   // first element
            pr->pLeftRouting = NULL;
        } 
        if (j == (Depth - 1)) {  // last element
            pr->pRightRouting = NULL;
        }
    }

    return S_OK;
}


HRESULT CCrossbar::DestroyRoutingList()
{
    int k, Depth;
    CRouting *pCurrent=0, *pFirst=0;	   

    while (m_RoutingList.size()) 
    {
        pCurrent = pFirst = m_RoutingList.front();
		m_RoutingList.pop_front();

        if (pCurrent)
        {
            Depth = pCurrent->Depth + 1;

            for (k = 0; k < Depth; k++) 
            {
                assert (pCurrent->pXbar != NULL);

                // Release the crossbar interface
                pCurrent->pXbar->Release();

                // Move to the next node in the list
                pCurrent = pCurrent->pRightRouting;
            }
        }
        
        delete [] pFirst;
    }

    return S_OK;
}


//
// Does not AddRef the returned *Pin 
//
HRESULT CCrossbar::GetCrossbarIPinAtIndex(
   IAMCrossbar *pXbar,
   LONG PinIndex,
   BOOL IsInputPin,
   IPin ** ppPin)
{
    LONG         cntInPins, cntOutPins;
    IPin        *pP = 0;
    IBaseFilter *pFilter = NULL;
    IEnumPins   *pins=0;
    ULONG        n;
    HRESULT      hr;

    if (!pXbar || !ppPin)
        return E_POINTER;

    *ppPin = 0;

    if(S_OK != pXbar->get_PinCounts(&cntOutPins, &cntInPins))
        return E_FAIL;

    LONG TrueIndex = IsInputPin ? PinIndex : PinIndex + cntInPins;

    hr = pXbar->QueryInterface(IID_IBaseFilter, (void **)&pFilter);

    if (hr == S_OK) 
    {
        if(SUCCEEDED(pFilter->EnumPins(&pins))) 
        {
            LONG i=0;
            while(pins->Next(1, &pP, &n) == S_OK) 
            {
                pP->Release();
                if (i == TrueIndex) 
                {
                    *ppPin = pP;
                    break;
                }
                i++;
            }
            pins->Release();
        }
        pFilter->Release();
    }
    
    return *ppPin ? S_OK : E_FAIL; 
}


//
// Find corresponding index of an IPin on a crossbar
//
HRESULT CCrossbar::GetCrossbarIndexFromIPin (
    IAMCrossbar * pXbar,
    LONG * PinIndex,
    BOOL IsInputPin,
    IPin * pPin)
{
    LONG         cntInPins, cntOutPins;
    IPin        *pP = 0;
    IBaseFilter *pFilter = NULL;
    IEnumPins   *pins = 0;
    ULONG        n;
    BOOL         fOK = FALSE;
    HRESULT      hr;

    if (!pXbar || !PinIndex || !pPin)
        return E_POINTER;

    if(S_OK != pXbar->get_PinCounts(&cntOutPins, &cntInPins))
        return E_FAIL;

    hr = pXbar->QueryInterface(IID_IBaseFilter, (void **)&pFilter);

    if (hr == S_OK) 
    {
        if(SUCCEEDED(pFilter->EnumPins(&pins))) 
        {
            LONG i=0;
        
            while(pins->Next(1, &pP, &n) == S_OK) 
            {
                pP->Release();
                if (pPin == pP) 
                {
                    *PinIndex = IsInputPin ? i : i - cntInPins;
                    fOK = TRUE;
                    break;
                }
                i++;
            }
            pins->Release();
        }
        pFilter->Release();
    }
    
    return fOK ? S_OK : E_FAIL; 
}


//
// How many unique video inputs can be selected?
//
HRESULT CCrossbar::GetInputCount (LONG *pCount)
{
    if (pCount )
    {
        *pCount = m_RoutingList.size();
        return S_OK;
    }
    else
        return E_POINTER;
}


//
// What is the physical type of a given input?
//
HRESULT CCrossbar::GetInputType (
    LONG Index, 
    LONG * plPhysicalType)
{
    if (!plPhysicalType)
        return E_POINTER;

	CRouting *pCurrent = m_RoutingList.front();

    if (Index >= m_RoutingList.size()) {
        return E_FAIL;
    }
    
	pCurrent = m_RoutingList[Index];
    assert (pCurrent != NULL);

    *plPhysicalType = pCurrent->InputPhysicalType;

    return S_OK;
}


//
// Converts a PinType into a String
//
BOOL  CCrossbar::StringFromPinType (TCHAR *pc, int nSize, long lType)
{
    TCHAR *pcT;
    BOOL bSuccess;

    if (!pc || !nSize)
        return FALSE;

    switch (lType) 
    {   
        case PhysConn_Video_Tuner:           pcT = TEXT("Video Tuner\0");          break;
        case PhysConn_Video_Composite:       pcT = TEXT("Video Composite\0");      break;
        case PhysConn_Video_SVideo:          pcT = TEXT("Video SVideo\0");         break;
        case PhysConn_Video_RGB:             pcT = TEXT("Video RGB\0");            break;
        case PhysConn_Video_YRYBY:           pcT = TEXT("Video YRYBY\0");          break;
        case PhysConn_Video_SerialDigital:   pcT = TEXT("Video SerialDigital\0");  break;
        case PhysConn_Video_ParallelDigital: pcT = TEXT("Video ParallelDigital\0");break;
        case PhysConn_Video_SCSI:            pcT = TEXT("Video SCSI\0");           break;
        case PhysConn_Video_AUX:             pcT = TEXT("Video AUX\0");            break;
        case PhysConn_Video_1394:            pcT = TEXT("Video 1394\0");           break;
        case PhysConn_Video_USB:             pcT = TEXT("Video USB\0");            break;
        case PhysConn_Video_VideoDecoder:    pcT = TEXT("Video Decoder\0");        break;
        case PhysConn_Video_VideoEncoder:    pcT = TEXT("Video Encoder\0");        break;
    
        case PhysConn_Audio_Tuner:           pcT = TEXT("Audio Tuner\0");          break;
        case PhysConn_Audio_Line:            pcT = TEXT("Audio Line\0");           break;
        case PhysConn_Audio_Mic:             pcT = TEXT("Audio Mic\0");            break;
        case PhysConn_Audio_AESDigital:      pcT = TEXT("Audio AESDigital\0");     break;
        case PhysConn_Audio_SPDIFDigital:    pcT = TEXT("Audio SPDIFDigital\0");   break;
        case PhysConn_Audio_SCSI:            pcT = TEXT("Audio SCSI\0");           break;
        case PhysConn_Audio_AUX:             pcT = TEXT("Audio AUX\0");            break;
        case PhysConn_Audio_1394:            pcT = TEXT("Audio 1394\0");           break;
        case PhysConn_Audio_USB:             pcT = TEXT("Audio USB\0");            break;
        case PhysConn_Audio_AudioDecoder:    pcT = TEXT("Audio Decoder\0");        break;
    
        default:
            pcT = TEXT("Unknown\0");
            break;
    }
    
    // return TRUE on sucessful copy
    if (SUCCEEDED(StringCbCopy(pc, nSize, pcT)))
        bSuccess = TRUE;
    else
        bSuccess = FALSE;
    
    return (bSuccess);
}


//
// Get a text version of an input
//
// Return S_OK if the buffer is large enough to copy the string name
//
HRESULT CCrossbar::GetInputName (
    LONG   Index, 
    TCHAR *pName, 
    LONG   Size)
{
    CRouting *pCurrent = m_RoutingList.front();

    if ((Index >= m_RoutingList.size()) || (pName == NULL)) {
        return E_FAIL;
    }

   
	pCurrent = m_RoutingList[Index];
    assert (pCurrent != NULL);

    return (StringFromPinType (pName, Size, pCurrent->InputPhysicalType) ?
            S_OK : E_FAIL);
}


//
// Select an input 
//
HRESULT CCrossbar::SetInputIndex (
    LONG Index)
{
    HRESULT hr = E_FAIL;

    CRouting *pCurrent = m_RoutingList.front();
    int j;

    if (Index >= m_RoutingList.size())
        return hr;	    
	pCurrent = m_RoutingList[Index];
    assert (pCurrent != NULL);

    int Depth= pCurrent->Depth + 1;

    for (j = 0; j < Depth; j++) 
    {
        hr = pCurrent->pXbar->Route (pCurrent->VideoOutputIndex, pCurrent->VideoInputIndex);
        assert (S_OK == hr);

        if ((pCurrent->AudioOutputIndex != -1) && (pCurrent->AudioInputIndex != -1)) {
            assert (S_OK == pCurrent->pXbar->Route (pCurrent->AudioOutputIndex, 
                            pCurrent->AudioInputIndex));
        }        

        pCurrent++;
    }

    m_CurrentRoutingIndex = Index;

    return hr;
}


//
// What input is currently selected?
//
HRESULT CCrossbar::GetInputIndex (
    LONG *plIndex)
{
    if (plIndex)
    {    
        *plIndex = m_CurrentRoutingIndex;
        return S_OK;
    }
    else
        return E_POINTER;
}

