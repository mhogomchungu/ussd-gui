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

#include "task.h"

#include "language_path.h"

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

static void _callback( GSM_StateMachine * gsm,GSM_USSDMessage * ussd,void * e )
{
	Q_UNUSED( gsm ) ;

	auto f = reinterpret_cast< foo * >( e ) ;

	( *f )( ussd ) ;
}

MainWindow::MainWindow( QWidget * parent ) : QMainWindow( parent ),
	m_ui( new Ui::MainWindow ),
	m_foo( [ this ]( GSM_USSDMessage * ussd ){ this->processResponce( ussd ) ; } ),
	m_settings( "ussd-gui","ussd-gui" )
{
	this->setLocalLanguage() ;

	m_ui->setupUi( this ) ;

	QCoreApplication::setApplicationName( "ussd-gui" ) ;

	this->setWindowIcon( QIcon( ":/ussd-gui" ) ) ;

	this->setFixedSize( this->size() ) ;

	connect( m_ui->pbCancel,SIGNAL( pressed() ),this,SLOT( pbQuit() ) ) ;
	connect( m_ui->pbSend,SIGNAL( pressed() ),this,SLOT( pbSend() ) ) ;

	QString e = QDir::homePath() + "/.config/ussd-gui" ;

	QDir d ;
	d.mkpath( e ) ;

	m_settings.setPath( QSettings::IniFormat,QSettings::UserScope,e ) ;

	this->setUpDevice() ;

	m_history = this->getSetting( "history" ) ;

	if( !m_history.isEmpty() ){

		m_ui->lineEditUSSD_code->setText( m_history.split( "\n",QString::SkipEmptyParts ).first() ) ;
		m_ui->pbHistory->setToolTip( this->setHistoryToolTip() ) ;
	}
}

QString MainWindow::setHistoryToolTip()
{
	return tr( "history:" ) + "\n" + m_history ;
}

MainWindow::~MainWindow()
{
	delete m_ui ;

	if( m_gsm ){

		GSM_TerminateConnection( m_gsm ) ;
		GSM_FreeStateMachine( m_gsm ) ;
	}
}

bool MainWindow::deviceIsConnected()
{
	return GSM_IsConnected( m_gsm ) ;
}

QString MainWindow::getSetting( const QString& opt )
{
	if( m_settings.contains( opt ) ){

		return m_settings.value( opt ).toString() ;
	}else{
		return QString() ;
	}
}

void MainWindow::setSetting( const QString& key, const QString& value )
{
	m_settings.setValue( key,value ) ;
}

void MainWindow::ConnectStatus()
{
	m_connectingMsg += "..." ;
	m_ui->textEditResult->setText( m_connectingMsg ) ;
}

bool MainWindow::initConnection()
{
	m_connectingMsg = tr( "status: connecting " ) ;

	QTimer timer ;

	connect( &timer,SIGNAL( timeout() ),this,SLOT( ConnectStatus() ) ) ;

	this->ConnectStatus() ;

	this->disableSending() ;
	m_ui->pbCancel->setEnabled( false ) ;

	timer.start( 1000 * 1 ) ;

	auto error = Task::await< GSM_Error >( [ this ](){ return GSM_InitConnection( m_gsm,1 ) ; } ) ;

	timer.stop() ;

	if( error != ERR_NONE ){

		m_ui->textEditResult->setText( tr( "ERROR 1: " ) + GSM_ErrorString( error ) ) ;

		this->enableSending() ;
		m_ui->pbCancel->setEnabled( true ) ;

		return false ;
	}else{
		m_ui->textEditResult->setText( tr( "status: connected" ) ) ;

		m_ui->pbSend->setEnabled( false ) ;

		_suspend_for_one_second() ;

		GSM_SetIncomingUSSDCallback( m_gsm,_callback,reinterpret_cast< void * >( &m_foo ) ) ;

		error = GSM_SetIncomingUSSD( m_gsm,true ) ;

		if( error != ERR_NONE ){

			m_ui->textEditResult->setText( tr( "ERROR 2: " ) + GSM_ErrorString( error ) ) ;

			return false ;
		}else{
			return true ;
		}
	}
}

void MainWindow::updateHistory( const QByteArray& e )
{
	QStringList l = m_history.split( "\n",QString::SkipEmptyParts ) ;

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
}

void MainWindow::pbSend()
{
	auto _send = [ this ](){

		QByteArray ussd = m_ui->lineEditUSSD_code->text().toLatin1() ;

		if( ussd.startsWith( "*" ) ){

			this->updateHistory( ussd ) ;

			this->setSetting( "history",m_history ) ;

			m_ui->pbHistory->setToolTip( this->setHistoryToolTip() ) ;
		}

		this->disableSending() ;

		m_ui->textEditResult->setText( tr( "status: sending a request" ) ) ;

		_suspend_for_one_second() ;

		auto error = GSM_DialService( m_gsm,ussd.data() ) ;

		if( error != ERR_NONE ){

			m_ui->textEditResult->setText( tr( "ERROR 3: " ) + GSM_ErrorString( error ) ) ;

			this->enableSending() ;
		}else{


			QString e( tr( "waiting for a reply ..." ) ) ;

			m_ui->pbCancel->setEnabled( false ) ;

			int r = 0 ;

			while( true ){

				if( r == 60 ){

					m_ui->textEditResult->setText( tr( "ERROR 6: no response within 1 minute." ) ) ;

					this->enableSending() ;

					GSM_SetIncomingUSSD( m_gsm,false ) ;

					break ;
				}else{
					r++ ;

					if( GSM_ReadDevice( m_gsm,false ) == 0 ){

						m_ui->textEditResult->setText( e ) ;

						e += "...." ;

						_suspend_for_one_second() ;
					}else{
						break ;
					}
				}
			}

			m_ui->pbCancel->setEnabled( true ) ;
		}
	} ;

	if( this->deviceIsConnected() ){

		_send() ;
	}else{
		if( this->initConnection() ){

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

void MainWindow::processResponce( GSM_USSDMessage * ussd )
{
	auto _error = []( GSM_USSDMessage * ussd ){

		switch( ussd->Status ){

		case USSD_NoActionNeeded:

			return tr( "Status: No action needed" ) ;

		case USSD_ActionNeeded:

			return tr( "Status: Action needed" ) ;

		case USSD_Terminated:

			return tr( "ERROR 7: connection was terminated" ) ;

		case USSD_AnotherClient:

			return tr( "ERROR 7: another client replied" ) ;

		case USSD_NotSupported:

			return tr( "ERROR 7: ussd code is not supported" ) ;

		case USSD_Timeout:

			return tr( "ERROR 7: connection timeout" ) ;

		case USSD_Unknown:

			return tr( "ERROR 7: unknown error has occured" ) ;

		default:
			return tr( "ERROR 7: unknown error has occured" ) ;
		}
	} ;

	/*
	 * this routine converts a unicode C string like "004F004D00470021"
	 * into a presentable string.
	 */
	auto _convert_unicode_C_string_to_qstring = []( const char * e ){

		auto _convert_base_16_to_base_10 = []( const char * e ){

			auto _convert_hex_to_decimal = []( const char * e ){

				char a = *e ;

				if( a >= 'A' && a <= 'F' ){

					return a - 'A' + 10 ;

				}else if( a >= 'a' && a <= 'f' ){

					return a - 'a' + 10 ;
				}else{
					return a - '0' ;
				}
			} ;

			return _convert_hex_to_decimal( e ) * 16 + _convert_hex_to_decimal( e + 1 ) ;
		} ;

		unsigned short buffer[ GSM_MAX_USSD_LENGTH + 1 ] = { 0 } ;

		for( int i = 0 ; *e ; e += 4,i++ ){

			*( buffer + i ) = _convert_base_16_to_base_10( e ) + _convert_base_16_to_base_10( e + 2 ) ;
		}

		return QString::fromUtf16( buffer ) ;
	} ;

	this->enableSending() ;

	if( ussd->Status == USSD_ActionNeeded ){

		m_ui->lineEditUSSD_code->setText( QString() ) ;
	}
	if( ussd->Status == USSD_ActionNeeded || ussd->Status == USSD_NoActionNeeded ){

		const char * e = DecodeUnicodeString( ussd->Text ) ;

		/*
		 * different network operators seem to return the result in different formats,
		 * just going with what works for me for now until i know more about this.
		 */
		if( false ){

			m_ui->textEditResult->setText( e ) ;
		}else{
			m_ui->textEditResult->setText( _convert_unicode_C_string_to_qstring( e ) ) ;
		}
	}else{
		m_ui->textEditResult->setText( _error( ussd ) ) ;
	}
}

void MainWindow::setUpDevice()
{
	GSM_InitLocales( nullptr ) ;

	m_gsm = GSM_AllocStateMachine() ;

	INI_Section * cfg = nullptr ;

	auto error = GSM_FindGammuRC( &cfg,nullptr ) ;

	if( error != ERR_NONE  ){

		m_ui->textEditResult->setText( tr( "ERROR 4: " ) + GSM_ErrorString( error ) ) ;

		this->disableSending() ;
	}else{
		error = GSM_ReadConfig( cfg,GSM_GetConfig( m_gsm,0 ),0 ) ;

		if( error != ERR_NONE  ){

			m_ui->textEditResult->setText( tr( "ERROR 5: " ) + GSM_ErrorString( error ) ) ;

			this->disableSending() ;
		}

		INI_Free( cfg ) ;

		GSM_SetConfigNum( m_gsm,1 ) ;
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
