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

#include <QString>
#include <QByteArray>
#include <QVector>

#include <functional>
#include <memory>

class gsm
{
public:
	struct USSDMessage
	{
		QByteArray Text ;

		enum {  NoActionNeeded,
			ActionNeeded,
			Terminated,
			AnotherClient,
			NotSupported,
			Timeout,
			Unknown } Status ;
	} ;

	struct SMSText
	{
		QString phoneNumber ;
		QString date ;
		QString message ;
		bool read ;
		bool inSIMcard ;
		bool inInbox ;
	} ;

	class pimpl ;

	static const char * decodeUnicodeString( const QByteArray& ) ;

	gsm( std::function< void( const gsm::USSDMessage& ussd ) > ) ;
	~gsm() ;

	QVector< gsm::SMSText > getSMSMessages() ;

	bool connect() ;
	bool disconnect() ;
	bool init( bool = false ) ;
	bool connected() ;
	bool dial( const QByteArray& ) ;
	bool hasData( bool waitForData = false ) ;
	bool listenForEvents( bool = true ) ;

	void setlocale( const char * = nullptr ) ;

	const char * lastError() ;
private:
	std::unique_ptr< gsm::pimpl > m_pimpl ;
} ;

#endif // GSM_H
