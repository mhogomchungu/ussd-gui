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

#include "utility.h"

bool utility::eventFilter( QObject * gui,QObject * watched,QEvent * event,std::function< void() > function )
{
	if( watched == gui ){

		if( event->type() == QEvent::KeyPress ){

			auto keyEvent = static_cast< QKeyEvent* >( event ) ;

			if( keyEvent->key() == Qt::Key_Escape ){

				function() ;

				return true ;
			}
		}
	}

	return false ;
}

QStringList utility::split( const QString& str,char token )
{
	return str.split( token,QString::SkipEmptyParts ) ;
}

QStringList utility::split( const QString& str,const char * token )
{
	return str.split( token,QString::SkipEmptyParts ) ;
}

QString utility::arrangeSMSInAscendingOrder( QVector< gsm::SMSText >& m )
{
	auto j = m.size() ;

	auto d = m.data() ;

	auto e = QObject::tr( "\nNumber Of Text Messages: %1" ).arg( QString::number( j ) ) ;

	for( decltype( j ) p = 0 ; p < j ; p++ ){

		auto& it = *( d + p ) ;

		for( decltype( j ) q = p + 1 ; q < j ; q++ ){

			auto& xt = *( d + q ) ;

			if( it.date < xt.date ){

				std::swap( it,xt ) ;
			}
		}

		auto _r  = []( bool e ){ return e ? QObject::tr( "Read" ) : QObject::tr( "Not Read" ) ; } ;

		auto _l  = []( bool inSimCard,bool inInbox ){

			if( inSimCard ){

				if( inInbox ){

					return QObject::tr( "SIM's Inbox" ) ;
				}else{
					return QObject::tr( "SIM's Outbox" ) ;
				}
			}else{
				if( inInbox ){

					return QObject::tr( "Phone's Inbox" ) ;
				}else{
					return QObject::tr( "Phone's Outbox" ) ;
				}
			}
		} ;

		auto _d = []( const auto& e ){

			auto f = e.mid( 2 ) ;

			switch( e.mid( 0,2 ).remove( "0" ).toInt() ){

				case 1  : return QObject::tr( "January" )   + f ;
				case 2  : return QObject::tr( "February" )  + f ;
				case 3  : return QObject::tr( "March" )     + f ;
				case 4  : return QObject::tr( "April" )     + f ;
				case 5  : return QObject::tr( "May" )       + f ;
				case 6  : return QObject::tr( "June" )      + f ;
				case 7  : return QObject::tr( "July" )      + f ;
				case 8  : return QObject::tr( "August" )    + f ;
				case 9  : return QObject::tr( "September" ) + f ;
				case 10 : return QObject::tr( "October" )   + f ;
				case 11 : return QObject::tr( "November" )  + f ;
				case 12 : return QObject::tr( "December" )  + f ;
			}

			return e ;
		} ;

		auto l = "\n------------------------------------------------------------------------------------\n" ;

		auto k = QObject::tr( "Number: %1\nDate: %2\nState: %3\nLocation: %4\n\n%5" ) ;

		auto& n = *( d + p ) ;

		e += l + k.arg( n.phoneNumber,_d( n.date ),_r( n.read ),_l( n.inSIMcard,n.inInbox ),n.message ) ;
	}

	return e ;
}

QVector< gsm::SMSText >& utility::condenseSMS( QVector< gsm::SMSText >& m )
{
	auto j = m.size() ;

	auto d = m.data() ;

	for( decltype( j ) i = 0 ; i < j ; i++ ){

		auto& it = *( d + i ) ;

		if( it.inInbox ){

			decltype( i ) k = i + 1 ;

			while( k < j ){

				/*
				 * Sometimes,a single text message may be split into multiple parts and we
				 * seem to get these parts as if they are independent text messages.This routine
				 * is a cheap attempt and combining these multi part text messages into one by
				 * assuming consercutive text messages that share the same time stamp are a part
				 * of the same text message.
				 */

				const auto& xt = *( d + k ) ;

				if( it.date == xt.date ){

					it.message += xt.message ;

					m.remove( k ) ;

					j-- ;
				}else{
					k++ ;
				}
			}
		}
	}

	return m ;
}
