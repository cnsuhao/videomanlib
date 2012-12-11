#include "KinectCamera.h"
#include <sstream>
#include <iostream>
#include <math.h>
using namespace std;

#define INITIALIZE         0x000
#define CAMERA_POWER       0x610

#define CURRENT_VIDEO_FORMAT  0x608
#define CURRENT_VIDEO_MODE    0x604

#define IMAGE_POSITION	      0x008
#define IMAGE_SIZE			0x00C
#define COLOR_CODING		0x010
#define PACKET_PARA_INQ	      0x040
#define BYTE_PER_PACKET	      0x044
#define VALUE_SETTING	      0x07c
#define FRAME_RATE	      0x83C

#define MAX_IMAGE_SIZE_INQ 0x000
#define UNIT_SIZE_INQ 0x004
#define UNIT_POSITION_INQ 0x04c

unsigned long videoModeBase[] ={0xA00,0xA80,0xB00,0xB80,0xC00,0xC80};//(pag47)*/

using namespace VideoMan;
using namespace VideoManPrivate;

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

KinectCamera::KinectCamera(void)
{
	raw2depth();
	__source=-1;
}


KinectCamera::~KinectCamera(void)
{
	delete __data;
	__context.Shutdown();

}


bool KinectCamera::initCamera( std::string source, std::string xmlFile )
{

	if (source.compare("KINECT_RGB_Camera")==0)
	{
		__source = RGB_CAMERA;
		format.SetFormat(640,480,30,RGB24,RGB24);
		__data = new char[640*480*3];
	}
	else if (source.compare("KINECT_Depth_Camera_Color")==0)
	{
		__source = DEPTH_CAMERA_COLOR;
		format.SetFormat(640,480,30,RGB24,RGB24);
		__data = new char[640*480*3];
	}
	else if (source.compare("KINECT_IR_Camera")==0)
	{
		__source = IR_CAMERA;
		format.SetFormat(1024,768,30,GREY16,GREY16);
		__data = new char[1024*768*2];

	}
	else{
		__source = DEPTH_CAMERA;
		format.SetFormat(640,480,30,GREY16,GREY16);
		__data = new char[640*480*2];
	}

	xn::EnumerationErrors errors;
	int nRetVal = __context.InitFromXmlFile(xmlFile.c_str(), &errors);
	if (nRetVal != XN_STATUS_OK) return false;
	// Create a DepthGenerator node 
	if(__source == DEPTH_CAMERA) nRetVal = __context.FindExistingNode(XN_NODE_TYPE_DEPTH, __depth);
	else nRetVal = __context.FindExistingNode(XN_NODE_TYPE_IMAGE, __image);
	if (nRetVal != XN_STATUS_OK) return false;
	

	copyStringToChar( "VMOpenNI", &identification.identifier );
	copyStringToChar( source, &identification.friendlyName );
	// Make it start generating data 
	nRetVal = __context.StartGeneratingAll();
	
	if (nRetVal != XN_STATUS_OK) return false;

	format.SetFormat(640,480,30,RGB24,RGB24);
	
		return true;
}

bool KinectCamera::initCamera( std::string source )
{
	int resX,resY;
	resX=640;
	resY=480;
	if (source.compare("KINECT_RGB_Camera")==0)
	{
		__source = RGB_CAMERA;
		format.SetFormat(640,480,30,RGB24,RGB24);
		__data = new char[640*480*3];
	}
	else if (source.compare("KINECT_Depth_Camera_Color")==0)
	{
		__source = DEPTH_CAMERA_COLOR;
		format.SetFormat(640,480,30,RGB24,RGB24);
		__data = new char[640*480*3];
	}
	else if (source.compare("KINECT_IR_Camera")==0)
	{
		__source = IR_CAMERA;
		format.SetFormat(resX,resY,30,GREY8,GREY8);
		__data = new char[resX*resY];

	}
	else
	{
		__source = DEPTH_CAMERA;
		format.SetFormat(640,480,30,GREY16,GREY16);
		__data = new char[640*480*2];
	}

	xn::EnumerationErrors errors;
	int nRetVal = __context.Init();
	if (nRetVal != XN_STATUS_OK) return false;
	// Create a DepthGenerator node 
	
	
	if (__source==DEPTH_CAMERA || __source == DEPTH_CAMERA_COLOR)
	{
		nRetVal = __depth.Create(__context); 
		__mapMode.nXRes = 640; 
		__mapMode.nYRes = 480; 
		__mapMode.nFPS = 30; 
		nRetVal = __depth.SetMapOutputMode(__mapMode);
	}
	else if (__source==IR_CAMERA)
	{
		nRetVal = __ir.Create(__context); 
		__mapMode.nXRes = resX; 
		__mapMode.nYRes = resY; 
		__mapMode.nFPS = 30; 
		nRetVal = __ir.SetMapOutputMode(__mapMode);
	}
	else
	{
		nRetVal = __image.Create(__context);
		__mapMode.nXRes = 640; 
		__mapMode.nYRes = 480; 
		__mapMode.nFPS = 30; 
		nRetVal = __image.SetMapOutputMode(__mapMode);

		/* Establecemos el formato interno */
		format.SetFormat(640,480,30,RGB24,RGB24);
	}
	
	if (nRetVal != XN_STATUS_OK) return false;
	
	/* Copiamos la identificaciones de la dll */
	copyStringToChar( "VMOpenNI", &identification.identifier );
	copyStringToChar( source, &identification.friendlyName );

	
	// Make it start generating data 
	nRetVal = __context.StartGeneratingAll();
	
	if (nRetVal != XN_STATUS_OK) return false;

	
		return true;
}

void KinectCamera::setFrameCallback( getFrameCallback theCallback )
{
	callback = theCallback ;
}


void KinectCamera::releaseFrame()
{
	//delete pixelBuffer;
}
void KinectCamera::raw2depth(){
        int i;
        for ( i=0; i<10000; i++) {
        //float dist =100/(-0.0030711016*i + 3.3309495161); //ros distance (cm)
        //float dist2 = 12.36 * tanf(i/ 2842.5 + 1.1863);//Stéphane Magnenat distance (cm)
        float v = (float)i/10000;//for visualization purposes only
                v = powf(v, 2);
                v = v*36*256;
        depth[i] = v;
        //printf("%f    ",dist2);
       
        /*
         * XnUInt64 F_;
         * XnDouble pixel_size_;
                // get the focal length in mm (ZPS = zero plane distance)
                        depth_.GetIntProperty ("ZPD", F_)
                // get the pixel size in mm ("ZPPS" = pixel size at zero plane)
                        depth_.GetRealProperty ("ZPPS", pixel_size_)
                        X = (u - 320) * depth_md_[k] * pixel_size_ * 0.001 / F_;
                        Y = (v - 240) * depth_md_[k] * pixel_size_ * 0.001 / F_;
                        Z = depth_md_[k] * 0.001; // from mm in meters!
         */
		}
}
char* KinectCamera::depth2rgb(const XnDepthPixel* Xn_disparity)
{
		int i;
        //const unsigned short *disparity = Xn_disparity;
		
		

		//depth_data = (char*)malloc(640*480*3);
	
        for (i=0; i<307200; i++) {
                int pval = depth[Xn_disparity[i]];
                //printf("%d: %u %d \n",i,*Xn_disparity,depth[*Xn_disparity]);
                //fflush(stdout);
                int lb = pval & 0xff;
                switch (pval>>8) {
                        case 0:
                                __data[3*i+0] = 255;
                                __data[3*i+1] = 255-lb;
                                __data[3*i+2] = 255-lb;
                                break;
                        case 1:
                                __data[3*i+0] = 255;
                                __data[3*i+1] = lb;
                                __data[3*i+2] = 0;
                                break;
                        case 2:
                                __data[3*i+0] = 255-lb;
                                __data[3*i+1] = 255;
                                __data[3*i+2] = 0;
                                break;
                        case 3:
                                __data[3*i+0] = 0;
                                __data[3*i+1] = 255;
                                __data[3*i+2] = lb;
                                break;
                        case 4:
                                __data[3*i+0] = 0;
                                __data[3*i+1] = 255-lb;
                                __data[3*i+2] = 255;
                                break;
                        case 5:
                                __data[3*i+0] = 0;
                                __data[3*i+1] = 0;
                                __data[3*i+2] = 255-lb;
                                break;
                        default:
                                __data[3*i+0] = 0;
                                __data[3*i+1] = 0;
                                __data[3*i+2] = 0;
                                break;
                }
        }
		return __data;
}

inline char *KinectCamera::getFrame( bool wait )
{
		int nRetVal;
		if (__source==DEPTH_CAMERA || __source==DEPTH_CAMERA_COLOR)
			nRetVal = __context.WaitOneUpdateAll(__depth); 
		else if (__source==IR_CAMERA)
			nRetVal = __context.WaitOneUpdateAll(__ir); 
		else nRetVal = __context.WaitOneUpdateAll(__image); 
		//int nRetVal = __context.WaitAndUpdateAll(); 
		if (nRetVal != XN_STATUS_OK) 
		{ 
			printf("Failed updating data: %s\n", xnGetStatusString(nRetVal)); 
			return NULL;
		} 
		
		if (__source==DEPTH_CAMERA || __source==DEPTH_CAMERA_COLOR)
		{
			// Take current depth map 
			__depth.GetMetaData(__depthMD);
			//const XnDepthPixel* pDepthMap = __depth.GetDepthMap();
			const XnDepthPixel* pDepthMap=__depthMD.Data();
			if (__source==DEPTH_CAMERA_COLOR) pixelBuffer=depth2rgb(pDepthMap);
			else pixelBuffer=(char*)pDepthMap;
		}
		else if (__source==IR_CAMERA)
		{
			__ir.GetMetaData(__irMD);
			const XnIRPixel* pIrMap=__irMD.Data();
			for (int i=0;i<640*480;i++)
			{
				__data[i]=pIrMap[i];

			}
			pixelBuffer=__data;

		}
		else
		{
			__image.GetMetaData(__imageMD);
			const XnUInt8* pImageMap=__imageMD.Data();
			pixelBuffer=(char*)pImageMap;
		}
		return pixelBuffer;
}

void KinectCamera::showPropertyPage()
{
//	dlg->Show();
}

VideoManInputController *KinectCamera::getController()
{
	return (VideoManInputController*)( controller );
}

bool KinectCamera::linkController( VideoManInputController *acontroller )
{
	if ( acontroller != NULL && acontroller->setInput(this) )
	{
		controller = acontroller;
		return true;
	}
	return false;
}
void KinectCamera::getFormat( VMInputFormat &_format )
{
	_format= this->format;
	
}
