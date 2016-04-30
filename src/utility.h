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

#ifndef MISCFUNCTIONS_H
#define MISCFUNCTIONS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QEvent>
#include <QMenu>
#include <QKeyEvent>
#include <QFile>
#include <functional>
#include <QDir>
#include <QSettings>
#include <QSettings>

#include "gsm.h"

namespace utility
{
	bool eventFilter( QObject * gui,QObject * watched,QEvent * event,std::function< void() > ) ;
	QStringList split( const QString&,char token = '\n' ) ;
	QStringList split( const QString&,const char * ) ;
	QString arrangeSMSInAscendingOrder( QVector< gsm::SMSText >& ) ;
	QVector< gsm::SMSText >& condenseSMS( QVector< gsm::SMSText >& ) ;
	void setWindowDimensions( QSettings&,const QString&,const std::initializer_list<int>& ) ;

	using array_t = std::array< int,7 > ;

	array_t getWindowDimensions( const QSettings&,const QString& ) ;
}

#endif
