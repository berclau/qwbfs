/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PartitionWidget.cpp
** Date      : 2010-06-16T14:19:29
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
#include "PartitionWidget.h"
#include "qwbfsdriver/Driver.h"
#include "models/DiscModel.h"
#include "models/DiscDelegate.h"
#include "ProgressDialog.h"
#include "UIMain.h"
#include "datacache/DataNetworkCache.h"

#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

PartitionWidget::PartitionWidget( QWidget* parent )
	: QWidget( parent )
{
	Q_ASSERT( parent );
	const UIMain* window = qobject_cast<UIMain*>( parent->window() );
	
	mDriver = new QWBFS::Driver( this );
	mDiscModel = new QWBFS::Model::DiscModel( this, mDriver );
	mImportModel = new QWBFS::Model::DiscModel( this, mDriver );
	
	setupUi( this );
	setAcceptDrops( true );
	lvDiscs->setModel( mDiscModel );
	lvDiscs->setItemDelegate( new QWBFS::Model::DiscDelegate( mDiscModel, window->cache() ) );
	lvImport->setModel( mImportModel );
	lvImport->setItemDelegate( new QWBFS::Model::DiscDelegate( mImportModel, window->cache() ) );
	
	sViews->setSizes( QList<int>() << QWIDGETSIZE_MAX << fImport->minimumSizeHint().height() );
	
	connect( mDiscModel, SIGNAL( countChanged( int ) ), this, SLOT( models_countChanged() ) );
	connect( mImportModel, SIGNAL( countChanged( int ) ), this, SLOT( models_countChanged() ) );
	connect( lvDiscs->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( views_selectionChanged() ) );
	connect( lvImport->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( views_selectionChanged() ) );
	connect( cbPartitions->lineEdit(), SIGNAL( textChanged( const QString& ) ), this, SLOT( setCurrentPartition( const QString& ) ) );
	connect( cbPartitions->lineEdit(), SIGNAL( returnPressed() ), tbLoad, SLOT( click() ) );
}

PartitionWidget::~PartitionWidget()
{
	mDriver->close();
}

const QWBFS::Driver* PartitionWidget::driver() const
{
	return mDriver;
}

QWBFS::Model::DiscModel* PartitionWidget::discModel() const
{
	return mDiscModel;
}

QWBFS::Model::DiscModel* PartitionWidget::importModel() const
{
	return mImportModel;
}

QToolButton* PartitionWidget::showHideImportViewButton() const
{
	return tbShowHideImportView;
}

QString PartitionWidget::currentPartition() const
{
	return cbPartitions->currentText();
}

void PartitionWidget::setMainView( bool main )
{
	tbOpen->setVisible( main );
	tbClose->setVisible( !main );
}

void PartitionWidget::setPartitions( const QStringList& partitions )
{
	const QString current = currentPartition();

	const bool locked = cbPartitions->blockSignals( true );
	cbPartitions->clear();
	cbPartitions->addItems( partitions );
	cbPartitions->setEditText( current );
	cbPartitions->blockSignals( locked );
}

void PartitionWidget::setCurrentPartition( const QString& partition )
{
	mDriver->setPartition( partition );
	cbPartitions->setEditText( partition );
}

void PartitionWidget::showError( const QString& error )
{
	QMessageBox::information( this, QString::null, error );
}

void PartitionWidget::showError( int error )
{
	showError( QWBFS::Driver::errorToString( QWBFS::Driver::Error( error ) ) );
}

void PartitionWidget::dragEnterEvent( QDragEnterEvent* event )
{
	if ( mDriver->isOpen() ) {
		foreach ( const QString& mimeType, mImportModel->mimeTypes() ) {
			if ( event->mimeData()->hasFormat( mimeType ) ) {
				event->setDropAction( Qt::CopyAction );
				event->accept();
				return;
			}
		}
	}
}

void PartitionWidget::dropEvent( QDropEvent* event )
{
	if ( !fImport->isVisible() ) {
		tbShowHideImportView->toggle();
	}
	
	mImportModel->dropMimeData( event->mimeData(), event->proposedAction(), -1, -1, QModelIndex() );
	event->acceptProposedAction();
}

void PartitionWidget::models_countChanged()
{
	QWBFS::Partition::Status status;
	/*const int result = */mDriver->status( status );
	
	gStatus->setSize( status.size );
	gStatus->setUsedSize( status.used );
	gStatus->setFreeSize( status.free );
	gStatus->setTemporarySize( mImportModel->size() );
	lInformations->setText( tr( "%1 disc(s) on the partition - %2 disc(s) to import." ).arg( mDiscModel->rowCount() ).arg( mImportModel->rowCount() ) );
	
	/*if ( result != QWBFS::Driver::Ok ) {
		showError( result );
	}*/
}

void PartitionWidget::views_selectionChanged()
{
	const QItemSelectionModel* sm = qobject_cast<const QItemSelectionModel*>( sender() );
	const QWBFS::Model::DiscModel* model = qobject_cast<const QWBFS::Model::DiscModel*>( sm->model() );
	
	foreach ( const QModelIndex& index, sm->selectedIndexes() ) {
		const QString discId = model->discId( index );
		emit coverRequested( discId );
	}
}

void PartitionWidget::progress_jobFinished( const QWBFS::Model::Disc& disc )
{
	mImportModel->updateDisc( disc );
}

void PartitionWidget::progress_finished()
{
	tbLoad->click();
}

void PartitionWidget::on_cbPartitions_currentIndexChanged( int index )
{
	setCurrentPartition( cbPartitions->itemText( index ) );
}	

void PartitionWidget::on_tbLoad_clicked()
{
	if ( !mDriver->open() && !mDriver->partition().isEmpty() ) {
		showError( tr( "Can't open partition." ) );
	}
	
	if ( mDiscModel->rowCount() == 0 ) {
		models_countChanged();
	}
	else {
		mDiscModel->clear();
	}
	
	if ( mDriver->isOpen() ) {
		QWBFS::Model::DiscList discs;
		const int result = mDriver->discList( discs );
		
		if ( result == QWBFS::Driver::Ok ) {
			mDiscModel->setDiscs( discs );
			
			if ( mDiscModel->rowCount() == 0 ) {
				models_countChanged();
			}
		}
		else {
			showError( result );
		}
	}
}

void PartitionWidget::on_tbFormat_clicked()
{
	const QString text = tr( "The partition '%1' will be formatted,\nall data will be erased permanently, are you sure?" ).arg( mDriver->partition() );
	const QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
	const QMessageBox::StandardButton button = QMessageBox::No;
	
	if ( QMessageBox::question( this, QString::null, text, buttons, button ) == button ) {
		return;
	}
	
	if ( mDriver->format() ) {
		tbLoad->click();
	}
	else {
		showError( tr( "Can't format partition." ) );
	}
}

void PartitionWidget::on_tbOpen_clicked()
{
	emit openViewRequested();
}

void PartitionWidget::on_tbClose_clicked()
{
	emit closeViewRequested();
}

void PartitionWidget::on_tbRemoveDiscs_clicked()
{
	const QModelIndexList indexes = lvDiscs->selectionModel()->selectedIndexes();
	
	const QString text = tr( "You are about to permanently delete %1 disc(s) on partition '%2', are you sure?" ).arg( indexes.count() ).arg( mDriver->partition() );
	const QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
	const QMessageBox::StandardButton button = QMessageBox::No;
	
	if ( indexes.isEmpty() || QMessageBox::question( this, QString::null, text, buttons, button ) == button ) {
		return;
	}
	
	int errors = 0;
	
	for ( int i = indexes.count() -1; i >= 0; i-- ) {
		const QModelIndex& index = indexes[ i ];
		const QString discId = mDiscModel->discId( index );
		
		if ( mDriver->removeDisc( discId ) == QWBFS::Driver::Ok ) {
			mDiscModel->removeRow( index.row() );
		}
		else {
			errors++;
		}
	}
	
	if ( errors > 0 ) {
		showError( tr( "One or more discs have failed to be removed." ) );
	}
}

void PartitionWidget::on_tbRenameDisc_clicked()
{
	const QModelIndex index = lvDiscs->selectionModel()->selectedIndexes().value( 0 );
	const QWBFS::Model::Disc disc = mDiscModel->disc( index );
	
	if ( !index.isValid() ) {
		return;
	}
	
	const QString name = QInputDialog::getText( this, QString::null, tr( "Choose a new name for the disc" ), QLineEdit::Normal, disc.title );
	
	if ( name.isNull() ) {
		return;
	}
	
	if ( mDriver->renameDisc( disc.id, name ) == QWBFS::Driver::Ok ) {
		mDiscModel->setData( index, name, Qt::DisplayRole );
	}
	else {
		showError( tr( "Can't rename disc id #%1 (%2) to '%3'" ).arg( disc.id ).arg( disc.title ).arg( name ) );
	}
}

void PartitionWidget::on_tbClearImport_clicked()
{
	mImportModel->clear();
}

void PartitionWidget::on_tbRemoveImport_clicked()
{
	mImportModel->removeSelection( lvImport->selectionModel()->selection() );
}

void PartitionWidget::on_tbImport_clicked()
{
	if ( mImportModel->rowCount() == 0 ) {
		return;
	}
	
	ProgressDialog* dlg = new ProgressDialog( this );
	
	connect( dlg, SIGNAL( jobFinished( const QWBFS::Model::Disc& ) ), this, SLOT( progress_jobFinished( const QWBFS::Model::Disc& ) ) );
	connect( dlg, SIGNAL( finished() ), this, SLOT( progress_finished() ) );
	
	dlg->importDiscs( mImportModel->discs(), mDriver->handle() );
}