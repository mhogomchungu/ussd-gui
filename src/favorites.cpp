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

#include "favorites.h"
#include "ui_favorites.h"
#include <iostream>

#include <QTableWidgetItem>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <QSettings>
#include <QDebug>

#include "utility.h"
#include "dialogmsg.h"
#include "tablewidget.h"

favorites::favorites( QWidget * parent,QSettings& e ) : QDialog( parent ),m_ui( new Ui::favorites ),m_settings( e )
{
	m_ui->setupUi( this ) ;

	this->setWindowFlags( Qt::Window | Qt::Dialog ) ;
	this->setFont( parent->font() ) ;

	this->setFixedSize( this->size() ) ;

	connect( m_ui->pbAdd,SIGNAL( clicked() ),this,SLOT( add() ) ) ;
	connect( m_ui->pbCancel,SIGNAL( clicked() ),this,SLOT( cancel() ) ) ;
	connect( m_ui->tableWidget,SIGNAL( currentItemChanged( QTableWidgetItem *,QTableWidgetItem * ) ),this,
		SLOT( currentItemChanged( QTableWidgetItem *,QTableWidgetItem * ) ) ) ;
	connect( m_ui->tableWidget,SIGNAL( itemClicked( QTableWidgetItem * ) ),this,
		SLOT( itemClicked( QTableWidgetItem * ) ) ) ;

	m_ac = new QAction( this ) ;
	QList<QKeySequence> keys ;
	keys.append( Qt::Key_Enter ) ;
	keys.append( Qt::Key_Return ) ;
	keys.append( Qt::Key_Menu ) ;
	m_ac->setShortcuts( keys ) ;
	connect( m_ac,SIGNAL( triggered() ),this,SLOT( shortcutPressed() ) ) ;
	this->addAction( m_ac ) ;

	this->installEventFilter( this ) ;

	this->ShowUI() ;
}

bool favorites::eventFilter( QObject * watched,QEvent * event )
{
	return utility::eventFilter( this,watched,event,[ this ](){ this->HideUI() ; } ) ;
}

void favorites::shortcutPressed()
{
	this->itemClicked( m_ui->tableWidget->currentItem(),false ) ;
}

void favorites::ShowUI()
{
	m_ui->tableWidget->setColumnWidth( 0,147 ) ;
	m_ui->tableWidget->setColumnWidth( 1,445 ) ;

	tablewidget::clearTable( m_ui->tableWidget ) ;

	auto _add_entry = [ this ]( const QStringList& l ){

		if( l.size() > 1 ){

			this->addEntries( l ) ;
		}
	} ;

	for( const auto& it : this->readFavorites() ){

		_add_entry( utility::split( it," - " ) ) ;
	}

	m_ui->lineEditUSSD->clear() ;
	m_ui->lineEditUSSDComment->clear() ;
	m_ui->lineEditUSSD->setFocus() ;

	this->show() ;
}

void favorites::HideUI()
{
	this->hide() ;
	this->deleteLater() ;
}

void favorites::addEntries( const QStringList& l )
{
	tablewidget::addRowToTable( m_ui->tableWidget,l ) ;
}

void favorites::itemClicked( QTableWidgetItem * current )
{
	this->itemClicked( current,true ) ;
}

void favorites::itemClicked( QTableWidgetItem * current,bool clicked )
{
	QMenu m ;
	m.setFont( this->font() ) ;
	connect( m.addAction( tr( "Remove Selected Entry" ) ),SIGNAL( triggered() ),this,SLOT( removeEntryFromFavoriteList() ) ) ;

	m.addSeparator() ;
	m.addAction( tr( "Cancel" ) ) ;

	if( clicked ){

		m.exec( QCursor::pos() ) ;
	}else{
		int x = m_ui->tableWidget->columnWidth( 0 ) ;
		int y = m_ui->tableWidget->rowHeight( current->row() ) * current->row() + 20 ;
		m.exec( m_ui->tableWidget->mapToGlobal( QPoint( x,y ) ) ) ;
	}
}

void favorites::removeEntryFromFavoriteList()
{
	auto table = m_ui->tableWidget ;

	table->setEnabled( false ) ;

	auto row = table->currentRow() ;

	auto p = table->item( row,0 )->text() ;
	auto q = table->item( row,1 )->text() ;

	if( !p.isEmpty() && !q.isEmpty() ){

		this->removeFavoriteEntry( QString( "%1 - %2" ).arg( p,q ) ) ;

		tablewidget::deleteRowFromTable( table,row ) ;
	}

	table->setEnabled( true ) ;
}

void favorites::cancel()
{
	this->HideUI() ;
}

void favorites::add()
{
	DialogMsg msg( this ) ;

	auto ussd = m_ui->lineEditUSSD->text() ;
	auto ussd_comment = m_ui->lineEditUSSDComment->text() ;

	if( ussd.isEmpty() ){

		return msg.ShowUIOK( tr( "ERROR!" ),tr( "USSD Code Field Is Empty" ) ) ;
	}
	if( ussd_comment.isEmpty() ){

		return msg.ShowUIOK( tr( "ERROR!" ),tr( "USSD Code Comment Field Is Empty" ) ) ;
	}

	m_ui->tableWidget->setEnabled( false ) ;

	this->addEntries( { ussd,ussd_comment } ) ;

	this->addToFavorite( ussd,ussd_comment ) ;

	m_ui->lineEditUSSD->clear() ; ;
	m_ui->lineEditUSSDComment->clear() ;

	m_ui->tableWidget->setEnabled( true ) ;
}

favorites::~favorites()
{
	delete m_ui ;
}

void favorites::closeEvent( QCloseEvent * e )
{
	e->ignore() ;
	this->HideUI() ;
}

void favorites::currentItemChanged( QTableWidgetItem * current, QTableWidgetItem * previous )
{
	tablewidget::selectTableRow( current,previous ) ;
}

static QString _ussdInfo()
{
	return "ussdInfo" ;
}

static void _update_favorites( QSettings& m,const QStringList& l )
{
	QString s ;

	for( const auto& it : l ){

		s += it + "\n" ;
	}

	m.setValue( _ussdInfo(),s ) ;
}

void favorites::addToFavorite( const QString& ussd,const QString& comment )
{
	auto l = this->readFavorites() ;

	l.append( ussd + " - " + comment ) ;

	_update_favorites( m_settings,l ) ;
}

void favorites::removeFavoriteEntry( const QString& entry )
{
	auto l = this->readFavorites() ;

	l.removeOne( entry ) ;

	_update_favorites( m_settings,l ) ;
}

QStringList favorites::readFavorites()
{
	return favorites::readFavorites( m_settings ) ;
}

QStringList favorites::readFavorites( QSettings& e )
{
	auto opt = _ussdInfo() ;

	if( e.contains( opt ) ){

		return utility::split( e.value( opt ).toString() ) ;
	}else{
		return QStringList() ;
	}
}
