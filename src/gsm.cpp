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

#include "gsm.h"

#include <QDebug>
#include <gammu.h>

static void _callback( GSM_StateMachine *,GSM_USSDMessage *,void * ) ;

class gsm::pimpl
{
public:
	pimpl( GSM_StateMachine * g,std::function< void( const gsm::USSDMessage& ) > f ) :
		m_gsm( g ),m_function( std::move( f ) )
	{
	}
	~pimpl()
	{
		GSM_TerminateConnection( m_gsm ) ;
		GSM_FreeStateMachine( m_gsm ) ;
	}
	bool disconnect()
	{
		return m_status = GSM_TerminateConnection( m_gsm ) ;
	}
	bool connected()
	{
		return GSM_IsConnected( m_gsm ) ;
	}
	bool hasData( bool waitForData )
	{
		return GSM_ReadDevice( m_gsm,waitForData ) != 0 ;
	}
	bool dial( const QByteArray& code )
	{
		return m_status = GSM_DialService( m_gsm,const_cast< char * >( code.data() ) ) ;
	}
	bool listenForEvents( bool e )
	{
		return m_status = GSM_SetIncomingUSSD( m_gsm,e ) ;
	}
	const char * lastError()
	{
		return GSM_ErrorString( m_status.error() ) ;
	}
	void setlocale( const char * e )
	{
		GSM_InitLocales( e ) ;
	}
	void operator()( GSM_USSDMessage * ussd )
	{
		switch( ussd->Status ){

			case USSD_NoActionNeeded : m_ussd.Status = gsm::USSDMessage::NoActionNeeded ; break ;
			case USSD_ActionNeeded   : m_ussd.Status = gsm::USSDMessage::ActionNeeded   ; break ;
			case USSD_AnotherClient  : m_ussd.Status = gsm::USSDMessage::AnotherClient  ; break ;
			case USSD_NotSupported   : m_ussd.Status = gsm::USSDMessage::NotSupported   ; break ;
			case USSD_Timeout        : m_ussd.Status = gsm::USSDMessage::Timeout        ; break ;
			case USSD_Terminated     : m_ussd.Status = gsm::USSDMessage::Terminated     ; break ;
			case USSD_Unknown        : m_ussd.Status = gsm::USSDMessage::Unknown        ; break ;
			default                  : m_ussd.Status = gsm::USSDMessage::Unknown        ; break ;
		}

		m_ussd.Text = QByteArray( ( const char * )ussd->Text,sizeof( ussd->Text ) ) ;
		m_function( m_ussd ) ;
	}
	bool init()
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
			}
		}

		return m_status ;
	}
	bool connect()
	{
		m_status = GSM_InitConnection( m_gsm,1 ) ;

		if( m_status ){

			auto mimi = reinterpret_cast< void * >( this ) ;

			GSM_SetIncomingUSSDCallback( m_gsm,_callback,mimi ) ;

			this->listenForEvents( true ) ;
		}

		return m_status ;
	}
private:
	class gsm_error
	{
	public:
		gsm_error& operator =( GSM_Error err )
		{
			m_error = err ;
			return *this ;
		}
		operator bool()
		{
			return m_error == ERR_NONE ;
		}
		GSM_Error error()
		{
			return m_error ;
		}
	private:
		GSM_Error m_error = ERR_UNKNOWN ;
	} m_status ;

	GSM_StateMachine * m_gsm ;
	gsm::USSDMessage m_ussd ;
	std::function< void( const gsm::USSDMessage& ussd ) > m_function ;
} ;

static void _callback( GSM_StateMachine * gsm,GSM_USSDMessage * ussd,void * e )
{
	Q_UNUSED( gsm ) ;

	auto call = reinterpret_cast< gsm::pimpl * >( e ) ;

	( *call )( ussd ) ;
}

const char * gsm::decodeUnicodeString( const QByteArray& e )
{
	/*
	 * DecodeUnicodeString() is provided by libgammu
	 */
	return DecodeUnicodeString( ( const unsigned char * )e.constData() ) ;
}

gsm::gsm( std::function< void( const gsm::USSDMessage& ussd ) > f ) :
	m_pimpl( new gsm::pimpl( GSM_AllocStateMachine(),std::move( f ) ) )
{
}

bool gsm::init()
{
	return m_pimpl->init() ;
}

gsm::~gsm()
{
}

bool gsm::connect()
{
	return m_pimpl->connect() ;
}

bool gsm::connected()
{
	return m_pimpl->connected() ;
}

bool gsm::disconnect()
{
	return m_pimpl->disconnect() ;
}

bool gsm::hasData( bool waitForData )
{
	return m_pimpl->hasData( waitForData ) ;
}

void gsm::setlocale( const char * e )
{
	m_pimpl->setlocale( e ) ;
}

const char * gsm::lastError()
{
	return m_pimpl->lastError() ;
}

bool gsm::dial( const QByteArray& code )
{
	return m_pimpl->dial( code ) ;
}

bool gsm::listenForEvents( bool e )
{
	return m_pimpl->listenForEvents( e ) ;
}
