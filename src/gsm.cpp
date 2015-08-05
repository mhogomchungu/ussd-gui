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

#include <qdebug.h>

#include <cstring>

class gsm::pimpl{
public:
	explicit pimpl( std::function< void( const gsm_USSDMessage& ) > f ) : m_function( std::move( f ) )
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

		std::memcpy( m_ussd.Text,ussd->Text,sizeof( m_ussd.Text ) ) ;
		m_function( m_ussd ) ;
	}
private:
	std::function< void( const gsm_USSDMessage& ussd ) > m_function ;
	gsm_USSDMessage m_ussd ;
} ;

static void _callback( GSM_StateMachine * gsm,GSM_USSDMessage * ussd,void * e )
{
	Q_UNUSED( gsm ) ;

	auto f = reinterpret_cast< gsm::pimpl * >( e ) ;

	( *f )( ussd ) ;
}

gsm::gsm( std::function< void( const gsm_USSDMessage& ussd ) > f ) :
	m_pimpl( new gsm::pimpl( std::move( f ) ) ),m_gsm( GSM_AllocStateMachine() )
{
}

bool gsm::init()
{
	INI_Section * cfg = nullptr ;

	m_status = GSM_FindGammuRC( &cfg,nullptr ) ;

	if( m_status ){

		m_status = GSM_ReadConfig( cfg,GSM_GetConfig( m_gsm,0 ),0 ) ;

		if( m_status ){

			INI_Free( cfg ) ;

			GSM_SetConfigNum( m_gsm,1 ) ;
		}
	}

	return m_status ;
}

gsm::~gsm()
{
	GSM_TerminateConnection( m_gsm ) ;
	GSM_FreeStateMachine( m_gsm ) ;
}

Task::future<bool>& gsm::connect()
{
	return Task::run< bool >( [ this ](){

		m_status = GSM_InitConnection( m_gsm,1 ) ;

		if( m_status ){

			auto _cast = []( std::unique_ptr< gsm::pimpl >& pimpl ){

				return reinterpret_cast< void * >( pimpl.get() ) ;
			} ;

			GSM_SetIncomingUSSDCallback( m_gsm,_callback,_cast( m_pimpl ) ) ;

			this->listenForEvents( true ) ;
		}

		return m_status ;
	} ) ;
}

bool gsm::connected()
{
	return GSM_IsConnected( m_gsm ) ;
}

bool gsm::hasData( bool waitForData )
{
	return GSM_ReadDevice( m_gsm,waitForData ) != 0 ;
}

void gsm::setlocale( const char * e )
{
	GSM_InitLocales( e ) ;
}

const char * gsm::lastError()
{
	return  GSM_ErrorString( m_status.error() ) ;
}

bool gsm::dial( const QByteArray& code )
{
	m_status = GSM_DialService( m_gsm,const_cast< char * >( code.data() ) ) ;
	return m_status ;
}

void gsm::listenForEvents( bool e )
{
	m_status = GSM_SetIncomingUSSD( m_gsm,e ) ;
}
