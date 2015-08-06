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

#include <cstring>

#include <gammu.h>

class gsm::pimpl{
public:	
	class gsm_error{
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
	} status ;

	GSM_StateMachine * gsm ;

	pimpl( GSM_StateMachine * g,std::function< void( const gsm_USSDMessage& ) > f ) :
		gsm( g ),m_function( std::move( f ) )
	{
	}
	void operator()( GSM_USSDMessage * ussd )
	{
		switch( ussd->Status ){

			case USSD_NoActionNeeded : m_ussd.Status = gsm_USSDMessage::NoActionNeeded ; break ;
			case USSD_ActionNeeded   : m_ussd.Status = gsm_USSDMessage::ActionNeeded   ; break ;
			case USSD_AnotherClient  : m_ussd.Status = gsm_USSDMessage::AnotherClient  ; break ;
			case USSD_NotSupported   : m_ussd.Status = gsm_USSDMessage::NotSupported   ; break ;
			case USSD_Timeout        : m_ussd.Status = gsm_USSDMessage::Timeout        ; break ;
			case USSD_Terminated     : m_ussd.Status = gsm_USSDMessage::Terminated     ; break ;
			case USSD_Unknown        : m_ussd.Status = gsm_USSDMessage::Unknown        ; break ;
			default                  : m_ussd.Status = gsm_USSDMessage::Unknown        ; break ;
		}

		m_ussd.Text = QByteArray( ( const char * )ussd->Text,sizeof( ussd->Text ) ) ;
		m_function( m_ussd ) ;
	}
private:
	std::function< void( const gsm_USSDMessage& ussd ) > m_function ;
	gsm_USSDMessage m_ussd ;
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

gsm::gsm( std::function< void( const gsm_USSDMessage& ussd ) > f ) :
	m_pimpl( new gsm::pimpl( GSM_AllocStateMachine(),std::move( f ) ) )
{
}

bool gsm::init()
{
	struct INI_config
	{
		~INI_config(){ INI_Free( cfg ) ; }
		INI_Section * cfg = nullptr ;
	} config ;

	m_pimpl->status = GSM_FindGammuRC( &config.cfg,nullptr ) ;

	if( m_pimpl->status ){

		m_pimpl->status = GSM_ReadConfig( config.cfg,GSM_GetConfig( m_pimpl->gsm,0 ),0 ) ;

		if( m_pimpl->status ){

			GSM_SetConfigNum( m_pimpl->gsm,1 ) ;
		}		
	}

	return m_pimpl->status ;
}

gsm::~gsm()
{
	GSM_TerminateConnection( m_pimpl->gsm ) ;
	GSM_FreeStateMachine( m_pimpl->gsm ) ;
}

Task::future<bool>& gsm::connect()
{
	return Task::run< bool >( [ this ](){

		m_pimpl->status = GSM_InitConnection( m_pimpl->gsm,1 ) ;

		if( m_pimpl->status ){

			auto _cast = []( std::unique_ptr< gsm::pimpl >& pimpl ){

				return reinterpret_cast< void * >( pimpl.get() ) ;
			} ;

			GSM_SetIncomingUSSDCallback( m_pimpl->gsm,_callback,_cast( m_pimpl ) ) ;

			this->listenForEvents( true ) ;
		}

		return m_pimpl->status ;
	} ) ;
}

bool gsm::connected()
{
	return GSM_IsConnected( m_pimpl->gsm ) ;
}

bool gsm::hasData( bool waitForData )
{
	return GSM_ReadDevice( m_pimpl->gsm,waitForData ) != 0 ;
}

void gsm::setlocale( const char * e )
{
	GSM_InitLocales( e ) ;
}

const char * gsm::lastError()
{
	return  GSM_ErrorString( m_pimpl->status.error() ) ;
}

bool gsm::dial( const QByteArray& code )
{
	m_pimpl->status = GSM_DialService( m_pimpl->gsm,const_cast< char * >( code.data() ) ) ;
	return m_pimpl->status ;
}

void gsm::listenForEvents( bool e )
{
	m_pimpl->status = GSM_SetIncomingUSSD( m_pimpl->gsm,e ) ;
}
