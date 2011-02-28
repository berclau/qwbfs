/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : UIMain.cpp
** Date      : 2010-06-16T14:19:29
** License   : GPL2
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
#include "UIMain.h"
#include "UIAbout.h"
#include "models/DiscModel.h"
#include "models/DiscDelegate.h"
#include "wiitdb/Covers.h"
#include "datacache/DataNetworkCache.h"
#include "ProgressDialog.h"
#include "PropertiesDialog.h"
#include "Properties.h"
#include "main.h"
#include "donation/PaypalDonationWidget.h"
#include "UpdateChecker/pUpdateChecker.h"

#include <pTranslationManager.h>
#include <pTranslationDialog.h>

#include <QFileSystemModel>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QSpacerItem>
#include <QDebug>

UIMain::UIMain( QWidget* parent )
	: QMainWindow( parent )
{
	mCache = new DataNetworkCache( this );
	mUpdateChecker = new pUpdateChecker( this );
	mUpdateChecker->setDownloadsFeedUrl( QUrl( APPLICATION_DOWNLOADS_FEED ) );
	mUpdateChecker->setVersion( APPLICATION_VERSION );
	mUpdateChecker->setVersionString( APPLICATION_VERSION_STR );
	mUpdateChecker->setVersionDiscoveryPattern( ".*qwbfsmanager-([0-9\\.]+).*" );
	
	setWindowTitle( QString( "%1 v%2" ).arg( APPLICATION_NAME ).arg( APPLICATION_VERSION_STR ) );
	setUnifiedTitleAndToolBarOnMac( true );
	setupUi( this );
	
	centralVerticalLayout->setMenuBar( qmtbInfos );
	qmtbInfos->setVisible( false );
	
	dwTools->toggleViewAction()->setIcon( QIcon( ":/icons/256/tools.png" ) );
	dwCovers->toggleViewAction()->setIcon( QIcon( ":/icons/256/covers.png" ) );
	
	mDonationWidget = new PaypalDonationWidget( this );
	mDonationWidget->setBusinessId( "5R924WYXJ6BAW" );
	mDonationWidget->setItemName( "QWBFS Manager" );
	mDonationWidget->setItemId( "QWBFS-DONATION" );
	mDonationWidget->setCurrencyCode( "EUR" );
	
	toolBar->insertAction( aAbout, mUpdateChecker->menuAction() );
	toolBar->addAction( dwTools->toggleViewAction() );
	toolBar->addAction( dwCovers->toggleViewAction() );
	QWidget* spacerWidget = new QWidget( toolBar );
	spacerWidget->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum ) );
	toolBar->addWidget( spacerWidget );
	toolBar->addWidget( mDonationWidget );
	
	mFoldersModel = new QFileSystemModel( this );
	mFoldersModel->setFilter( QDir::Dirs | QDir::NoDotAndDotDot );
	
	mFilesModel = new QFileSystemModel( this );
	mFilesModel->setFilter( QDir::Files );

	tvFolders->setModel( mFoldersModel );
	tvFolders->setColumnHidden( 1, true );
	tvFolders->setColumnHidden( 2, true );
	tvFolders->setColumnHidden( 3, true );
	
	lvFiles->setModel( mFilesModel );
	
	mExportModel = new QWBFS::Model::DiscModel( this );
	
	lvExport->setModel( mExportModel );
	lvExport->setItemDelegate( new QWBFS::Model::DiscDelegate( mExportModel, mCache ) );
	
	mLastDiscId = -1;
	
	pwMainView->setMainView( true );
	pwMainView->showHideImportViewButton()->setChecked( false );
	connectView( pwMainView );
	tbReloadDrives->click();
	aReloadPartitions->trigger();
	
	localeChanged();
	
	connect( mCache, SIGNAL( dataCached( const QUrl& ) ), this, SLOT( dataNetworkCache_dataCached( const QUrl& ) ) );
	connect( mCache, SIGNAL( error( const QString&, const QUrl& ) ), this, SLOT( dataNetworkCache_error( const QString&, const QUrl& ) ) );
	connect( mCache, SIGNAL( invalidated() ), this, SLOT( dataNetworkCache_invalidated() ) );
}

UIMain::~UIMain()
{
	//qWarning() << Q_FUNC_INFO;
}

DataNetworkCache* UIMain::cache() const
{
	return mCache;
}

void UIMain::showEvent( QShowEvent* event )
{
	QMainWindow::showEvent( event );
	
	static bool shown = false;
	
	if ( !shown ) {
		shown = true;
		loadProperties();
		mUpdateChecker->silentCheck();
		qmtbInfos->appendMessage( tr(
				"Welcome to %1, the cross-platform WBFS Manager. Report bugs <a href=\"%2\">here</a>, discuss <a href=\"%3\">here</a>."
			).arg( APPLICATION_NAME ).arg( APPLICATION_REPORT_BUG_URL ).arg( APPLICATION_DISCUSS_URL ) );
	}
}

void UIMain::closeEvent( QCloseEvent* event )
{
	saveProperties();
	
	QMainWindow::closeEvent( event );
}

bool UIMain::event( QEvent* event )
{
	switch ( event->type() ) {
		case QEvent::LocaleChange:
			localeChanged();
			break;
		default:
			break;
	}
	
	return QMainWindow::event( event );
}

void UIMain::localeChanged()
{
	retranslateUi( this );
}

void UIMain::loadProperties()
{
	Properties properties( this );
	
	mCache->setDiskCacheSize( properties.cacheDiskSize() );
	mCache->setMemoryCacheSize( properties.cacheMemorySize() );
	mCache->setWorkingPath( properties.cacheUseTemporaryPath() ? properties.temporaryPath() : properties.cacheWorkingPath() );
	
	QNetworkProxy proxy( properties.proxyType() );
	proxy.setHostName( properties.proxyServer() );
	proxy.setPort( properties.proxyPort() );
	proxy.setUser( properties.proxyLogin() );
	proxy.setPassword( properties.proxyPassword() );
	
	QNetworkProxy::setApplicationProxy( proxy );
	
	mDonationWidget->cache()->setDiskCacheSize( mCache->diskCacheSize() );
	mDonationWidget->cache()->setMemoryCacheSize( mCache->memoryCacheSize() );
	mDonationWidget->cache()->setWorkingPath( mCache->workingPath() );
	
	mUpdateChecker->setLastUpdated( properties.updateLastUpdated() );
	mUpdateChecker->setLastChecked( properties.updateLastChecked() );
	
	pTranslationManager* translationManager = pTranslationManager::instance();
	translationManager->setTranslationsPaths( properties.translationsPaths() );
	translationManager->setCurrentLocale( properties.locale().name() );
	
	if ( !properties.localeAccepted() ) {
		changeLocaleRequested();
	}
	
	translationManager->reloadTranslations();
	
	foreach ( QWidget* widget, QApplication::topLevelWidgets() ) {
		widget->setLocale( translationManager->currentLocale() );
	}
	
	properties.restoreState( this );
}

void UIMain::saveProperties()
{
	Properties properties;
	
	properties.setUpdateLastUpdated( mUpdateChecker->lastUpdated() );
	properties.setUpdateLastChecked( mUpdateChecker->lastChecked() );
	properties.saveState( this );
}

void UIMain::connectView( PartitionWidget* widget )
{
	connect( widget, SIGNAL( openViewRequested() ), this, SLOT( openViewRequested() ) );
	connect( widget, SIGNAL( closeViewRequested() ), this, SLOT( closeViewRequested() ) );
	connect( widget, SIGNAL( coverRequested( const QString& ) ), this, SLOT( coverRequested( const QString& ) ) );
}

void UIMain::changeLocaleRequested()
{
	pTranslationManager* translationManager = pTranslationManager::instance();
	const QString locale = pTranslationDialog::getLocale( translationManager );
	
	if ( !locale.isEmpty() )
	{
		Properties properties;
		properties.setTranslationsPaths( translationManager->translationsPaths() );
		properties.setLocaleAccepted( true );
		properties.setLocale( QLocale( locale ) );
		
		translationManager->setCurrentLocale( locale );
	}
}

void UIMain::propertiesChanged()
{
	loadProperties();
}

void UIMain::openViewRequested()
{
	PartitionWidget* pw = new PartitionWidget( this );
	pw->setMainView( false );
	pw->setPartitions( mPartitions );
	pw->showHideImportViewButton()->setChecked( false );
	connectView( pw );
	sViews->addWidget( pw );
}

void UIMain::closeViewRequested()
{
	sender()->deleteLater();
}

void UIMain::coverRequested( const QString& id )
{
	mLastDiscId = id;
	
	const QUrl urlCD = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Disc, id );
	const QUrl urlCDCustom = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::DiscCustom, id );
	const QUrl urlCover = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Cover, id );
	
	lCDCover->clear();
	lCover->clear();
	
	if ( mCache->hasCachedData( urlCD ) || mCache->hasCachedData( urlCDCustom ) || mCache->hasCachedData( urlCover ) ) {
		dataNetworkCache_dataCached( QUrl() );
	}
	
	if ( !lCDCover->pixmap() ) {
		mCache->cacheData( urlCD );
	}
	
	if ( !lCover->pixmap() ) {
		mCache->cacheData( urlCover );
	}
}

void UIMain::progress_jobFinished( const QWBFS::Model::Disc& disc )
{
	mExportModel->updateDisc( disc );
}

void UIMain::dataNetworkCache_dataCached( const QUrl& url )
{
	Q_UNUSED( url );
	
	// update all views
	const QList<QAbstractItemView*> views = findChildren<QAbstractItemView*>();
	
	foreach ( QAbstractItemView* view, views ) {
		view->viewport()->update();
	}
	
	// update preview dock
	if ( mLastDiscId.isEmpty() ) {
		return;
	}
	
	const QUrl urlCD = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Disc, mLastDiscId );
	const QUrl urlCDCustom = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::DiscCustom, mLastDiscId );
	const QUrl urlCover = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Cover, mLastDiscId );
	
	if ( mCache->hasCachedData( urlCD ) ) {
		lCDCover->setPixmap( mCache->cachedPixmap( urlCD ) );
	}
	
	if ( mCache->hasCachedData( urlCDCustom ) ) {
		lCDCover->setPixmap( mCache->cachedPixmap( urlCDCustom ) );
	}
	
	if ( mCache->hasCachedData( urlCover ) ) {
		lCover->setPixmap( mCache->cachedPixmap( urlCover ) );
	}
}

void UIMain::dataNetworkCache_error( const QString& message, const QUrl& url )
{
	switch ( QWBFS::WiiTDB::Covers::type( url ) )
	{
		case QWBFS::WiiTDB::Covers::Disc:
			mCache->cacheData( QWBFS::WiiTDB::Covers( url ).url( QWBFS::WiiTDB::Covers::DiscCustom ) );
			return;
		case QWBFS::WiiTDB::Covers::HQ:
		case QWBFS::WiiTDB::Covers::Cover:
		case QWBFS::WiiTDB::Covers::_3D:
		case QWBFS::WiiTDB::Covers::DiscCustom:
		case QWBFS::WiiTDB::Covers::Full:
		case QWBFS::WiiTDB::Covers::Invalid:
			break;
	}
	
	qmtbInfos->appendMessage( message );
}

void UIMain::dataNetworkCache_invalidated()
{
	dataNetworkCache_dataCached( QUrl() );
}

void UIMain::on_aReloadPartitions_triggered()
{
	mPartitions.clear();

#if defined( Q_OS_WIN )
	foreach ( const QFileInfo& drive, QDir::drives() ) {
		mPartitions << drive.absoluteFilePath().remove( ":" ).remove( "/" ).remove( "\\" );
	}
#elif defined( Q_OS_MAC )
	QProcess process;
	process.start( "diskutil list" );
	process.waitForFinished();
	
	const QStringList partitions = QString::fromLocal8Bit( process.readAll() ).split( "\n" );
	
	foreach ( QString partition, partitions ) {
		partition = partition.trimmed();
		
		if ( partition.startsWith( "/" ) || partition.startsWith( "#" ) || partition.isEmpty() ) {
			continue;
		}
		
		partition = partition.simplified().section( ' ', -1 );
		
		// skip disks
		if ( partition[ partition.size() -2 ].toLower() != 's' ) {
			continue;
		}
		
		mPartitions << QString( "/dev/%1" ).arg( partition );
	}
#elif defined( __linux__ )
	QProcess process;
	process.start( "cat /proc/partitions" );
	process.waitForFinished();
	
	const QStringList partitions = QString::fromLocal8Bit( process.readAll() ).split( "\n" );
	
	foreach ( QString partition, partitions ) {
		if ( partition.startsWith( "major" ) || partition.isEmpty() ) {
			continue;
		}
		
		partition = partition.simplified().section( ' ', -1 );
		
		// skip disks
		if ( !partition[ partition.size() -1 ].isDigit() ) {
			continue;
		}
		
		mPartitions << QString( "/dev/%1" ).arg( partition );
	}
#else
	QMessageBox::information( this, QString::null,
		tr(
			"I don't know how to list partition for this platform.\n"
			"You will have to set the correct partition path yourself for mounting partitions."
		) );
#endif
	
	const QList<PartitionWidget*> widgets = sViews->findChildren<PartitionWidget*>();
	
	foreach ( PartitionWidget* widget, widgets ) {
		widget->setPartitions( mPartitions );
	}
}

void UIMain::on_aQuit_triggered()
{
	close();
}

void UIMain::on_aAbout_triggered()
{
	UIAbout* about = new UIAbout( this );
	about->open();
}

void UIMain::on_aProperties_triggered()
{
	saveProperties();
	
	PropertiesDialog* dlg = new PropertiesDialog( this );
	connect( dlg, SIGNAL( propertiesChanged() ), this, SLOT( propertiesChanged() ) );
	
	dlg->open();
}

void UIMain::on_tvFolders_activated( const QModelIndex& index )
{
	const QString filePath = mFoldersModel->filePath( index );
	mFilesModel->setRootPath( filePath );
	lvFiles->setRootIndex( mFilesModel->index( filePath ) );
}

void UIMain::on_tbReloadDrives_clicked()
{
	const QString drive = cbDrives->currentText();
	QFileInfoList drives = QDir::drives();
	QStringList pathsToScan;
	
	cbDrives->clear();
#if defined( Q_OS_WIN )
#elif defined( Q_OS_MAC )
	pathsToScan << "/Volumes";
#else
	pathsToScan  << "/media" << "/mnt";
#endif

	foreach ( const QString& path, pathsToScan ) {
		foreach ( const QFileInfo& fi, QDir( path ).entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot ) ) {
			if ( !drives.contains( fi ) ) {
				drives << fi;
			}
		}
	}
	
	foreach ( const QFileInfo& fi, drives ) {
		cbDrives->addItem( fi.absoluteFilePath() );
	}
	
	if ( !drive.isEmpty() ) {
		cbDrives->setCurrentIndex( cbDrives->findText( drive ) );
	}
}

void UIMain::on_cbDrives_currentIndexChanged( const QString& text )
{
	mFoldersModel->setRootPath( text );
	tvFolders->setRootIndex( mFoldersModel->index( text ) );
	on_tvFolders_activated( tvFolders->rootIndex() );
}

void UIMain::on_tbClearExport_clicked()
{
	mExportModel->clear();
}

void UIMain::on_tbRemoveExport_clicked()
{
	mExportModel->removeSelection( lvExport->selectionModel()->selection() );
}

void UIMain::on_tbExport_clicked()
{
	if ( mExportModel->rowCount() == 0 ) {
		return;
	}
	
	const QString path = QFileDialog::getExistingDirectory( this, tr( "Choose a folder to export the discs" ), QString::null );
	
	if ( path.isEmpty() ) {
		return;
	}
	
	ProgressDialog* dlg = new ProgressDialog( this );
	
	connect( dlg, SIGNAL( jobFinished( const QWBFS::Model::Disc& ) ), this, SLOT( progress_jobFinished( const QWBFS::Model::Disc& ) ) );
	
	dlg->exportDiscs( mExportModel->discs(), path );
}