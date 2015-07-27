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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <gammu.h>

#include <QCloseEvent>

#include <QSettings>

#include <functional>

#include <memory>

namespace Ui {
class MainWindow ;
}

class foo{
public:
	explicit foo( std::function< void( GSM_USSDMessage * ) > f ) : m_function( std::move( f ) )
	{
	}
	void operator()( GSM_USSDMessage * ussd )
	{
		m_function( ussd ) ;
	}
private:
	std::function< void( GSM_USSDMessage * ussd ) > m_function ;
} ;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow( QWidget  * parent = nullptr ) ;
	~MainWindow() ;

private slots:
	void pbSend() ;
	void pbQuit() ;
	void disableSending() ;
	void enableSending() ;
private:
	bool setConnection() ;
	void processResponce( GSM_USSDMessage * ) ;
	void setUpDevice() ;
	void closeEvent( QCloseEvent * ) ;
	bool deviceIsNotConnected() ;
	bool deviceIsConnected() ;
	QString getSetting( const QString& ) ;
	void setSetting( const QString&,const QString& ) ;
	Ui::MainWindow * m_ui ;
	GSM_StateMachine * m_gsm = nullptr ;
	foo m_foo ;
	QSettings m_settings ;
};

#endif // MAINWINDOW_H
