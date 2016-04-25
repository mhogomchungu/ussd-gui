/*
 *
 *  Copyright (c) 2015-2016
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
#include <QStringList>

#include <QTranslator>

#include <QDebug>

#include "language_path.h"

#include "../3rd_party/qgsmcodec.h"

static QStringList _source( QSettings& settings )
{
	auto _option = [ & ]( const QString& key,QString value ){

		if( settings.contains( key ) ){

			return settings.value( key ).toString() ;
		}else{
			settings.setValue( key,value ) ;
			return value ;
		}
	} ;

	return { _option( "source","libgammu" ),_option( "device","/dev/ttyACM0" ) } ;
}

MainWindow::MainWindow( bool log ) :
	m_ui( new Ui::MainWindow ),
	m_settings( "ussd-gui","ussd-gui" ),
	m_gsm( gsm::Source( _source( m_settings ),[ this ]( const gsm::USSDMessage& ussd ){ this->processResponce( ussd ) ; } ) )

{
	this->setLocalLanguage() ;

	m_ui->setupUi( this ) ;

	m_timer.setTextEdit( *( m_ui->textEditResult ) ) ;

	m_ui->pbConnect->setFocus() ;

	m_ui->pbConnect->setDefault( true ) ;

	m_ui->pbSMS->setEnabled( false ) ;

	m_ui->groupBox->setTitle( QString() ) ;

	QCoreApplication::setApplicationName( "ussd-gui" ) ;

	this->setWindowIcon( QIcon( ":/ussd-gui" ) ) ;

	this->setFixedSize( this->size() ) ;

	connect( m_ui->pbConnect,SIGNAL( pressed() ),this,SLOT( pbConnect() ) ) ;
	connect( m_ui->pbCancel,SIGNAL( pressed() ),this,SLOT( pbQuit() ) ) ;
	connect( m_ui->pbSend,SIGNAL( pressed() ),this,SLOT( pbSend() ) ) ;
	connect( m_ui->pbConvert,SIGNAL( pressed() ),this,SLOT( pbConvert() ) ) ;
	connect( m_ui->pbSMS,SIGNAL( pressed() ),this,SLOT( pbSMS() ) ) ;

	connect( &m_menu,SIGNAL( triggered( QAction * ) ),this,SLOT( setHistoryItem( QAction * ) ) ) ;

	connect( this,SIGNAL( displayResultSignal() ),this,SLOT( displayResult() ) ) ;
	connect( this,SIGNAL( updateTitleSignal() ),this,SLOT( updateTitle() ) ) ;
	connect( this,SIGNAL( serverResponseSignal( QString ) ),this,SLOT( serverResponse( QString ) ) ) ;
	connect( this,SIGNAL( enableConvertSignal() ),this,SLOT( enableConvert() ) ) ;

	connect( &m_eventTimer,SIGNAL( timeout() ),&m_eventLoop,SLOT( quit() ) ) ;

	m_ui->pbConvert->setEnabled( false ) ;

	this->disableSending() ;

	auto e = QDir::homePath() + "/.config/ussd-gui" ;

	QDir d ;
	d.mkpath( e ) ;

	m_settings.setPath( QSettings::IniFormat,QSettings::UserScope,e ) ;

	m_timeout = this->timeOutInterval() ;

	m_autowaitInterval = this->autowaitInterval() ;

	m_history = this->getSetting( "history" ) ;

	auto l = this->historyList() ;

	if( !l.isEmpty() ){

		m_ui->lineEditUSSD_code->setText( l.first() ) ;
	}

	this->setHistoryMenu( l ) ;

	if( !m_gsm->init( log ) ){

		m_ui->textEditResult->setText( QObject::tr( "Status: ERROR 1: " ) + m_gsm->lastError() ) ;

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
}

template< typename T >
static QString _arrange_sms_in_descending_order( T& m )
{
	auto j = m.size() ;

	auto d = m.data() ;

	auto e = QObject::tr( "\nNumber Of Text Messages: %1" ).arg( QString::number( j ) ) ;

	for( decltype( j ) p = 0 ; p < j ; p++ ){

		auto& it = *( d + p ) ;

		for( decltype( j ) q = p + 1 ; q < j ; q++ ){

			auto& xt = *( d + q ) ;

			if( it.date < xt.date ){

				std::swap( it,xt ) ;
			}
		}

		auto _r  = []( bool e ){ return e ? QObject::tr( "Read" ) : QObject::tr( "Not Read" ) ; } ;

		auto _l  = []( bool inSimCard,bool inInbox ){

			if( inSimCard ){

				if( inInbox ){

					return QObject::tr( "SIM's Inbox" ) ;
				}else{
					return QObject::tr( "SIM's Outbox" ) ;
				}
			}else{
				if( inInbox ){

					return QObject::tr( "Phone's Inbox" ) ;
				}else{
					return QObject::tr( "Phone's Outbox" ) ;
				}
			}
		} ;

		auto _d = []( const auto& e ){

			auto f = e.mid( 2 ) ;

			switch( e.mid( 0,2 ).remove( "0" ).toInt() ){

				case 1  : return QObject::tr( "January" )   + f ;
				case 2  : return QObject::tr( "February" )  + f ;
				case 3  : return QObject::tr( "March" )     + f ;
				case 4  : return QObject::tr( "April" )     + f ;
				case 5  : return QObject::tr( "May" )       + f ;
				case 6  : return QObject::tr( "June" )      + f ;
				case 7  : return QObject::tr( "July" )      + f ;
				case 8  : return QObject::tr( "August" )    + f ;
				case 9  : return QObject::tr( "September" ) + f ;
				case 10 : return QObject::tr( "October" )   + f ;
				case 11 : return QObject::tr( "November" )  + f ;
				case 12 : return QObject::tr( "December" )  + f ;
			}

			return e ;
		} ;

		auto l = "\n------------------------------------------------------------------------------------\n" ;

		auto k = QObject::tr( "Number: %1\nDate: %2\nState: %3\nLocation: %4\n\n%5" ) ;

		auto& n = *( d + p ) ;

		e += l + k.arg( n.phoneNumber,_d( n.date ),_r( n.read ),_l( n.inSIMcard,n.inInbox ),n.message ) ;
	}

	return e ;
}

template< typename T >
static T& _remove_duplicate_sms( T& m )
{
	auto j = m.size() ;

	auto d = m.data() ;

	for( decltype( j ) i = 0 ; i < j ; i++ ){

		auto& it = *( d + i ) ;

		if( it.inInbox ){

			decltype( i ) k = i + 1 ;

			while( k < j ){

				/*
				 * Sometimes,a single text message may be split into multiple parts and we
				 * seem to get these parts as if they are independent text messages.This routine
				 * is a cheap attempt and combining these multi part text messages into one by
				 * assuming consercutive text messages that share the same time stamp are a part
				 * of the same text message.
				 */

				const auto& xt = *( d + k ) ;

				if( it.date == xt.date ){

					it.message += xt.message ;

					m.remove( k ) ;

					j-- ;
				}else{
					k++ ;
				}
			}
		}
	}

	return m ;
}

void MainWindow::pbSMS()
{
	m_ui->textEditResult->setText( QString() ) ;

	m_ui->groupBox->setTitle( QString() ) ;

	this->disableSending() ;

	m_ui->pbSMS->setEnabled( false ) ;
	m_ui->pbConnect->setEnabled( false ) ;
	m_ui->pbCancel->setEnabled( false ) ;

	m_ui->textEditResult->setText( tr( "Status: Retrieving Text Messages." ) ) ;

	this->wait() ;

	auto m = m_gsm->getSMSMessages().await() ;

	auto j = m.size() ;

	if( j > 0 ){

		m_ui->groupBox->setTitle( tr( "SMS messages." ) ) ;

		m_ui->textEditResult->setText( _arrange_sms_in_descending_order( _remove_duplicate_sms( m ) ) ) ;
	}else{
		QString e = m_gsm->lastError() ;

		if( e == "No error." ){

			m_ui->textEditResult->setText( tr( "Status: No Text Messages Were Found." ) ) ;
		}else{
			m_ui->textEditResult->setText( tr( "Status: ERROR 7: " ) + e ) ;
		}
	}

	this->enableSending() ;

	m_ui->pbSMS->setEnabled( true ) ;
	m_ui->pbConnect->setEnabled( true ) ;
	m_ui->pbCancel->setEnabled( true ) ;
}

void MainWindow::pbConnect()
{
	this->disableSending() ;

	m_ui->pbConnect->setEnabled( false ) ;

	m_ui->pbConvert->setEnabled( false ) ;

	if( m_gsm->connected() ){

		if( m_gsm->disconnect() ){

			m_ui->pbSMS->setEnabled( false ) ;

			m_ui->pbConnect->setText( tr( "&Connect" ) ) ;

			m_ui->textEditResult->setText( tr( "Status: Disconnected." ) ) ;

			m_ui->lineEditUSSD_code->setText( this->topHistory() ) ;
		}else{
			m_ui->textEditResult->setText( tr( "Status: ERROR 6: " ) + m_gsm->lastError() ) ;
		}
	}else{
		if( this->Connect() ){

			if( m_ui->lineEditUSSD_code->text().isEmpty() ){

				m_ui->lineEditUSSD_code->setText( this->topHistory() ) ;
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

int MainWindow::timeOutInterval()
{
	QString timeOut = "timeout" ;

	if( m_settings.contains( timeOut ) ){

		return m_settings.value( timeOut ).toInt() ;
	}else{
		this->setSetting( timeOut,QString( "30" ) ) ;
		return 30 ;
	}
}

int MainWindow::autowaitInterval()
{
	QString waitInterval = "autowaitInterval" ;

	if( m_settings.contains( waitInterval ) ){

		return m_settings.value( waitInterval ).toInt() ;
	}else{
		m_settings.setValue( waitInterval,QString( "2" ) ) ;

		return 2 ;
	}
}

void MainWindow::setHistoryItem( QAction * ac )
{
	auto e = ac->text() ;

	e.remove( "&" ) ;

	if( e != tr( "Empty History." ) ){

		m_ui->lineEditUSSD_code->setText( e ) ;
	}
}

QStringList MainWindow::historyList()
{
	return m_history.split( "\n",QString::SkipEmptyParts ) ;
}

QString MainWindow::topHistory()
{
	auto l = this->historyList() ;

	if( l.isEmpty() ){

		return QString() ;
	}else{
		return l.first() ;
	}
}

bool MainWindow::Connect()
{
	m_timer.start( tr( "Status: Connecting" ) ) ;

	this->disableSending() ;

	m_ui->pbCancel->setEnabled( false ) ;

	auto connected = m_gsm->connect().await() ;

	m_timer.stop() ;

	m_ui->pbCancel->setEnabled( true ) ;

	if( connected ){

		m_ui->pbSMS->setEnabled( m_gsm->canCheckSms() ) ;

		this->enableSending() ;

		m_ui->pbConnect->setEnabled( true ) ;

		m_ui->pbConnect->setText( tr( "&Disconnect" ) ) ;

		m_ui->textEditResult->setText( tr( "Status: Connected." ) ) ;

		this->wait() ;
	}else{
		m_ui->pbConnect->setText( tr( "&Connect" ) ) ;

		m_ui->textEditResult->setText( tr( "Status: ERROR 2: " ) + m_gsm->lastError() ) ;

		this->disableSending() ;
	}

	return connected ;
}

void MainWindow::updateHistory( const QByteArray& e )
{
	auto l = this->historyList() ;

	if( l.contains( e ) ){

		l.removeOne( e ) ;

		m_history = e ;

		for( const auto& it : l ){

			m_history += "\n" + it ;
		}
	}else{
		auto q = this->getSetting( "no_history" ).split( "\n",QString::SkipEmptyParts ) ;

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
	}

	this->setHistoryMenu() ;
}

void MainWindow::send( const QString& code )
{
	QByteArray ussd ;

	if( code.isEmpty() ){

		ussd = m_ui->lineEditUSSD_code->text().toLatin1() ;

		if( ussd.startsWith( "*" ) ){

			this->updateHistory( ussd ) ;

			this->setSetting( "history",m_history ) ;
		}

		m_autoSend = QString( ussd ).split( ' ',QString::SkipEmptyParts ) ;

		ussd = m_autoSend.first().toLatin1() ;

		m_autoSend.removeFirst() ;
	}else{
		ussd = code.toLatin1() ;

		m_ui->lineEditUSSD_code->setText( code ) ;
	}

	if( ussd.isEmpty() ){

		m_ui->textEditResult->setText( tr( "Status: ERROR 6: ussd code required." ) ) ;

		return ;
	}

	this->disableSending() ;

	m_ui->pbConnect->setEnabled( false ) ;

	m_ui->pbCancel->setEnabled( false ) ;

	m_ui->textEditResult->setText( tr( "Status: Sending A Request." ) ) ;

	this->wait() ;

	m_waiting = true ;

	if( m_gsm->dial( ussd ).await() ){

		auto e = tr( "Status: Waiting For A Reply " ) ;

		auto r = 0 ;

		auto has_no_data = true ;

		while( true ){

			if( r == m_timeout ){

				auto e = QString::number( m_timeout ) ;

				m_ui->textEditResult->setText( tr( "Status: ERROR 3: No Response Within %1 Seconds." ).arg( e ) ) ;

				this->enableSending() ;

				m_gsm->listenForEvents( false ) ;

				break ;
			}else{
				r++ ;

				if( has_no_data ){

					has_no_data = !m_gsm->canRead() ;
				}

				if( m_waiting ){

					m_ui->textEditResult->setText( e ) ;

					e += ".... " ;

					this->wait() ;
				}else{
					break ;
				}
			}
		}
	}else{
		m_ui->textEditResult->setText( tr( "Status: ERROR 4: " ) + m_gsm->lastError() ) ;

		this->enableSending() ;
	}

	m_ui->pbConnect->setEnabled( true ) ;

	m_ui->pbCancel->setEnabled( true ) ;
}

void MainWindow::wait( int interval )
{
	m_eventTimer.start( 1000 * interval ) ;

	m_eventLoop.exec() ;

	m_eventTimer.stop() ;
}

void MainWindow::pbSend()
{
	m_ui->groupBox->setTitle( QString() ) ;

	m_ui->pbConvert->setEnabled( false ) ;

	if( m_gsm->connected() ){

		this->send() ;
	}else{
		if( this->Connect() ){

			this->send() ;
		}
	}
}

void MainWindow::pbQuit()
{
	if( m_ui->pbCancel->isEnabled() ){

		QCoreApplication::quit() ;
	}
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

void MainWindow::updateTitle()
{
	m_ui->groupBox->setTitle( tr( "USSD Server Response." ) ) ;

	this->enableSending() ;
}

void MainWindow::processResponce( const gsm::USSDMessage& ussd )
{
	m_ussd = ussd ;

	emit updateTitleSignal() ;

	m_waiting = false ;

	using _gsm = gsm::USSDMessage ;

	if( m_ussd.Status == _gsm::ActionNeeded || m_ussd.Status == _gsm::NoActionNeeded ){

		if( m_ussd.Status == _gsm::ActionNeeded ){

			emit serverResponseSignal( QString() ) ;
		}

		emit enableConvertSignal() ;

		emit displayResultSignal() ;
	}else{
		auto _error = []( const _gsm& ussd ){

			switch( ussd.Status ){

			case _gsm::NoActionNeeded:

				return tr( "Status: No Action Needed." ) ;

			case _gsm::ActionNeeded:

				return tr( "Status: Action Needed." ) ;

			case _gsm::Terminated:

				return tr( "Status: ERROR 5: Connection Was Terminated." ) ;

			case _gsm::AnotherClient:

				return tr( "Status: ERROR 5: Another Client Replied." ) ;

			case _gsm::NotSupported:

				return tr( "Status: ERROR 5: USSD Code Is Not Supported." ) ;

			case _gsm::Timeout:

				return tr( "Status: ERROR 5: Connection Timeout." ) ;

			case _gsm::Unknown:

				return tr( "Status: ERROR 5: Unknown Error Has Occured." ) ;

			default:
				return tr( "Status: ERROR 5: Unknown Error Has Occured." ) ;
			}
		} ;

		emit serverResponseSignal( _error( m_ussd ) ) ;
	}
}

void MainWindow::enableConvert()
{
	m_ui->pbConvert->setEnabled( true ) ;
}

void MainWindow::serverResponse( QString e )
{
	if( e.isEmpty() ){

		m_ui->lineEditUSSD_code->clear() ;
		m_ui->lineEditUSSD_code->setFocus() ;
	}

	m_ui->textEditResult->setText( e ) ;
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
	if( m_gsm->source() == "internal" ){

		m_ui->textEditResult->setText( m_ussd.Text ) ;
	}else{
		auto e = gsm::decodeUnicodeString( m_ussd.Text ) ;

		if( this->gsm7Encoded() ){

			m_ui->textEditResult->setText( QGsmCodec::fromGsm7BitEncodedtoUnicode( e ) ) ;
		}else{
			m_ui->textEditResult->setText( QGsmCodec::fromUnicodeStringInHexToUnicode( e ) ) ;
		}
	}

	if( m_autoSend.size() > 0 ){

		this->disableSending() ;

		auto first = m_autoSend.first() ;

		m_autoSend.removeFirst() ;

		this->wait( m_autowaitInterval ) ;

		this->send( first ) ;
	}
}

void MainWindow::closeEvent( QCloseEvent * e )
{
	e->ignore() ;
	this->pbQuit() ;
}

void MainWindow::setLocalLanguage()
{
	auto lang = this->getSetting( "language" ) ;

	if( !lang.isEmpty() ){

		auto r = lang.toLatin1() ;

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
