#ifndef CAMERASELECTOR_H
#define CAMERASELECTOR_H

#include <QDialog>
#include <QComboBox>

QT_BEGIN_NAMESPACE
class QListWidget;
class QTextEdit;
class QPushButton;
QT_END_NAMESPACE

class CameraSelector : public QDialog  
{
	Q_OBJECT

public:
	CameraSelector(QWidget *parent, bool multipleSelection, bool cancelButton = false);
	~CameraSelector();

	void getSelectedIndexes( std::vector<int> &selectedIndexes );	
	bool isAccepted();

	QListWidget *listWidget;

private slots:
 void Aceptar();

private:
	bool accepted; 
	QTextEdit *textEdit;
    QPushButton *botonAceptar;
    QPushButton *botonCancelar;
	QComboBox *listaCamaras;
	
};

#endif // CAMERASELECTOR_H
