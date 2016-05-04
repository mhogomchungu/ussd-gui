/*
 *
 *  Copyright ( c ) 2011-2015
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  ( at your option ) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MANAGEDEVICENAMES_H
#define MANAGEDEVICENAMES_H

#include <QDialog>
#include <QString>
#include <QStringList>

class QAction ;
class QWidget ;
class QTableWidgetItem ;
class QCloseEvent ;
class QSettings ;
class QMenu ;

namespace Ui {
class favorites ;
}

class favorites : public QDialog
{
	Q_OBJECT
public:
	static QStringList readFavorites( QSettings& ) ;

	static favorites& instance( QWidget * parent,QSettings& e )
	{
		return *( new favorites( parent,e ) ) ;
	}
	explicit favorites( QWidget * parent,QSettings& e ) ;
	~favorites() ;
private slots:
	void keyDelete( void ) ;
	void pbMoveUp( void ) ;
	void pbMoveDown( void ) ;
	void pbEdit( void ) ;
	void pbDelete( bool = true ) ;
	void pbAdd( void ) ;
	void pbCancel( void ) ;
	void currentItemChanged( QTableWidgetItem * current,QTableWidgetItem * previous ) ;
	void itemClicked( QTableWidgetItem * current,bool ) ;
	void itemClicked( QTableWidgetItem * current ) ;
	void shortcutPressed( void ) ;
private:
	void updateFavoriteList( void ) ;
	void HideUI( void ) ;
	void ShowUI( void ) ;
	void addToFavorite( const QString& ussd,const QString& comment,QStringList&  ) ;
	QStringList readFavorites( void ) ;
	void removeFavoriteEntry( const QString& ) ;
	void closeEvent( QCloseEvent * ) ;
	bool eventFilter( QObject * watched,QEvent * event ) ;
	void addEntries( const QStringList& ) ;

	bool m_edit = false ;
	Ui::favorites * m_ui ;
	QSettings& m_settings ;
};

#endif // MANAGEDEVICENAMES_H
