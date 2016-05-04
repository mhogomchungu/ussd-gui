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

#include "libgammu.h"
#include <QDebug>

static void _callback( GSM_StateMachine *,GSM_USSDMessage *,void * ) ;

static QByteArray _QByteArray( decltype(( GSM_USSDMessage::Text )) buffer )
{
	return QByteArray( reinterpret_cast< const char * >( buffer ),int( sizeof( buffer ) ) ) ;
}

libgammu::libgammu( std::function< void( const gsm::USSDMessage& ) >&& function ) :
		m_gsm( GSM_AllocStateMachine() ),m_function( std::move( function ) )
{
}

libgammu::~libgammu()
{
	GSM_TerminateConnection( m_gsm ) ;
	GSM_FreeStateMachine( m_gsm ) ;
}

Task::future< bool >& libgammu::disconnect()
{
	return Task::run< bool >( [ this ]{

		m_status = GSM_TerminateConnection( m_gsm ) ;

		if( m_status ){

			m_function( { "",gsm::USSDMessage::Timeout } ) ;
		}

		return m_status ;
	} ) ;
}

bool libgammu::connected()
{
	return GSM_IsConnected( m_gsm ) ;
}

bool libgammu::canRead( bool waitForData )
{
	return GSM_ReadDevice( m_gsm,waitForData ) != 0 ;
}

Task::future< bool >& libgammu::hasData( bool waitForData )
{
	return Task::run< bool >( [ this,waitForData ]{

		return this->canRead( waitForData ) ;
	} ) ;
}

Task::future< bool >& libgammu::dial( const QByteArray& code )
{
	return Task::run< bool >( [ this,code ]{

		this->listenForEvents( true ) ;
		return m_status = GSM_DialService( m_gsm,const_cast< char * >( code.data() ) ) ;
	} ) ;
}

bool libgammu::listenForEvents( bool e )
{
	return m_status = GSM_SetIncomingUSSD( m_gsm,e ) ;
}

const char * libgammu::lastError()
{
	return m_status.errorString() ;
}

void libgammu::setlocale( const char * e )
{
	GSM_InitLocales( e ) ;
}

void libgammu::operator()( GSM_USSDMessage * ussd )
{
	using _gsm = gsm::USSDMessage ;

	switch( ussd->Status ){

		case USSD_NoActionNeeded : m_ussd.Status = _gsm::NoActionNeeded ; break ;
		case USSD_ActionNeeded   : m_ussd.Status = _gsm::ActionNeeded   ; break ;
		case USSD_AnotherClient  : m_ussd.Status = _gsm::AnotherClient  ; break ;
		case USSD_NotSupported   : m_ussd.Status = _gsm::NotSupported   ; break ;
		case USSD_Timeout        : m_ussd.Status = _gsm::Timeout        ; break ;
		case USSD_Terminated     : m_ussd.Status = _gsm::Terminated     ; break ;
		case USSD_Unknown        : m_ussd.Status = _gsm::Unknown        ; break ;
		default                  : m_ussd.Status = _gsm::Unknown        ; break ;
	}

	m_ussd.Text = _QByteArray( ussd->Text ) ;

	m_function( m_ussd ) ;
}

bool libgammu::init( bool log )
{
	struct INI_config
	{
		~INI_config(){ INI_Free( cfg ) ; }
		INI_Section * cfg = nullptr ;
	} config ;

	m_status = GSM_FindGammuRC( &config.cfg,nullptr ) ;

	if( m_status ){

		m_status = GSM_ReadConfig( config.cfg,GSM_GetConfig( m_gsm,0 ),0 ) ;

		if( m_status ){

			GSM_SetConfigNum( m_gsm,1 ) ;

			if( log ){

				auto e = GSM_GetGlobalDebug() ;
				GSM_SetDebugFileDescriptor( stderr,true,e ) ;
				GSM_SetDebugLevel( "textall",e ) ;
			}
		}
	}

	return m_status ;
}

bool libgammu::cancelCurrentOperation()
{
	this->listenForEvents( false ) ;
	return m_status = GSM_AbortOperation( m_gsm ) ;
}

Task::future< bool >& libgammu::connect()
{
	return Task::run< bool >( [ this ]{

		m_status = GSM_InitConnection( m_gsm,1 ) ;

		if( m_status ){

			auto mimi = reinterpret_cast< void * >( this ) ;

			GSM_SetIncomingUSSDCallback( m_gsm,_callback,mimi ) ;

			this->listenForEvents( true ) ;
		}

		return m_status ;
	} ) ;
}

static const char * _message( const unsigned char * e,GSM_Coding_Type c ){

	/*
	 * No idea what to do with these options and ignoring them seems
	 * to work just fine with my modem.
	 */

	/*
	 * DecodeUnicodeString() is provided by libgammu
	 */
	switch( c ){

	case SMS_Coding_Unicode_No_Compression:

		return DecodeUnicodeString( e ) ;

	case SMS_Coding_Unicode_Compression:

		return DecodeUnicodeString( e ) ;

	case SMS_Coding_Default_No_Compression:

		return DecodeUnicodeString( e ) ;

	case SMS_Coding_Default_Compression:

		return DecodeUnicodeString( e ) ;

	case SMS_Coding_8bit:

		return DecodeUnicodeString( e ) ;
	default:
		return DecodeUnicodeString( e ) ;
	}
}

QVector< gsm::SMSText > libgammu::_getSMSMessages( bool deleteSMS )
{
	auto _sms = [ & ]( QVector< gsm::SMSText >& messages,GSM_MultiSMSMessage * m ){

		auto _getSMS = [ & ]( GSM_SMSMessage * m ){

			gsm::SMSText sms ;

			auto d = &m->DateTime ;

			auto _d = []( decltype( d->Day ) n )->QString{

				if( n < 10 ){

					return "0" + QString::number( n ) ;
				}else{
					return QString::number( n ) ;
				}
			} ;

			auto a          = QString( "%1-%2-%3" ).arg( _d( d->Month ),_d( d->Day ),_d( d->Year ) ) ;
			auto b          = QString( "%1:%2:%3" ).arg( _d( d->Hour ),_d( d->Minute ),_d( d->Second ) ) ;

			sms.date        = a + " " + b ;

			sms.inSIMcard   = m->Memory == MEM_SM ;

			sms.inInbox     = m->PDU == SMS_Deliver ;

			sms.phoneNumber = DecodeUnicodeString( m->Number ) ;

			sms.message     = _message( m->Text,m->Coding ) ;

			return sms ;
		} ;

		for( decltype( m->Number ) i = 0 ; i < m->Number ; i++ ){

			auto sms = &m->SMS[ i ] ;

			messages.append( _getSMS( sms ) ) ;

			if( deleteSMS ){

				;//GSM_DeleteSMS( m_gsm,sms ) ;
			}else{
				;//GSM_GetSMS( m_gsm,m ) ;
			}
		}
	} ;

	GSM_MultiSMSMessage sms ;

	QVector< gsm::SMSText > messages ;

	m_status = GSM_GetNextSMS( m_gsm,&sms,true ) ;

	if( m_status ){

		_sms( messages,&sms ) ;

		while( true ){

			m_status = GSM_GetNextSMS( m_gsm,&sms,false ) ;

			if( m_status.errEmpty() ){

				m_status = ERR_NONE ;

				break ;

			}else if( m_status ){

				_sms( messages,&sms ) ;
			}else{
				break ;
			}
		}

	}else if( m_status.errEmpty() ){

		m_status = ERR_NONE ;
	}

	return messages ;
}

Task::future< QVector< gsm::SMSText > >& libgammu::getSMSMessages( bool deleteSMS )
{
	return Task::run< QVector< gsm::SMSText > >( [ = ]{

		return this->_getSMSMessages( deleteSMS ) ;
	} ) ;
}

QString libgammu::source()
{
	return "libgammu" ;
}

bool libgammu::canCheckSms()
{
	return true ;
}

static void _callback( GSM_StateMachine * gsm,GSM_USSDMessage * ussd,void * e )
{
	Q_UNUSED( gsm ) ;

	auto function = reinterpret_cast< libgammu * >( e ) ;

	( *function )( ussd ) ;
}
