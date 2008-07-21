/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/



#include "BookmarkService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "Client.h"
#include "PyServiceCD.h"
#include "PyServiceMgr.h"

PyCallable_Make_InnerDispatcher(BookmarkService)


BookmarkService::BookmarkService(PyServiceMgr *mgr)
: PyService(mgr, "bookmark"),
m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(BookmarkService, GetBookmarks)
	PyCallable_REG_CALL(BookmarkService, BookmarkLocation)
}

BookmarkService::~BookmarkService() {
	delete m_dispatch;
}


PyCallResult BookmarkService::Handle_GetBookmarks(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepObject *rowset = new PyRepObject();
	result = rowset;
	rowset->type = "util.Rowset";
	PyRepDict *args = new PyRepDict();
	rowset->arguments = args;

	//RowClass:
	args->add("RowClass", new PyRepString("util.Row", true));

	//header:
	PyRepList *header = new PyRepList();
	args->add("header", header);
	header->add("bookmarkID");
	header->add("ownerID");
	header->add("itemID");
	header->add("typeID");
	header->add("flag");
	header->add("memo");
	header->add("created");
	header->add("x");
	header->add("y");
	header->add("z");
	header->add("locationID");

	//lines:
	PyRepList *charlist = new PyRepList();
	args->add("lines", charlist);

	return(result);
}


PyCallResult BookmarkService::Handle_BookmarkLocation(PyCallArgs &call) {
	//takes (locationID, None, description)
	PyRep *result = NULL;

	//returns:
	//(bookmarkID, itemID, typeID, x, y, z, locationID,) 
	result = new PyRepNone();
	
	return(result);
}





















