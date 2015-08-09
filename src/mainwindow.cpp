/*
 *
 *  Copyright (c) 2015
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QDir>

#include <QTimer>
#include <QEventLoop>

#include <QTranslator>

#include <QDebug>

#include "language_path.h"

#include "../3rd_party/qgsmcodec.h"

#include "task.h"

static void _suspend( int time )
{
	QEventLoop l ;

	QTimer t ;

	QObject::connect( &t,SIGNAL( timeout() ),&l,SLOT( quit() ) ) ;

	t.start( 1000 * time ) ;

	l.exec() ;
}

static void _suspend_for_one_second()
{
	_suspend( 1 ) ;
}

MainWindow::MainWindow( QWidget * parent ) : QMainWindow( parent ),
	m_ui( new Ui::MainWindow ),
	m_gsm( [ this ]( const gsm::USSDMessage& ussd ){ this->processResponce( ussd ) ; } ),
	m_settings( "ussd-gui","ussd-gui" )
{
	this->setLocalLanguage() ;

	m_ui->setupUi( this ) ;

	QCoreApplication::setApplicationName( "ussd-gui" ) ;

	this->setWindowIcon( QIcon( ":/ussd-gui" ) ) ;

	this->setFixedSize( this->size() ) ;

	connect( m_ui->pbConnect,SIGNAL( pressed() ),this,SLOT( pbConnect() ) ) ;
	connect( m_ui->pbCancel,SIGNAL( pressed() ),this,SLOT( pbQuit() ) ) ;
	connect( m_ui->pbSend,SIGNAL( pressed() ),this,SLOT( pbSend() ) ) ;
	connect( m_ui->pbConvert,SIGNAL( pressed() ),this,SLOT( pbConvert() ) ) ;
	connect( &m_menu,SIGNAL( triggered( QAction * ) ),this,SLOT( setHistoryItem( QAction * ) ) ) ;

	m_ui->pbConvert->setEnabled( false ) ;

	this->disableSending() ;

	QString e = QDir::homePath() + "/.config/ussd-gui" ;

	QDir d ;
	d.mkpath( e ) ;

	m_settings.setPath( QSettings::IniFormat,QSettings::UserScope,e ) ;

	m_history = this->getSetting( "history" ) ;

	QStringList l = this->historyList() ;

	if( !l.isEmpty() ){

		m_ui->lineEditUSSD_code->setText( l.first() ) ;
	}

	this->setHistoryMenu( l ) ;

	if( !m_gsm.init() ){

		m_ui->textEditResult->setText( QObject::tr( "Status: ERROR 1: " ) + m_gsm.lastError() ) ;

		this->disableSending() ;
	}
}

void MainWindow::setHistoryMenu( const QStringList& l )
{
	m_menu.clear() ;

	if( l.isEmpty() ){

		m_menu.addAction( tr( "Empty History." ) ) ;
	}else{
		for( const auto& it : l ){

			m_menu.addAction( it ) ;
		}
	}

	m_ui->pbHistory->setMenu( &m_menu ) ;
}

void MainWindow::setHistoryMenu()
{
	this->setHistoryMenu( this->historyList() ) ;
}

MainWindow::~MainWindow()
{
	delete m_ui ;
}

void MainWindow::pbConnect()
{
	this->disableSending() ;

	m_ui->pbConnect->setEnabled( false ) ;

	m_ui->pbConvert->setEnabled( false ) ;

	if( m_gsm.connected() ){

		if( m_gsm.disconnect() ){

			m_ui->pbConnect->setText( tr( "&Connect" ) ) ;

			m_ui->textEditResult->setText( tr( "Status: Disconnected." ) ) ;
		}else{
			m_ui->textEditResult->setText( tr( "Status: ERROR 6: " ) + m_gsm.lastError() ) ;
		}
	}else{
		if( this->Connect() ){

			if( m_ui->lineEditUSSD_code->text().isEmpty() ){

				m_ui->lineEditUSSD_code->setText( this->historyList().first() ) ;
			}
		}
	}

	m_ui->pbConnect->setEnabled( true ) ;
}

QString MainWindow::getSetting( const QString& opt )
{
	if( m_settings.contains( opt ) ){

		return m_settings.value( opt ).toString() ;
	}else{
		return QString() ;
	}
}

bool MainWindow::getBoolSetting( const QString& opt )
{
	if( m_settings.contains( opt ) ){

		return m_settings.value( opt ).toBool() ;
	}else{
		return false ;
	}
}

void MainWindow::setSetting( const QString& key, const QString& value )
{
	m_settings.setValue( key,value ) ;
}

void MainWindow::setSetting(const QString& key,bool value )
{
	m_settings.setValue( key,value ) ;
}

void MainWindow::connectStatus()
{
	m_ui->textEditResult->setText( m_connectingMsg ) ;
	m_connectingMsg += "..." ;
}

void MainWindow::setHistoryItem( QAction * ac )
{
	auto e = ac->text() ;

	if( e != tr( "Empty History." ) ){

		m_ui->lineEditUSSD_code->setText( e ) ;
	}
}

QStringList MainWindow::historyList()
{
	return m_history.split( "\n",QString::SkipEmptyParts ) ;
}

bool MainWindow::Connect()
{
	m_connectingMsg = tr( "Status: Connecting ..." ) ;

	QTimer timer ;

	connect( &timer,SIGNAL( timeout() ),this,SLOT( connectStatus() ) ) ;

	this->connectStatus() ;

	this->disableSending() ;

	m_ui->pbCancel->setEnabled( false ) ;

	timer.start( 1000 * 1 ) ;

	bool connected = Task::await< bool >( [ this ]{ return m_gsm.connect() ; } ) ;

	timer.stop() ;

	m_ui->pbCancel->setEnabled( true ) ;

	if( connected ){

		this->enableSending() ;

		m_ui->pbConnect->setEnabled( true ) ;

		m_ui->pbConnect->setText( tr( "&Disconnect" ) ) ;

		m_ui->textEditResult->setText( tr( "Status: Connected." ) ) ;

		_suspend_for_one_second() ;
	}else{
		m_ui->pbConnect->setText( tr( "&Connect" ) ) ;

		m_ui->textEditResult->setText( tr( "Status: ERROR 2: " ) + m_gsm.lastError() ) ;

		this->disableSending() ;
	}

	return connected ;
}

void MainWindow::updateHistory( const QByteArray& e )
{
	QStringList l = this->historyList() ;

	if( !l.contains( e ) ){

		QStringList q = this->getSetting( "no_history" ).split( "\n",QString::SkipEmptyParts ) ;

		for( const auto& it : q ){

			if( e.startsWith( it.toLatin1() ) ){

				return ;
			}
		}

		if( l.size() >= 10 ){

			l.removeLast() ;

			l.append( e ) ;
		}

		m_history = e ;

		for( const auto& it : l ){

			m_history += "\n" + it ;
		}
	}else{
		l.removeOne( e ) ;

		m_history = e ;

		for( const auto& it : l ){

			m_history += "\n" + it ;
		}
	}

	this->setHistoryMenu() ;
}

void MainWindow::pbSend()
{
	auto _send = [ this ](){

		QByteArray ussd = m_ui->lineEditUSSD_code->text().toLatin1() ;

		if( ussd.startsWith( "*" ) ){

			this->updateHistory( ussd ) ;

			this->setSetting( "history",m_history ) ;
		}

		this->disableSending() ;

		m_ui->pbConnect->setEnabled( false ) ;

		m_ui->textEditResult->setText( tr( "Status: Sending A Request." ) ) ;

		_suspend_for_one_second() ;

		if( m_gsm.dial( ussd ) ){

			QString e( tr( "Status: Waiting For A Reply ..." ) ) ;

			m_ui->pbCancel->setEnabled( false ) ;

			int r = 0 ;

			while( true ){

				if( r == 30 ){

					m_ui->textEditResult->setText( tr( "Status: ERROR 3: no response within 30 seconds." ) ) ;

					this->enableSending() ;

					m_gsm.listenForEvents( false ) ;

					break ;
				}else{
					r++ ;

					if( m_gsm.hasData() ){

						break ;
					}else{
						m_ui->textEditResult->setText( e ) ;

						e += "...." ;

						_suspend_for_one_second() ;
					}
				}
			}

			m_ui->pbCancel->setEnabled( true ) ;
		}else{
			m_ui->textEditResult->setText( tr( "Status: ERROR 4: " ) + m_gsm.lastError() ) ;

			this->enableSending() ;
		}

		m_ui->pbConnect->setEnabled( true ) ;
	} ;

	m_ui->pbConvert->setEnabled( false ) ;

	if( m_gsm.connected() ){

		_send() ;
	}else{
		if( this->Connect() ){

			_send() ;
		}
	}
}

void MainWindow::pbQuit()
{
	QCoreApplication::quit() ;
}

void MainWindow::disableSending()
{
	m_ui->pbSend->setEnabled( false ) ;
	m_ui->lineEditUSSD_code->setEnabled( false ) ;
	m_ui->labelInput->setEnabled( false ) ;
}

void MainWindow::enableSending()
{
	m_ui->pbSend->setEnabled( true ) ;
	m_ui->lineEditUSSD_code->setEnabled( true ) ;
	m_ui->labelInput->setEnabled( true ) ;
}

void MainWindow::processResponce( const gsm::USSDMessage& ussd )
{
	this->enableSending() ;

	if( ussd.Status == gsm::USSDMessage::ActionNeeded || ussd.Status == gsm::USSDMessage::NoActionNeeded ){

		if( ussd.Status == gsm::USSDMessage::ActionNeeded ){

			m_ui->lineEditUSSD_code->setText( QString() ) ;
		}

		m_ui->pbConvert->setEnabled( true ) ;

		m_ussd.Text = ussd.Text ;

		this->displayResult() ;
	}else{
		auto _error = []( const gsm::USSDMessage& ussd ){

			switch( ussd.Status ){

			case gsm::USSDMessage::NoActionNeeded:

				return tr( "Status: No Action Needed." ) ;

			case gsm::USSDMessage::ActionNeeded:

				return tr( "Status: Action Needed." ) ;

			case gsm::USSDMessage::Terminated:

				return tr( "Status: ERROR 5: Connection Was Terminated." ) ;

			case gsm::USSDMessage::AnotherClient:

				return tr( "Status: ERROR 5: Another Client Replied." ) ;

			case gsm::USSDMessage::NotSupported:

				return tr( "Status: ERROR 5: USSD Code Is Not Supported." ) ;

			case gsm::USSDMessage::Timeout:

				return tr( "Status: ERROR 5: Connection Timeout." ) ;

			case gsm::USSDMessage::Unknown:

				return tr( "Status: ERROR 5: Unknown Error Has Occured." ) ;

			default:
				return tr( "Status: ERROR 5: Unknown Error Has Occured." ) ;
			}
		} ;

		m_ui->textEditResult->setText( _error( ussd ) ) ;
	}
}

/*
 * Different operators seems to return data in different formats and trying to predict data
 * was returned in what format is something i do not want to do and instead,i give the user
 * a button to switch between formats i currently know.
 *
 * currently known formats are:
 * 1. gsm 7 bit encoded string.
 * 2. a string of big endian short values in hex representation. example: "004F004D004700210021"
 */
void MainWindow::pbConvert()
{
	this->setSetting( "gsm7Encoded",!this->gsm7Encoded() ) ;
	this->displayResult() ;
}

bool MainWindow::gsm7Encoded()
{
	QString opt = "gsm7Encoded" ;

	if( m_settings.contains( opt ) ){

		return m_settings.value( opt ).toBool() ;
	}else{
		m_settings.setValue( opt,true ) ;
		return true ;
	}
}

void MainWindow::displayResult()
{
	auto e = gsm::decodeUnicodeString( m_ussd.Text ) ;

	if( this->gsm7Encoded() ){

		m_ui->textEditResult->setText( QGsmCodec::fromGsm7BitEncodedtoUnicode( e ) ) ;
	}else{
		m_ui->textEditResult->setText( QGsmCodec::fromUnicodeStringInHexToUnicode( e ) ) ;
	}
}

void MainWindow::closeEvent( QCloseEvent * e )
{
	e->ignore() ;
	this->pbQuit() ;
}

void MainWindow::setLocalLanguage()
{
	QString lang = this->getSetting( "language" ) ;

	if( !lang.isEmpty() ){

		QByteArray r = lang.toLatin1() ;

		if( r == "english_US" ){
			/*
			 * english_US language,its the default and hence dont load anything
			 */
		}else{
			auto translator = new QTranslator( this ) ;

			translator->load( r.constData(),LANGUAGE_FILE_PATH ) ;
			QCoreApplication::installTranslator( translator ) ;
		}
	}
}
