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

#include "3rd_party/qgsmcodec.h"

#include <cstring>

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
	connect( m_ui->pbConvert,SIGNAL( pressed() ),this,SLOT( pbConvert() ) ) ;
	connect( &m_menu,SIGNAL( triggered( QAction * ) ),this,SLOT( setHistoryItem( QAction * ) ) ) ;

	m_ui->pbConvert->setEnabled( false ) ;

	QString e = QDir::homePath() + "/.config/ussd-gui" ;

	QDir d ;
	d.mkpath( e ) ;

	m_settings.setPath( QSettings::IniFormat,QSettings::UserScope,e ) ;

	this->setUpDevice() ;

	m_history = this->getSetting( "history" ) ;

	QStringList l = this->historyList() ;

	if( !l.isEmpty() ){

		m_ui->lineEditUSSD_code->setText( l.first() ) ;		
	}

	this->setHistoryMenu( l ) ;
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
	m_connectingMsg += "..." ;
	m_ui->textEditResult->setText( m_connectingMsg ) ;
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

bool MainWindow::initConnection()
{
	m_connectingMsg = tr( "Status: Connecting " ) ;

	QTimer timer ;

	connect( &timer,SIGNAL( timeout() ),this,SLOT( connectStatus() ) ) ;

	this->connectStatus() ;

	this->disableSending() ;
	m_ui->pbCancel->setEnabled( false ) ;

	timer.start( 1000 * 1 ) ;

	auto error = Task::await< GSM_Error >( [ this ](){ return GSM_InitConnection( m_gsm,1 ) ; } ) ;

	timer.stop() ;

	if( error != ERR_NONE ){

		m_ui->textEditResult->setText( tr( "Status: ERROR 1: " ) + GSM_ErrorString( error ) ) ;

		this->enableSending() ;
		m_ui->pbCancel->setEnabled( true ) ;

		return false ;
	}else{
		m_ui->textEditResult->setText( tr( "Status: Connected." ) ) ;

		m_ui->pbSend->setEnabled( false ) ;

		_suspend_for_one_second() ;

		GSM_SetIncomingUSSDCallback( m_gsm,_callback,reinterpret_cast< void * >( &m_foo ) ) ;

		error = GSM_SetIncomingUSSD( m_gsm,true ) ;

		if( error != ERR_NONE ){

			m_ui->textEditResult->setText( tr( "Status: ERROR 2: " ) + GSM_ErrorString( error ) ) ;

			return false ;
		}else{
			return true ;
		}
	}
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

		m_ui->textEditResult->setText( tr( "Status: Sending A Request." ) ) ;

		_suspend_for_one_second() ;

		auto error = GSM_DialService( m_gsm,ussd.data() ) ;

		if( error != ERR_NONE ){

			m_ui->textEditResult->setText( tr( "Status: ERROR 3: " ) + GSM_ErrorString( error ) ) ;

			this->enableSending() ;
		}else{
			QString e( tr( "Status: Waiting For A Reply ..." ) ) ;

			m_ui->pbCancel->setEnabled( false ) ;

			int r = 0 ;

			while( true ){

				if( r == 30 ){

					m_ui->textEditResult->setText( tr( "Status: ERROR 6: no response within 30 seconds." ) ) ;

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

	m_ui->pbConvert->setEnabled( false ) ;

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

			return tr( "Status: No Action Needed." ) ;

		case USSD_ActionNeeded:

			return tr( "Status: Action Needed." ) ;

		case USSD_Terminated:

			return tr( "Status: ERROR 7: Connection Was Terminated." ) ;

		case USSD_AnotherClient:

			return tr( "Status: ERROR 7: Another Client Replied." ) ;

		case USSD_NotSupported:

			return tr( "Status: ERROR 7: USSD Code Is Not Supported." ) ;

		case USSD_Timeout:

			return tr( "Status: ERROR 7: Connection Timeout." ) ;

		case USSD_Unknown:

			return tr( "Status: ERROR 7: Unknown Error Has Occured." ) ;

		default:
			return tr( "Status: ERROR 7: Unknown Error Has Occured." ) ;
		}
	} ;

	this->enableSending() ;

	if( ussd->Status == USSD_ActionNeeded ){

		m_ui->lineEditUSSD_code->setText( QString() ) ;
	}
	if( ussd->Status == USSD_ActionNeeded || ussd->Status == USSD_NoActionNeeded ){

		std::memcpy( &m_ussd,ussd,sizeof( m_ussd ) ) ;

		m_ui->pbConvert->setEnabled( true ) ;

		this->displayResult() ;
	}else{
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
	/*
	 * DecodeUnicodeString() is provided by libgammu
	 */
	const char * e = DecodeUnicodeString( m_ussd.Text ) ;

	if( this->gsm7Encoded() ){

		m_ui->textEditResult->setText( QGsmCodec::fromGsm7BitEncodedtoUnicode( e ) ) ;
	}else{
		m_ui->textEditResult->setText( QGsmCodec::fromUnicodeStringInHexToUnicode( e ) ) ;
	}
}

void MainWindow::setUpDevice()
{
	GSM_InitLocales( nullptr ) ;

	m_gsm = GSM_AllocStateMachine() ;

	INI_Section * cfg = nullptr ;

	auto error = GSM_FindGammuRC( &cfg,nullptr ) ;

	if( error != ERR_NONE  ){

		m_ui->textEditResult->setText( tr( "Status: ERROR 4: " ) + GSM_ErrorString( error ) ) ;

		this->disableSending() ;
	}else{
		error = GSM_ReadConfig( cfg,GSM_GetConfig( m_gsm,0 ),0 ) ;

		if( error != ERR_NONE  ){

			m_ui->textEditResult->setText( tr( "Status: ERROR 5: " ) + GSM_ErrorString( error ) ) ;

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
