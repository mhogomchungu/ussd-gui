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

#include "gsm.h"
#include <QFile>

class internal : public gsm
{
public:
	internal( const QString& device,std::function< void( const gsm::USSDMessage& ) >&& ) ;
	~internal() ;
	bool disconnect() ;
	bool connected() ;
	bool canRead( bool waitForData ) ;
	Task::future< bool >& hasData( bool waitForData ) ;
	Task::future< bool >& dial( const QByteArray& code ) ;
	bool listenForEvents( bool e ) ;
	const char * lastError() ;
	void setlocale( const char * e ) ;
	bool init( bool log ) ;
	Task::future< bool >& connect() ;
	Task::future< QVector< gsm::SMSText > >& getSMSMessages() ;
	QString source() ;
	bool canCheckSms() ;
private:
	void readDevice() ;
	gsm::USSDMessage m_ussd ;
	std::function< void( const gsm::USSDMessage& ussd ) > m_function ;
	QFile m_read ;
	QFile m_write ;
} ;

