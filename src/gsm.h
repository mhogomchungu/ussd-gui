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


#ifndef GSM_H
#define GSM_H

#include <gammu.h>

#include "task.h"

#include <QByteArray>

#include <memory>

struct gsm_USSDMessage{

	enum { NoActionNeeded,ActionNeeded,Terminated,AnotherClient,NotSupported,Timeout,Unknown } Status ;
	unsigned char Text[ sizeof( GSM_USSDMessage::Text ) ] ;
} ;

class gsm
{
public:
	class pimpl ;
	explicit gsm( std::function< void( const gsm_USSDMessage& ussd ) > ) ;
	~gsm() ;
	bool init() ;
	Task::future<bool>& connect() ;
	bool connected() ;
	bool dial( const QByteArray& ) ;
	bool hasData( bool waitForData = false ) ;
	void setlocale( const char * = nullptr ) ;
	void listenForEvents( bool = true ) ;
	const char * lastError() ;
private:
	std::unique_ptr< gsm::pimpl > m_pimpl ;
	GSM_StateMachine * m_gsm = nullptr ;

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
	} m_status ;
};

#endif // GSM_H
