/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : PartitionStatus.cpp
** Date      : 2010-04-25T13:05:33
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a crossplatform WBFS Manager developed using Qt4/C++.
** It's currently working under Unix/Linux, Mac OS X, and build under windows (but not yet working).
** 
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR WII CONSOLE OR WII PARTITION
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "PartitionStatus.h"

using namespace QWBFS::Partition;

Status::Status( const QWBFS::Partition::Handle& handle )
{
	blocks = handle.isValid() ? wbfs_count_usedblocks( handle.ptr() ) : -1;
	size = handle.isValid() ? (double)handle.ptr()->n_wbfs_sec *handle.ptr()->wbfs_sec_sz : -1;
	used = handle.isValid() ? (double)( handle.ptr()->n_wbfs_sec -blocks ) *handle.ptr()->wbfs_sec_sz : -1;
	free = handle.isValid() ? (double)(blocks) *handle.ptr()->wbfs_sec_sz : -1;
}