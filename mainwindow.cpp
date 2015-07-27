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

#include <QDebug>

#include <QTimer>
#include <QEventLoop>

static void _suspend( int time )
{
	QEventLoop l ;

	QTimer t ;

	QObject::connect( &t,SIGNAL( timeout() ),&l,SLOT( quit() ) ) ;

	t.start( 1000 * time ) ;

	l.exec() ;
}

static char _convert( const char * e )
{
	char a = * e ;

	if( a >= 'A' && a <= 'F' ){

		return a - 'A' + 10 ;

	}else if( a >= 'a' && a <= 'f' ){

		return a - 'a' + 10 ;
	}else{
		return a - '0' ;
	}
}

static char _convert_base_16_to_base_10( const char * e )
{
	return _convert( e ) * 16 + _convert( e + 1 ) ;
}

static char * _convert_hex_to_char_buffer( char * buffer,const char * e,size_t skip,size_t block_size )
{
	size_t r = 0 ;

	while( *e ){

		*( buffer + r ) = _convert_base_16_to_base_10( e + skip ) ;
		e += block_size ;
		r++ ;
	}

	*( buffer + r ) = '\0' ;

	return buffer ;
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
	m_ui->setupUi( this ) ;

	this->setWindowIcon( QIcon( ":/ussd-gui" ) ) ;

	this->setFixedSize( this->size() ) ;

	connect( m_ui->pbCancel,SIGNAL( pressed() ),this,SLOT( pbQuit() ) ) ;
	connect( m_ui->pbSend,SIGNAL( pressed() ),this,SLOT( pbSend() ) ) ;

	QString e = QDir::homePath() + "/.config/ussd-gui" ;

	QDir d ;
	d.mkpath( e ) ;

	m_settings.setPath( QSettings::IniFormat,QSettings::UserScope,e ) ;

	m_ui->lineEditUSSD_code->setText( this->getSetting( "last_cmd" ) ) ;

	this->setUpDevice() ;
}

MainWindow::~MainWindow()
{
	delete m_ui ;

	if( m_gsm ){

		GSM_TerminateConnection( m_gsm ) ;
		GSM_FreeStateMachine( m_gsm ) ;
	}
}

bool MainWindow::deviceIsNotConnected()
{
	return !this->deviceIsConnected() ;
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

bool MainWindow::setConnection()
{
	m_ui->textEditResult->setText( "status: connecting ..." ) ;

	this->disableSending() ;

	_suspend( 2 ) ;

	/*
	 * TODO: Below function hangs the GUI,look into running it from a back ground thread.
	 */
	auto error = GSM_InitConnection( m_gsm,1 ) ;

	this->enableSending() ;

	if( error != ERR_NONE ){

		m_ui->textEditResult->setText( QString( "ERROR 5: " ) + GSM_ErrorString( error ) ) ;

		return false ;
	}else{
		m_ui->textEditResult->setText( "status: connected" ) ;

		m_ui->pbSend->setEnabled( false ) ;

		GSM_SetIncomingUSSDCallback( m_gsm,_callback,reinterpret_cast< void * >( &m_foo ) ) ;

		auto error = GSM_SetIncomingUSSD( m_gsm,true ) ;

		if( error != ERR_NONE ){

			m_ui->textEditResult->setText( QString( "ERROR 1: " ) + GSM_ErrorString( error ) ) ;

			return false ;
		}else{
			return true ;
		}
	}
}

void MainWindow::pbSend()
{
	auto _send = [ this ](){

		QByteArray ussd = m_ui->lineEditUSSD_code->text().toLatin1() ;

		if( ussd.startsWith( "*" ) ){

			this->setSetting( "last_cmd",ussd ) ;
		}

		auto error = GSM_DialService( m_gsm,ussd.data() ) ;

		if( error != ERR_NONE ){

			m_ui->textEditResult->setText( QString( "ERROR 2: " ) + GSM_ErrorString( error ) ) ;
		}else{
			QString e( "waiting for a reply ..." ) ;

			while( true ){

				if( GSM_ReadDevice( m_gsm,TRUE ) == 0 ){

					m_ui->textEditResult->setText( e ) ;
					e += "." ;

					_suspend( 2 ) ;
				}else{
					break ;
				}
			}
		}
	} ;

	if( this->deviceIsConnected() ){

		_send() ;
	}else{
		if( this->setConnection() ){

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
}

void MainWindow::enableSending()
{
	m_ui->pbSend->setEnabled( true ) ;
}

void MainWindow::processResponce( GSM_USSDMessage * ussd )
{
	auto _response = []( GSM_USSDMessage * ussd )->QString{

		switch( ussd->Status ){

		case USSD_NoActionNeeded:

			return "Status: No action needed\n\nServer response below:\n\n" ;

		case USSD_ActionNeeded:

			return "Status: Action needed\n\nServer response below:\n\n" ;

		case USSD_Terminated:

			return "Status: Terminated\n\nServer response below:\n\n" ;

		case USSD_AnotherClient:

			return "Status: Another client replied\n\nServer response below:\n\n" ;

		case USSD_NotSupported:

			return "Status: Not supported\n\nServer response below:\n\n" ;

		case USSD_Timeout:

			return "Status: Timeout\n\nServer response below:\n\n" ;

		case USSD_Unknown:

			return "Status: Unknown\n\nServer response below:\n\n" ;

		default:
			return "Status: Unknown\n\nServer response below:\n\n" ;
		}
	} ;

	char buffer[ GSM_MAX_USSD_LENGTH ] = { '\0' } ;

	_convert_hex_to_char_buffer( buffer,DecodeUnicodeString( ussd->Text ),2,4 ) ;

	m_ui->textEditResult->setText( _response( ussd ) + buffer ) ;

	m_ui->pbSend->setEnabled( true ) ;

	if( ussd->Status == USSD_ActionNeeded ){

		m_ui->lineEditUSSD_code->setText( QString() ) ;
	}
}

/*
 * I have no idea what is going on in this function,i just took all of it from
 * "GetUSSD" function in gammu sources
 */
void MainWindow::setUpDevice()
{
	GSM_Config * smcfg ;
	GSM_Config * smcfg0 ;
	GSM_Debug_Info * di ;

	INI_Section * cfg ;
	GSM_Error error ;

	gboolean debug_level_set = false ;
	gboolean debug_file_set = false ;

	int only_config = -1 ;

	di = GSM_GetGlobalDebug() ;

	auto a = ( const unsigned char * )"gammu" ;

	auto b = ( const unsigned char * )"gammucoding" ;

	error = GSM_FindGammuRC( &cfg,nullptr ) ;

	if( error != ERR_NONE ){

		m_ui->textEditResult->setText( QString( "ERROR 3: " ) + GSM_ErrorString( error ) ) ;
		this->disableSending() ;
		return ;
	}

	auto cp = INI_GetValue( cfg,a,b,false ) ;

	if( cp ){

		GSM_SetDebugCoding( (const char * )cp,di ) ;
	}

	m_gsm = GSM_AllocStateMachine() ;

	smcfg0 = GSM_GetConfig( m_gsm,0 ) ;

	for( auto i = 0 ; ( smcfg = GSM_GetConfig( m_gsm,i ) ) != nullptr ; i++ ){

		if( only_config != -1 ){

			smcfg = smcfg0;
			error = GSM_ReadConfig( cfg,smcfg,only_config ) ;

			if( error != ERR_NONE ){

				GSM_ReadConfig( nullptr,smcfg,0 ) ;
			}
		}else{
			error = GSM_ReadConfig( cfg,smcfg,i ) ;

			if( error != ERR_NONE ){

				if( i != 0 ){

					break;
				}
			}
		}

		GSM_SetConfigNum( m_gsm,GSM_GetConfigNum( m_gsm ) + 1 ) ;

		smcfg->UseGlobalDebugFile = true ;

		if( i == 0 ) {

			if( !debug_level_set ){

				GSM_SetDebugLevel( smcfg->DebugLevel,di ) ;
			}

			if( !debug_file_set ){

				error = GSM_SetDebugFile( smcfg->DebugFile,di ) ;

				if( error != ERR_NONE ){

					m_ui->textEditResult->setText( QString( "ERROR 4: " ) + GSM_ErrorString( error ) ) ;
					this->disableSending() ;
					return ;
				}
			}
		}

		if( i == 0 ){

			a = ( const unsigned char * )"gammu" ;
			b = ( const unsigned char * )"rsslevel" ;

			INI_GetValue( cfg,a,b,false ) ;
		}

		if( only_config != -1 ){

			break ;
		}
	}
}

void MainWindow::closeEvent( QCloseEvent * e )
{
	e->ignore() ;
	this->pbQuit() ;
}
