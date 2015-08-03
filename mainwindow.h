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

#include <QStringList>

#include <QString>

#include <QMenu>

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
	void pbConvert() ;
	void disableSending() ;
	void enableSending() ;
	void connectStatus() ;
	void setHistoryItem( QAction * ) ;
private:
	QStringList historyList() ;
	bool gsm7Encoded() ;
	void displayResult() ;
	void updateHistory( const QByteArray& ) ;
	bool initConnection() ;
	void processResponce( GSM_USSDMessage * ) ;
	void setUpDevice() ;
	void closeEvent( QCloseEvent * ) ;
	bool deviceIsConnected() ;
	void setLocalLanguage() ;
	void setHistoryMenu( const QStringList& ) ;
	void setHistoryMenu() ;
	QString getSetting( const QString& ) ;
	bool getBoolSetting( const QString& ) ;
	void setSetting( const QString&,const QString& ) ;
	void setSetting( const QString&,bool ) ;
	Ui::MainWindow * m_ui ;
	GSM_StateMachine * m_gsm = nullptr ;
	foo m_foo ;
	QSettings m_settings ;
	QString m_connectingMsg ;
	QString m_history ;
	GSM_USSDMessage m_ussd ;
	QMenu m_menu ;
};

#endif // MAINWINDOW_H
