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
	internal( const QString& device,
		  const QString& terminatorSequence,
		  std::function< void( const gsm::USSDMessage& ) >&& ) ;
	~internal() ;

	bool canCheckSms() ;
	bool disconnect() ;
	bool connected() ;
	bool canRead( bool waitForData ) ;
	bool listenForEvents( bool ) ;
	bool init( bool log ) ;
	bool cancelCurrentOperation() ;

	Task::future< bool >& hasData( bool waitForData ) ;
	Task::future< bool >& dial( const QByteArray& code ) ;
	Task::future< bool >& connect() ;
	Task::future< QVector< gsm::SMSText > >& getSMSMessages() ;

	const char * lastError() ;
	void setlocale( const char * ) ;

	QString source() ;
private:
	void setDeviceToDefaultState() ;
	void readDevice() ;
	gsm::USSDMessage m_ussd ;
	QFile m_read ;
	QFile m_write ;
	QByteArray m_lastError ;
	bool m_log ;
	QString m_terminatorSequence ;
	std::function< void( const gsm::USSDMessage& ussd ) > m_function ;
} ;

