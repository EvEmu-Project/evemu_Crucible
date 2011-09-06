#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(MailMgrService)

MailMgrService::MailMgrService(PyServiceMgr *mgr)
: PyService(mgr, "mailMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(MailMgrService, PrimeOwners);
	PyCallable_REG_CALL(MailMgrService, SyncMail);
	PyCallable_REG_CALL(MailMgrService, GetMailHeaders);
	PyCallable_REG_CALL(MailMgrService, MoveToTrash);
	PyCallable_REG_CALL(MailMgrService, MoveFromTrash);
	PyCallable_REG_CALL(MailMgrService, MarkAsUnread);
	PyCallable_REG_CALL(MailMgrService, MarkAsRead);
	PyCallable_REG_CALL(MailMgrService, MoveAllToTrash);
	PyCallable_REG_CALL(MailMgrService, MoveToTrashByLabel);
	PyCallable_REG_CALL(MailMgrService, MoveToTrashByList);
	PyCallable_REG_CALL(MailMgrService, MarkAllAsUnread);
	PyCallable_REG_CALL(MailMgrService, MarkAsUnreadByLabel);
	PyCallable_REG_CALL(MailMgrService, MarkAsUnreadByList);
	PyCallable_REG_CALL(MailMgrService, MarkAllAsRead);
	PyCallable_REG_CALL(MailMgrService, MarkAsReadByLabel);
	PyCallable_REG_CALL(MailMgrService, MarkAsReadByList);
	PyCallable_REG_CALL(MailMgrService, MoveAllFromTrash);
	PyCallable_REG_CALL(MailMgrService, EmptyTrash);
	PyCallable_REG_CALL(MailMgrService, DeleteMail);
	PyCallable_REG_CALL(MailMgrService, GetBody);
	PyCallable_REG_CALL(MailMgrService, GetLabels);
	PyCallable_REG_CALL(MailMgrService, EditLabel);
	PyCallable_REG_CALL(MailMgrService, CreateLabel);
	PyCallable_REG_CALL(MailMgrService, DeleteLabel);
	PyCallable_REG_CALL(MailMgrService, AssignLabels);
	PyCallable_REG_CALL(MailMgrService, RemoveLabels);
}

MailMgrService::~MailMgrService() {
	delete m_dispatch;
}

PyResult MailMgrService::Handle_PrimeOwners(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_SyncMail(PyCallArgs &call)
{
	Call_TwoIntegerArgs args;
	if (!args.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode SyncMail args");
		return NULL;
	}

	// referring to mail ids
	int firstId = args.arg1, secondId = args.arg2;

	return NULL;
}

PyResult MailMgrService::Handle_AssignLabels(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_CreateLabel(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_DeleteLabel(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_DeleteMail(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_EditLabel(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_EmptyTrash(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_GetBody(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_GetLabels(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_GetMailHeaders(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MarkAllAsRead(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MarkAllAsUnread(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MarkAsRead(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MarkAsReadByLabel(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MarkAsReadByList(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MarkAsUnread(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MarkAsUnreadByLabel(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MarkAsUnreadByList(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MoveAllFromTrash(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MoveAllToTrash(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MoveFromTrash(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MoveToTrash(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MoveToTrashByLabel(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_MoveToTrashByList(PyCallArgs &call)
{
	return NULL;
}

PyResult MailMgrService::Handle_RemoveLabels(PyCallArgs &call)
{
	return NULL;
}