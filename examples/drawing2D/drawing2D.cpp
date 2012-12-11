#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <vector>
#include "cxcore.h"
#include "highgui.h"

#include "VideoManControl.h"

using namespace std;
using namespace VideoMan;

/*
This is an example about 2D drawing, zoom renderer functionality and OpenGL-OpenCV compatibility. 
One user input is initialized form an image file
To use this example, VideoMan must be built with the directive VM_OGLRenderer
This example also shows how to print 2d text using code by Sven Olsen from http://nehe.gamedev.net
The font is loaded using the library Freetype http://www.freetype.org
If you want to use FreeType in your project please check the license
http://freetype.sourceforge.net/license.html
*/

VideoManControl videoMan;
int screenLeft, screenUp, screenWidth, screenHeight;
bool fullScreened;

int inputID; //Index of the video input
IplImage *img; //Input image

std::vector<CvPoint2D32f> pointsList; //List of created points

#include "FreeType.h"

freetype::font_data font;

void glutResize(int width, int height)
{
	screenLeft = 0;
	screenUp = 0;
	screenWidth = width;
	screenHeight = height;
	//Notify to VideoMan the change of the screen size
	videoMan.changeScreenSize( screenLeft, screenUp, screenWidth, screenHeight );
}

void clear()
{
	cvReleaseImage( &img );
}


void glutMouseFunc( int button, int mouseState, int x, int y )
{
	//Draw a new point
	if ( mouseState == GLUT_DOWN && button == GLUT_LEFT_BUTTON )
	{
		float xf = (float)x;
		float yf = (float)y;
		//Transform from screen coordinates to image coordinates
		//Check if the point is inside the input region
		if ( videoMan.screenToImageCoords( xf, yf ) == inputID )
		{
			//Draw the circle in the image			
			int xi = xf;
			int yi = yf;
			cvCircle( img, cvPoint( (int)xi, (int)yi ), 6, CV_RGB( 0, 255, 0 ), 2 );
			cout << "Screen coords: " << x << " " << y << endl;
			cout << "Image coords: " << xf << " " << yf << endl;
			
			pointsList.push_back( cvPoint2D32f( (int)xi+0.5, (int)yi+0.5 ) );
		}
		//We need to update the texture
		videoMan.updateTexture( inputID  );
		glutPostRedisplay();
	}
}

void glutKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
		{
			clear();
			exit(0);
		}
		case 'a':
		case 'A':
		{
			videoMan.setKeepAspectRatio( inputID, !videoMan.getKeepAspectRatio( inputID ) );
			break;
		}
		case 'h':
		case 'H':
		{
			videoMan.setHorizontalFlip( inputID , !videoMan.getHorizontalFlip( inputID  ) );
			break;
		}
		case 'v':
		case 'V':
		{
			videoMan.setVerticalFlip( inputID , !videoMan.getVerticalFlip( inputID  ) );
			break;
		}
		case 'r':
		case 'R':
		{
			videoMan.resetRendererZoom( inputID );
			break;
		}
		case '+':
		{
			//Increase zoom
			float viewX, viewY, zoom;
			videoMan.getRendererZoom( inputID , viewX, viewY, zoom );
			zoom += 0.5f;
			videoMan.setRendererZoom( inputID , viewX, viewY, zoom, true );
			break;
		}
		case '-':
		{
			//Decrease zoom
			float viewX, viewY, zoom;
			videoMan.getRendererZoom( inputID , viewX, viewY, zoom );			
			zoom -= 0.5f;
			videoMan.setRendererZoom( inputID , viewX, viewY, zoom, true );
			break;
		}
	}
	glutPostRedisplay();
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
		case GLUT_KEY_UP:
		{
			//Move up the zoom center
			float viewX, viewY, zoom;
			videoMan.getRendererZoom( inputID, viewX, viewY, zoom );
			viewY += 10;
			videoMan.setRendererZoom( inputID, viewX, viewY, zoom, true  );
			break;
		}
		case GLUT_KEY_DOWN:
		{
			//Move down the zoom center
			float viewX, viewY, zoom;
			videoMan.getRendererZoom( inputID, viewX, viewY, zoom  );
			viewY -= 10;
			videoMan.setRendererZoom( inputID, viewX, viewY, zoom, true  );
			break;
		}
		case GLUT_KEY_LEFT:
		{
			//Move left the zoom center
			float viewX, viewY, zoom;
			videoMan.getRendererZoom( inputID, viewX, viewY, zoom );
			viewX -= 10;
			videoMan.setRendererZoom( inputID, viewX, viewY, zoom, true  );
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			//Move right the zoom center
			float viewX, viewY, zoom;
			videoMan.getRendererZoom( inputID, viewX, viewY, zoom );
			viewX += 10;
			videoMan.setRendererZoom( inputID, viewX, viewY, zoom, true );
			break;
		}
    }
	glutPostRedisplay();
}


bool InitializeVideoMan()
{
	inputID = -1;
	VMInputFormat format;
	VMInputIdentification device;
	videoMan.setTextureFiltering( 0 );
	//Initialize one user input	
	device.identifier = "USER_INPUT"; //using directshow			
	format.SetFormat( img->width, img->height, 0, BGR24, BGR24 );
	if ( img->widthStep % 4 != 0 )
	{
		if ( img->widthStep % 8 == 0 )
			format.align = 8;
		else
			format.align = 1;
	}
	if ( ( inputID = videoMan.addVideoInput( device, &format ) ) != -1 )	
		videoMan.setUserInputImage( inputID, img->imageData );
	videoMan.setKeepAspectRatio( inputID, true );
	videoMan.activateAllVideoInputs();
	videoMan.updateTexture( inputID );
	videoMan.setVerticalFlip( inputID, img->origin == 0 );

	return ( inputID != -1 );
}


void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );		
	//We really do not need to update the texture in this example
	//videoMan.updateTexture( videoInputID );	
	//render the image of input n in the screen
	videoMan.renderFrame( inputID );

	//Draw the point with image coordinates
	videoMan.activate2DDrawingSetup( inputID );
	glDisable( GL_TEXTURE_2D );
	glColor3f( 1.0, 0.0f, 0.0f );
	glBegin( GL_LINES );
	for ( size_t p = 0; p < pointsList.size(); ++p )
	{
		glVertex2f( pointsList[p].x - 15.0f, pointsList[p].y );
		glVertex2f( pointsList[p].x + 15.0f, pointsList[p].y );
		glVertex2f( pointsList[p].x, pointsList[p].y - 15.0f );
		glVertex2f( pointsList[p].x, pointsList[p].y + 15.0f );
	}
	glEnd();
	
	//Calculate the points coordinates in screen coordinates space
	std::vector<CvPoint2D32f> screenPointsList; //List of created points
	for ( size_t p = 0; p < pointsList.size(); ++p )
	{
		float x = pointsList[p].x;
		float y = pointsList[p].y;
		//imageToScreenCoords can't be used after glBegin
		videoMan.imageToScreenCoords( inputID, x, y );
		screenPointsList.push_back( cvPoint2D32f(x, y ) );		
	}
	//Draw the point with screen coordinates
	int left, bottom, width, height;
	videoMan.getScreenCoordsArea( inputID, left, bottom, width, height );
	glViewport( left, bottom, width, height );
	glMatrixMode( GL_PROJECTION );	
	glLoadIdentity();
	glOrtho( left, left + width, bottom, bottom + height, -1.0, 1.0 ); 
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glColor3f( 0.0, 0.0f, 1.0f );
	glBegin( GL_LINES );
	for ( size_t p = 0; p < screenPointsList.size(); ++p )
	{
		float x = screenPointsList[p].x;
		float y = screenPointsList[p].y;
		glVertex2f( x - 10.0f, y - 10.0f );
		glVertex2f( x + 10.0f, y + 10.0f );
		glVertex2f( x + 10.0f, y - 10.0f );
		glVertex2f( x - 10.0f, y + 10.0f );
	}
	glEnd();

	videoMan.drawInputBorder( inputID, 1.0f, 0.6f, 0.6f, 0.6f );

	//Print text
	glEnable( GL_BLEND );
	glViewport( 0, 0, screenWidth, screenHeight );
	glMatrixMode( GL_PROJECTION );	
	glLoadIdentity();
	glOrtho( 0, screenWidth, 0, screenHeight, -1.0, 1.0 ); 
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glColor4f( 1.0f, 1.0f, 1.0f, 0.9f);
	freetype::print( font, 10, 70, "VideoMan Library 2010 \nhttp://videomanlib.sourceforge.net \nJavier Barandiaran Martirena" );
	glDisable( GL_BLEND );

    glutSwapBuffers();
}


void showHelp()
{
	cout << "========" << endl;
	cout << "keys:" << endl;
	cout << "Esc->Exit" << endl;
	cout << "F1->Fullscreen" << endl;
	cout << "H/V->Horizontal and vertical flip" << endl;
	cout << "A->Keep aspect ratio or not" << endl;
	cout << "+/-->Increase and decrease zoom" << endl;
	cout << "Arrows->Move the center of the zoom)" << endl;
	cout << "R->Reset zoom" << endl;
	cout << "Left button->Draw a new point" << endl;
	cout << "========" << endl;
}

int main(int argc, char** argv)
{
	cout << "This is an example about 2D drawing, zoom renderer functionality and OpenGL-OpenCV compatibility" << endl;	
	cout << "Draw points with the left button:" << endl;	
	cout << "-Green points: Drew over the image with OpenCV" << endl;	
	cout << "-Red cross: Drew over the screen with OpenGL (image coordinates)" << endl;	
	cout << "-Blue X: Drew over the screen with OpenGL (screen coordinates)" << endl;	
	cout << "Usage: drawing2D.exe imageFilePath(string)" << endl;
	cout << "Example: drawing2D.exe c:\\image.jpg" << endl;	
	if ( argc > 1 )
		img = cvLoadImage( argv[1], CV_LOAD_IMAGE_COLOR );		
	else
	{
		cout << "You must specify a path to an image file" << endl;
		return -1;
	}
	
	if ( !img )
	{
		string file = argv[1];
		cout << "Error loading image " << file << endl;
		return -1;
	}
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("2D Drawing demo");
	glutReshapeFunc(glutResize );  
    glutDisplayFunc(glutDisplay);
	//glutIdleFunc(glutDisplay);
    glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecialKeyboard);
	glutMouseFunc( glutMouseFunc );

	if ( !InitializeVideoMan() )
		return -1; 

	fullScreened = false;

	//Load the font	
	font.init( "font.ttf", 16 );
	
	showHelp();
    
	glutMainLoop();
	return 0;
}
