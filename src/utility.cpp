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

		auto k = QObject::tr( "Number: %1\nDate: %2\nLocation: %3\n\n%4" ) ;

		auto& n = *( d + p ) ;

		e += l + k.arg( n.phoneNumber,_d( n.date ),_l( n.inSIMcard,n.inInbox ),n.message ) ;
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

void utility::setWindowDimensions( QSettings& m,const QString& setting,const std::initializer_list<int>& e )
{
	QString s ;

	for( const auto& it : e ){

		s += QString::number( it ) + " " ;
	}

	m.setValue( setting,s ) ;
}

utility::array_t utility::getWindowDimensions( const QSettings& m,const QString& setting )
{
	if( setting == "favorites" ){

		if( m.contains( setting ) ){

			auto l = utility::split( m.value( setting ).toString(),' ' ) ;

			if( l.size() == 6 ){

				auto _opt = [ & ]( int e ){

					return l.at( e ).toInt() ;
				} ;

				return { { _opt( 0 ),_opt( 1 ),_opt( 2 ),_opt( 3 ),_opt( 4 ),_opt( 5 ) } } ;
			}else{
				return { { 362,195,641,357,147,445 } } ;
			}
		}else{
			return { { 362,195,641,357,147,445 } } ;
		}

	}else if( setting == "main" ){

		if( m.contains( setting ) ){

			auto l = utility::split( m.value( setting ).toString(),' ' ) ;

			if( l.size() == 4 ){

				auto _opt = [ & ]( int e ){

					return l.at( e ).toInt() ;
				} ;

				return { { _opt( 0 ),_opt( 1 ),_opt( 2 ),_opt( 3 ) } } ;
			}else{
				return { { 362,195,628,387 } } ;
			}
		}else{
			return { { 362,195,628,387 } } ;
		}
	}else{
		return { { 362,195,641,357,147,445 } } ;
	}
}

void utility::wait( int interval )
{
	QTimer e ;

	QEventLoop s ;

	QObject::connect( &e,SIGNAL( timeout() ),&s,SLOT( quit() ) ) ;

	e.start( 1000 * interval ) ;

	s.exec() ;
}
