#include "cameraSelector.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>

CameraSelector::CameraSelector( QWidget *parent, bool multipleSelection, bool cancelButton )
	: QDialog (parent)
{
	setWindowFlags( Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint );
	setMinimumSize( 200, 100 );	
	
	textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setLineWrapMode(QTextEdit::NoWrap);

	//listaCamaras = new QComboBox();	
	
    botonCancelar = new QPushButton(tr("&Cancel"));
    connect(botonCancelar, SIGNAL(clicked()), this, SLOT(close()));

	botonAceptar = new QPushButton(tr("&OK"));
    connect(botonAceptar, SIGNAL(clicked()), this, SLOT(Aceptar()));

	QHBoxLayout *layoutH = new QHBoxLayout;
	layoutH->addWidget(botonAceptar);
    layoutH->addWidget(botonCancelar);
	if ( !cancelButton )
		botonCancelar->setEnabled( false );

    QVBoxLayout *layout = new QVBoxLayout;
    listWidget = new QListWidget();	
	layout->addWidget(listWidget);
    layout->addLayout(layoutH);
	if ( multipleSelection )
	{
		listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
		setWindowTitle(tr("Select the devices"));
	}
	else
	{
		listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
		setWindowTitle(tr("Select the device"));
	}
    setLayout(layout);

	accepted = false;
}

CameraSelector::~CameraSelector()
{

}

void CameraSelector::Aceptar()
{
	accepted = true;
	close();
}

void CameraSelector::getSelectedIndexes( std::vector<int> &selectedIndexes )
{	
	QList<QListWidgetItem *> selected = listWidget->selectedItems();
	for ( int s = 0; s < selected.size(); ++s )
	{
		int index = listWidget->row(selected[s]);
		selectedIndexes.push_back( index );
	}
}

bool CameraSelector::isAccepted()
{
	return accepted;
}