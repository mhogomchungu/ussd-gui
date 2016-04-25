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
#include <QDebug>

internal::internal( std::function< void( const gsm::USSDMessage& ) >&& function )
{
	Q_UNUSED( function ) ;
}

internal::~internal()
{
}

bool internal::disconnect()
{
	return false ;
}

bool internal::connected()
{
	return false ;
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

Task::future< bool >& internal::dial( const QByteArray& code )
{
	Q_UNUSED( code ) ;

	return Task::run< bool >( [](){

		return false ;
	} ) ;
}

bool internal::listenForEvents( bool e )
{
	Q_UNUSED( e ) ;
	return false ;
}

const char * internal::lastError()
{
	return "" ;
}

void internal::setlocale( const char * e )
{
	Q_UNUSED( e ) ;
}

bool internal::init( bool log )
{
	Q_UNUSED( log ) ;
	return false ;
}

Task::future< bool >& internal::connect()
{
	return Task::run< bool >( [](){

		return false ;
	} ) ;
}

Task::future< QVector< gsm::SMSText > >& internal::getSMSMessages()
{
	return Task::run< QVector< gsm::SMSText > >( [](){

		return QVector< gsm::SMSText >() ;
	} ) ;
}

QString internal::source()
{
	return "internal" ;
}
