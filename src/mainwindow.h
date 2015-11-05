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
#include <QCloseEvent>
#include <QSettings>
#include <functional>
#include <memory>
#include <QStringList>
#include <QString>
#include <QMenu>

#include "gsm.h"

namespace Ui {
class MainWindow ;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow( bool log ) ;
	~MainWindow() ;
private slots:
	void pbSMS() ;
	void pbConnect() ;
	void pbSend() ;
	void pbQuit() ;
	void pbConvert() ;
	void disableSending() ;
	void enableSending() ;
	void connectStatus() ;
	void setHistoryItem( QAction * ) ;
private:
	QStringList historyList() ;
	QString getSetting( const QString& ) ;

	bool gsm7Encoded() ;
	bool Connect() ;
	bool getBoolSetting( const QString& ) ;

	void send() ;
	void displayResult() ;
	void updateHistory( const QByteArray& ) ;
	void processResponce( const gsm::USSDMessage& ) ;
	void closeEvent( QCloseEvent * ) ;
	void setLocalLanguage() ;
	void setHistoryMenu( const QStringList& ) ;
	void setHistoryMenu() ;
	void setSetting( const QString&,const QString& ) ;
	void setSetting( const QString&,bool ) ;
	int  timeOutInterval() ;

	bool m_waiting ;
	int m_timeout ;

	QString m_connectingMsg ;
	QString m_history ;

	Ui::MainWindow * m_ui ;

	gsm::USSDMessage m_ussd ;
	gsm m_gsm ;
	
	QMenu m_menu ;
	QSettings m_settings ;
};

#endif // MAINWINDOW_H
