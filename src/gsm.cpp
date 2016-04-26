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

#include "internal.h"
#include "libgammu.h"

const char * gsm::decodeUnicodeString( const QByteArray& e )
{
	/*
	 * DecodeUnicodeString() is provided by libgammu
	 */
	return DecodeUnicodeString( reinterpret_cast< const unsigned char * >( e.constData() ) ) ;
}

gsm * gsm::instance( const QStringList& backend,std::function< void( const gsm::USSDMessage& ) >&& function )
{
	const auto& e = backend.first() ;

	if( e == "libgammu" ){

		return new libgammu( GSM_AllocStateMachine(),std::move( function ) ) ;

	}else if( e == "internal" ){

		return new internal( backend.at( 1 ),std::move( function ) ) ;
	}else{
		return new libgammu( GSM_AllocStateMachine(),std::move( function ) ) ;
	}
}

gsm::~gsm()
{
}
