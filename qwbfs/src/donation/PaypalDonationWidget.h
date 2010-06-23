/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PaypalDonationWidget.h
** Date      : 2010-20-16T16:19:29
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
** In addition, as a special exception, the copyright holders give permission
** to link this program with the OpenSSL project's "OpenSSL" library (or with
** modified versions of it that use the same license as the "OpenSSL"
** library), and distribute the linked executables. You must obey the GNU
** General Public License in all respects for all of the code used other than
** "OpenSSL".  If you modify file(s), you may extend this exception to your
** version of the file(s), but you are not obligated to do so. If you do not
** wish to do so, delete this exception statement from your version.
**
****************************************************************************/
#ifndef PAYPALDONATIONWIDGET_H
#define PAYPALDONATIONWIDGET_H

#include <QLabel>
#include <QUrl>
#include <QHash>

class DataNetworkCache;

class PaypalDonationWidget : public QLabel
{
	Q_OBJECT
	
public:
	PaypalDonationWidget( QWidget* parent = 0 );
	virtual ~PaypalDonationWidget();
	
	virtual bool event( QEvent* event );
	
	QString actionPost() const;
	QString businessId() const;
	QString itemName() const;
	QString itemId() const;
	QString currencyCode() const;
	
	QPixmap pixmap( const QUrl& url ) const;
	DataNetworkCache* cache() const;
	QUrl url() const;
	
	static QUrl pixmapUrl( const QString& locale );

public slots:
	void setActionPost( const QString& value );
	void setBusinessId( const QString& value );
	void setItemName( const QString& value );
	void setItemId( const QString& value );
	void setCurrencyCode( const QString& value );

protected:
	QHash<QString, QString> mQueryItems;
	
	virtual void mousePressEvent( QMouseEvent* event );
	
	void localeChanged();
	void updatePixmap();

protected slots:
	void networkCache_dataCached( const QUrl& url );
	void networkCache_error( const QString& message, const QUrl& url = QUrl() );
	void networkCache_invalidated();
};

#endif // PAYPALDONATIONWIDGET_H