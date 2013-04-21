#include <QtOpenGL>
class QWidget;

#include "VideoManControl.h"
#include "VideoManInputFormat.h"


class GLWindow : public QGLWidget
{
	Q_OBJECT

public:
	GLWindow( int timerInterval=30, QWidget *parent=0, const char *name=0 );//: QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {}
	~GLWindow(){}

protected:
	void initializeGL();
	void resizeGL( int width, int height );
	void paintGL();

	void keyPressEvent( QKeyEvent *e );

	void mousePressEvent( QMouseEvent *e );
  
protected slots:
	virtual void timeOutSlot();
	virtual void openVideoFile();
	virtual void openCamera();
	virtual void changeMode0();
	virtual void changeMode1();
  
private:
	QTimer *m_timer;
	VideoMan::VideoManControl m_videoMan;
	std::vector< int > m_videoInputIDs; //List of indexes of the initialized inputs
	QString m_dirPath; 
};