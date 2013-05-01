#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/freeglut.h>
#include "cv.h"
#include <iostream>
#include <string>

#include "VideoManControl.h"

using namespace std;
using namespace VideoMan;

/*
This is the opencv example using optical flow.
If no argument is passed, a camera is initialized.
You can pass the path to a video file (you need the VMDirectshow module)
*/

VideoManControl videoMan;
int screenLeft, screenUp, screenWidth, screenHeight;
size_t videoInput;
bool fullScreened;
double videoLength;
VMInputFormat format;
char *fileName = 0;

//LKdemo sample data

IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp;
IplImage *frame = 0;

int win_size = 10;
const int MAX_COUNT = 500;
CvPoint2D32f* points[2] = {0,0}, *swap_points;
char* status = 0;
int featuresCount = 0;
int need_to_init = 0;
int night_mode = 0;
int flags = 0;
int add_remove_pt = 0;
CvPoint pt;


void glutResize(int width, int height)
{
	screenLeft = 0;
	screenUp = 0;
	screenWidth = width;
	screenHeight = height;
	//Notify to VideoMan the change of the screen size
	videoMan.changeScreenSize( screenLeft, screenUp, screenWidth, screenHeight );
}


void glutKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
		{
			glutLeaveMainLoop();
			break;
		}
		case 'r':
            need_to_init = 1;
            break;
        case 'c':
            featuresCount = 0;
            break;
        case 'n':
            night_mode ^= 1;
            break;
        default:
			break;
	}
}


void glutSpecialKeyboard(int value, int x, int y)
{
	switch (value)
    {
		case GLUT_KEY_F1:
		{
			if ( !fullScreened )
				glutFullScreen();
			else
			{
				glutPositionWindow( 0, 20 );
				glutReshapeWindow( 640, 480 );
			}
			fullScreened = !fullScreened;
			break;
		}		
    }
}

bool InitializeVideoMan()
{
	VMInputIdentification device;

	if ( fileName )
	{
		//Initialize one input from a video file
		device.fileName = fileName; //file name
		bool withHighgui = false;
		if ( videoMan.supportedIdentifier( "DSHOW_VIDEO_FILE" ) )
			device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
		else if ( videoMan.supportedIdentifier( "HIGHGUI_VIDEO_FILE" ) )
		{
			device.identifier = "HIGHGUI_VIDEO_FILE"; //using highugui	
			withHighgui = true;
		}
		if ( ( videoInput = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			if ( device.fileName )
				printf("Loaded video file: %s\n", device.fileName );
			printf("resolution: %d %d\n", format.width, format.height );

			//get the length of the video
			videoLength = videoMan.getLengthSeconds( videoInput );
			videoMan.playVideo( videoInput );
			if ( withHighgui )
				videoMan.setVerticalFlip( videoInput, true );
			printf("duration: %f seconds\n\n", videoLength );
		}
		else 
			return false;
	}
	else
	{
		//Initialize one input from a camera
		VMInputIdentification *list;
		int numDevices;
		videoMan.getAvailableDevices( &list, numDevices ); //list all the available devices
		//Intialize on of the devices
		int d = 0;
		videoInput = -1;
		while ( d < numDevices && videoInput == -1 )
		{
			device = list[d];
			format.showDlg = true;
			if ( ( videoInput = videoMan.addVideoInput( device, &format ) ) != -1 )
			{
				videoMan.showPropertyPage( videoInput );
				videoMan.getFormat( videoInput, format );
				if ( device.friendlyName )
					printf("Initilized camera: %s\n", device.friendlyName );
				printf("resolution: %d %d\n", format.width, format.height );
				printf("FPS: %f\n\n", format.fps );
			}			
			d++;
		}		
		videoMan.freeAvailableDevicesList(  &list, numDevices );
		if ( videoInput == -1 )
		{
			printf("There is no available camera\n");
			videoMan.freeAvailableDevicesList(  &list, numDevices );
			return false;
		}
	}
	
	//We want to display all the intialized video inputs
	videoMan.activateAllVideoInputs();	

	return true;
}


void glutDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	char *frameData = videoMan.getFrame( videoInput );
	
	if ( frameData != NULL )
	{
		cvSetImageData( frame, frameData, frame->widthStep );

		cvCopy( frame, image, 0 );
		if ( format.nChannels > 1 )
			cvCvtColor( image, grey, CV_BGR2GRAY );
		else
			cvCopy( frame, grey, 0 );

		videoMan.releaseFrame( videoInput );

        if( night_mode )
            cvZero( image );

        if( need_to_init )
        {
            /* automatic initialization */
            IplImage* eig = cvCreateImage( cvGetSize(grey), 32, 1 );
            IplImage* temp = cvCreateImage( cvGetSize(grey), 32, 1 );
            double quality = 0.01;
            double min_distance = 10;

            featuresCount = MAX_COUNT;
            cvGoodFeaturesToTrack( grey, eig, temp, points[1], &featuresCount,
                                   quality, min_distance, 0, 3, 0, 0.04 );
            cvFindCornerSubPix( grey, points[1], featuresCount,
                cvSize(win_size,win_size), cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
            cvReleaseImage( &eig );
            cvReleaseImage( &temp );

            add_remove_pt = 0;
        }
        else if( featuresCount > 0 )
        {
            cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
                points[0], points[1], featuresCount, cvSize(win_size,win_size), 3, status, 0,
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
            flags |= CV_LKFLOW_PYR_A_READY;
			int i,k;		
            for( i = k = 0; i < featuresCount; i++ )
            {
                if( add_remove_pt )
                {
                    double dx = pt.x - points[1][i].x;
                    double dy = pt.y - points[1][i].y;

                    if( dx*dx + dy*dy <= 25 )
                    {
                        add_remove_pt = 0;
                        continue;
                    }
                }
                
                if( !status[i] )
                    continue;
                
                points[1][k++] = points[1][i];
                //cvCircle( image, cvPointFrom32f(points[1][i]), 3, CV_RGB(0,255,0), -1, 8,0);
				cvLine( image, cvPointFrom32f(points[1][i]) , cvPointFrom32f(points[0][i]), CV_RGB(0,255,0), 2, 4 );
            }
            featuresCount = k;
        }

        if( add_remove_pt && featuresCount < MAX_COUNT )
        {
            points[1][featuresCount++] = cvPointTo32f(pt);
            cvFindCornerSubPix( grey, points[1] + featuresCount - 1, 1,
                cvSize(win_size,win_size), cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
            add_remove_pt = 0;
        }

        CV_SWAP( prev_grey, grey, swap_temp );
        CV_SWAP( prev_pyramid, pyramid, swap_temp );
        CV_SWAP( points[0], points[1], swap_points );
        need_to_init = 0;		
	}	
	
	videoMan.updateTexture( videoInput, image->imageData ); //Update the texture of the renderer
	videoMan.renderFrame( videoInput ); //render the image in the screen	
	

	//Check if the video file (input number 0) has reached the end	
	if ( videoMan.getPositionSeconds( videoInput ) == videoLength )
		videoMan.goToFrame( videoInput, 0 ); //restart from the begining

	glFlush();
    glutSwapBuffers();
}

void showHelp()
{
	printf("========\n");
	printf("keys:\n");	
	printf("Esc->Exit\n");
	printf("F1->Fullscreen\n");
	printf("r->Detected features\n");
	printf("c->Delete features\n");
	printf("n->Night mode Y/N\n");
	printf("Left Button->Create feature\n");
	printf("========\n");
}

void clear()
{
	cvReleaseImageHeader( &frame );
    cvReleaseImage( &image );
    cvReleaseImage( &grey );
    cvReleaseImage( &prev_grey );
    cvReleaseImage( &pyramid );
    cvReleaseImage( &prev_pyramid );
	cvFree( &points[0] );
	cvFree( &points[1] );	
	cvFree( &status );    
}

void glutMouseFunc( int button, int state, int x, int y )
{
	if ( state ==  GLUT_DOWN && button == GLUT_LEFT_BUTTON )
	{
		float xn = (float)x;
		float yn = (float)y;
		videoMan.screenToImageCoords( xn, yn );
		pt = cvPoint((int)xn,(int)yn);
		add_remove_pt = 1;
	}
}

int main(int argc, char** argv)
{
	cout << "This is the opencv example using optical flow. One video input is initialized" << endl;	
	cout << "Usage: VMwithDirectShow.exe filePath(string)" << endl;
	cout << "Example: VMwithDirectShow.exe c:\\video.avi" << endl;
	cout << "If no argument is passed, a camera is initialized" << endl;	
	cout << "=====================================================" << endl;

	if ( argc > 1 ) 
		fileName = argv[1];

	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan-OpenCV LKdemo");
	glutHideWindow();
	
	if ( !InitializeVideoMan() )
	{
		showHelp();
		cout << "Error intializing VideoMan" << endl;
		cout << "Pres Enter to exit" << endl;		 
		getchar();
		return -1;
	}

	/* allocate all the buffers */
	frame = cvCreateImageHeader( cvSize( format.width, format.height ), format.depth, (int)format.nChannels );	
    image = cvCreateImage( cvGetSize(frame), format.depth, (int)format.nChannels );
    image->origin = frame->origin;
    grey = cvCreateImage( cvGetSize(frame), 8, 1 );
    prev_grey = cvCreateImage( cvGetSize(frame), 8, 1 );
    pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
    prev_pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
    points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
    points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
    status = (char*)cvAlloc(MAX_COUNT);
    flags = 0;

	showHelp();
	glutShowWindow();
	glutReshapeFunc(glutResize);
    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutDisplay);
    glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecialKeyboard);
	glutMouseFunc(glutMouseFunc);
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); 
	
	fullScreened = false;
    glutMainLoop();
	clear();
    return 0;
}
