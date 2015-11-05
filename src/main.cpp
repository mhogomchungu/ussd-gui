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

#include "mainwindow.h"
#include <QApplication>
#include <QStringList>

#include "version.h"

#include <iostream>

static bool version_info( const QStringList& l )
{
	return l.contains( "-h" )        ||
	       l.contains( "-help" )     ||
	       l.contains( "--help" )    ||
	       l.contains( "-v" )        ||
	       l.contains(  "-version" ) ||
	       l.contains( "--version" ) ;
}

static const auto e = "\n\
copyright: 2015 Francis Banyikwa,mhogomchungu@gmail.com\n\
license  : GPLv2+\n\
version  : " VERSION "\n" ;

int main( int argc,char * argv[] )
{
	QApplication a( argc,argv ) ;

	auto l = QCoreApplication::arguments() ;

	if( version_info( l ) ){

		std::cout << e << std::endl ;

		return 0 ;
	}else{
		MainWindow w( l.contains( "-d" ) ) ;

		w.show() ;

		return a.exec() ;
	}
}
