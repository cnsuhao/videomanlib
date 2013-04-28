#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include <windows.h>
#include <GL/freeglut.h>
#include <iostream>
#include <string>
#include <vector>

#include "VideoManControl.h"

using namespace std;
using namespace VideoMan;

/*
This is an example using VideoMan with DirectShow and OpenGL.
Multiple video files are initilized using DirectShow
To use this example, VideoMan must be built with the directive VM_OGLRenderer, 
also you need to build the input VMDirectShow
*/

VideoManControl videoMan;
int screenLeft, screenUp, screenWidth, screenHeight;
bool fullScreened;
double videoLength;
int visualMode = 0;
int mainInput = 0;
std::vector< int > videoInputIDs; //List of indexes of the initialized inputs

size_t maxVideos = 6;
bool renderAudio = true;
char *dirPath = 0;


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
		case GLUT_KEY_F2:
		{
			visualMode = (visualMode + 1 ) %9;
			videoMan.changeVisualizationMode( visualMode);
			break;
		}		
		case GLUT_KEY_F3:
		{
			mainInput = ( mainInput + 1 ) %videoInputIDs.size();
			videoMan.changeMainVisualizationInput( videoInputIDs[mainInput] );
			break;
		}
    }
}

void loadFiles( string dirPath, vector< string > &files )
{
	WIN32_FIND_DATA fd;
	DWORD dwAttr = FILE_ATTRIBUTE_DIRECTORY;
	const std::string path  = dirPath + "/*.*";
	HANDLE hFind = FindFirstFile( path.c_str(), &fd);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		FindClose( hFind);
		return;
	}
	do
	{
		if ( !( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			std::string fileName = dirPath + "/" + fd.cFileName;			 
			files.push_back( fileName );
		}
	}while( FindNextFile( hFind, &fd));
	FindClose( hFind);
}

bool InitializeVideoMan()
{
	if ( !dirPath )
		return false;
	vector< string > files;
	loadFiles( dirPath, files );

	VMInputFormat format;	
	VMInputIdentification device;

	for ( size_t v = 0; v < files.size() && videoInputIDs.size() < maxVideos; ++v )
	{
		//Initialize one input from a video file
		device.fileName = new char[files[v].length() + 1];
		strcpy( device.fileName, files[v].c_str() );
		device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
		//play in real-time
		format.clock = true;
		format.renderAudio = renderAudio;
		int inputID;
		if ( ( inputID = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			cout << endl;
			if ( device.fileName )
				cout << "Loaded video file: " << device.fileName << endl;
			cout << "resolution: " << format.width <<" " << format.height << endl;
			//get the length of the video
			videoLength = videoMan.getLengthSeconds( inputID );
			printf("duration: %f seconds\n\n", videoLength );
			videoInputIDs.push_back( inputID );
		}
		delete device.fileName;
	}
	for ( size_t v = 0; v < videoInputIDs.size(); ++v )
		videoMan.playVideo( videoInputIDs[v] );

	//We want to display all the intialized video inputs
	videoMan.activateAllVideoInputs();	

	return ( videoInputIDs.size() > 0);
}


void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );
	//For each initialized inputs
	for ( size_t n=0; n < videoInputIDs.size(); n++ )
	{
		//Get a new frame from input n
		char *image = videoMan.getFrame( videoInputIDs[n] );
		if ( image != NULL )
		{
			//Update the texture of the renderer
			videoMan.updateTexture( videoInputIDs[n] ); 
            
			/*
				Process the image...
			*/

			//Release the frame
			videoMan.releaseFrame( videoInputIDs[n] );
		}
		//render the image of input n in the screen
		videoMan.renderFrame( videoInputIDs[n] ); 
	}

	glFlush();
    glutSwapBuffers();
}


void showHelp()
{
	cout << "========" << endl;
	cout << "keys:" << endl;
	cout << "Esc->Exit" << endl;
	cout << "F1->Fullscreen" << endl;
	cout << "F2->Switch Visualization Mode" << endl;
	cout << "F3->Switch Main Input" << endl;
	cout << "========" << endl;
}

int main(int argc, char** argv)
{
	cout << "Multiple video files are initilized using VMDirectShow" << endl;
	cout << "Usage: VMwithDirectShowMultiVideo.exe directoryPath(string) playAudio(0/1)" << endl;
	cout << "Example: VMwithDirectShowMultiVideo.exe c:\\MyVideos 0" << endl;
	cout << "=====================================================" << endl;

	if ( argc > 1 )
		dirPath = argv[1];
	else
	{
		showHelp();
		cout << "Error: A path to a folder containing videos is needed" << endl;
		cout << "Pres Enter to exit" << endl;
		std::getchar();
		return -1;
	}
	if ( argc > 2 )
	{
		string num = argv[2];
		renderAudio = ( num != "0" );
	}

	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan with DirectShow");
	glutHideWindow();
	
	if ( !InitializeVideoMan() )
	{
		showHelp();
		cout << "Error intializing VideoMan" << endl;
		cout << "Pres Enter to exit" << endl;		 
		getchar();
		return -1;
	}

	showHelp();
	glutShowWindow();
	glutReshapeFunc(glutResize);
    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutDisplay);
    glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecialKeyboard);
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); 
	
	fullScreened = false;	
    glutMainLoop();
	return 0;
}