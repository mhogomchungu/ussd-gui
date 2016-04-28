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

#include "internal.h"
#include "../3rd_party/qgsmcodec.h"

#include <unistd.h>

#include <QDebug>

internal::internal( const QString& device,const QString& e,std::function< void( const gsm::USSDMessage& ) >&& function ) :
	m_terminatorSequence( e ),m_function( std::move( function ) )
{
	m_read.setFileName( device ) ;
	m_write.setFileName( device ) ;
}

internal::~internal()
{
}

bool internal::disconnect()
{
	m_write.write( "AT+CUSD=2,,15\r" ) ;
	m_read.close() ;

	if( m_read.isOpen() ){

		m_lastError = QObject::tr( "Failed to close reading channel." ).toLatin1() ;
		return false ;
	}

	m_write.close() ;

	if( m_write.isOpen() ){

		m_lastError = QObject::tr( "Failed to close writing channel." ).toLatin1() ;
		return false ;
	}

	return true ;
}

bool internal::connected()
{
	return m_write.isOpen() && m_read.isOpen() ;
}

bool internal::canRead( bool waitForData )
{
	Q_UNUSED( waitForData ) ;
	return false ;
}

Task::future< bool >& internal::hasData( bool waitForData )
{
	Q_UNUSED( waitForData ) ;

	return Task::run< bool >( [](){

		return false ;
	} ) ;
}

void internal::readDevice()
{
	Task::exec( [ this ](){

		QByteArray tmp ;

		QByteArray e ;

		auto _timeout = [ this ](){

			m_ussd.Status = gsm::USSDMessage::Timeout ;
			m_function( m_ussd ) ;
		} ;

		while( true ){

			e = m_read.read( 1 ) ;

			if( e.size() < 1 ){

				return _timeout() ;
			}else{
				tmp += e ;

				if( tmp.endsWith( "+CUSD: " ) ){

					break ;
				}

				if( tmp.endsWith( "ERROR" ) ){

					m_ussd.Status = gsm::USSDMessage::Unknown ;
					m_function( m_ussd ) ;

					return ;
				}
			}
		}

		m_ussd.Text = "+CUSD: " ;

		while( true ){

			e = m_read.read( 1 ) ;

			if( e.size() < 1 ){

				return _timeout() ;
			}else{
				m_ussd.Text += e ;

				if( m_ussd.Text.endsWith( '\r' ) ){

					break ;
				}
			}
		}

		if( m_log ){			

			qDebug() << tmp + QByteArray( m_ussd.Text.data() + 7 ) ;
		}

		switch( m_ussd.Text.at( 7 ) ){

			case '0' : m_ussd.Status = gsm::USSDMessage::NoActionNeeded ; break ;
			case '1' : m_ussd.Status = gsm::USSDMessage::ActionNeeded   ; break ;
			case '2' : m_ussd.Status = gsm::USSDMessage::Terminated     ; break ;
			case '3' : m_ussd.Status = gsm::USSDMessage::AnotherClient  ; break ;
			case '4' : m_ussd.Status = gsm::USSDMessage::NotSupported   ; break ;
			case '5' : m_ussd.Status = gsm::USSDMessage::Timeout        ; break ;
			default  : m_ussd.Status = gsm::USSDMessage::Unknown        ;
		}

		m_ussd.Text.remove( 0,10 ) ;

		int index = m_ussd.Text.lastIndexOf( '\"' ) ;

		if( index != -1 ){

			m_ussd.Text.truncate( index ) ;
		}

		this->decodeString( m_ussd.Text ) ;

		m_function( m_ussd ) ;
	} ) ;
}

void internal::decodeString( QByteArray& s )
{
	if( QGsmCodec::stringHex( s ) ){

		s = QGsmCodec::fromUnicodeStringInHexToUnicode( s.constData() ).toLatin1() ;
	}
}

Task::future< bool >& internal::dial( const QByteArray& code )
{
	this->readDevice() ;

	return Task::run< bool >( [ this,code ](){

		if( m_write.write( "AT+CUSD=1,\"" + code + "\",15\r" ) > 0 ){

			return true ;
		}else{
			m_lastError = QObject::tr( "Failed to write to device when sending ussd code." ).toLatin1() ;
			return false ;
		}
	} ) ;
}

bool internal::listenForEvents( bool e )
{
	Q_UNUSED( e ) ;
	return true ;
}

const char * internal::lastError()
{
	return m_lastError.constData() ;
}

void internal::setlocale( const char * e )
{
	Q_UNUSED( e ) ;
}

bool internal::init( bool log )
{
	m_log = log ;
	return true ;
}

bool internal::cancelCurrentOperation()
{
	if( this->disconnect() ){

		return this->connect().await() ;
	}else{
		return false ;
	}
}

Task::future< bool >& internal::connect()
{
	return Task::run< bool >( [ this ](){

		sleep( 2 ) ;

		m_read.open( QIODevice::ReadOnly | QIODevice::Unbuffered ) ;

		if( m_read.isOpen() ){

			m_write.open( QIODevice::WriteOnly | QIODevice::Unbuffered ) ;

			if( m_write.isOpen() ){

				//m_write.write( "AT^SYSCFGEX="030201",3FFFFFFF,1,2,800C5,," ) ;
				//m_write.write( "AT+CMGF=1;^CURC=0;^USSDMODE=0" ) ;

				this->setDeviceToDefaultState() ;

				return true ;
			}else{
				m_lastError = QObject::tr( "Failed to open writing channel.Device is in use or does not exist." ).toLatin1() ;
				return false ;
			}
		}else{
			m_lastError = QObject::tr( "Failed to open reading channel.Device is in use or does not exist." ).toLatin1() ;
			return false ;
		}

		return m_read.isOpen() && m_write.isOpen() ;
	} ) ;
}

Task::future< QVector< gsm::SMSText > >& internal::getSMSMessages()
{
	return Task::run< QVector< gsm::SMSText > >( [ this ](){

		QVector< gsm::SMSText > r ;

		std::unique_ptr< gsm,std::function< void( gsm * ) > > e( gsm::instance( { "libgammu" } ),[ this ]( gsm * e ){

			delete e ;

			this->setDeviceToDefaultState() ;
		} ) ;

		if( e->init( false ) && e->connect().get() ){

			r = e->getSMSMessages().get() ;
		}else{
			m_lastError = e->lastError() ;
		}

		return r ;
	} ) ;
}

QString internal::source()
{
	return "internal" ;
}

void internal::setDeviceToDefaultState()
{
	m_write.write( "ATZ\r" ) ;
}

bool internal::canCheckSms()
{
	return true ;
}
