
 /**
  * @name FleetObject.cpp
  *     Fleet Object code for EVEmu
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "fleet/FleetObject.h"
#include "fleet/FleetBound.h"

PyCallable_Make_InnerDispatcher(FleetObject)

FleetObject::FleetObject(PyServiceMgr *mgr)
: PyService(mgr, "fleetObjectHandler"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FleetObject, CreateFleet);
}

FleetObject::~FleetObject()
{
    delete m_dispatch;
}

PyBoundObject* FleetObject::CreateBoundObject( Client* pClient, const PyRep* bind_args )
{
    if (is_log_enabled(FLEET__BIND_DUMP)) {
        _log( FLEET__BIND_DUMP, "FleetObject bind request for:" );
        bind_args->Dump( FLEET__BIND_DUMP, "    " );
    }

    if (!bind_args->IsInt()) {
        _log(FLEET__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return nullptr;
    }
    /*  do we need to bind object like this?   probably not, cause it works as-is
    //we just bind up a new inventory object for container requested and give it back to them.
    InventoryBound *ib = new InventoryBound(m_manager, item, flag);
    PyRep *result = m_manager->BindObject(call.client, ib);
    */
    return new FleetBound( m_manager, bind_args->AsInt()->value());
}

// FOH::CreateFleet, FOH::
PyResult FleetObject::Handle_CreateFleet(PyCallArgs &call) {
    //self.fleet = sm.RemoteSvc('fleetObjectHandler').CreateFleet()
    FleetBindRSP fbr;
        fbr.nodeID = 888444;    // may have to update this later, or use dedicated fleet node
        fbr.fleetID = sFltSvc.CreateFleet(call.client);
        fbr.unknown = 0;
    if (fbr.fleetID == 0)
        return nullptr;
    return fbr.Encode();
}

/*{'FullPath': u'UI/Messages', 'messageID': 259608, 'label': u'FleetCandidateDodgy1Title'}(u'Fleet Candidate Dodgy', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259609, 'label': u'FleetCandidateDodgy1Body'}(u'The pilot that you wish to add to your fleet is at war with the corporation <b>{corpName}</b>. Confirming that you want him to join your fleet might make all your fleet members vulnerable to this corporation, however they will not be vulnerable to other members of your fleet that are not at war with this corporation unless they decide to attack you. Are you sure you want to add this pilot to your fleet?', None, {u'{corpName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corpName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259610, 'label': u'FleetCandidateDodgyNTitle'}(u'Fleet Candidate Dodgy', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259611, 'label': u'FleetCandidateDodgyNBody'}(u'The pilot that you wish to add to your fleet is at war with these corporations <b>{corpNames}</b>. Confirming that you want him to join your fleet might make all your fleet members vulnerable to these corporations, however they will not be vulnerable to other members of your fleet that are not associated with war with these corporations unless they decide to attack you. Are you sure you want to add this pilot to your fleet?', None, {u'{corpNames}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corpNames'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259612, 'label': u'FleetMembersDodgy1Title'}(u'Join Warring Fleet?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259613, 'label': u'FleetMembersDodgy1Body'}(u'{name} wants you to join their fleet, do you accept?<br><br>This fleet has members that are at war with the corporation {corpName}. Confirming that you will join this fleet will make you vulnerable to this corporation, however they will not be vulnerable to you unless they decide to attack you. Are you sure you want to join this fleet?<br><br>NOTE: Attacking members of your fleet is not a CONCORD sanctioned activity and may result in security status loss and a police response.', None, {u'{name}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'name'}, u'{corpName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corpName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259614, 'label': u'FleetMembersDodgyNTitle'}(u'Join Warring Fleet?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259615, 'label': u'FleetMembersDodgyNBody'}(u'{name} wants you to join their fleet, do you accept?<br><br>This fleet has members that are associated with war with these corporations {corpNames}. Confirming that you will join this fleet will make you vulnerable to these corporations, however they will not be vulnerable to you unless they decide to attack you. Are you sure you want to join this fleet?<br><br>NOTE: Attacking members of your gang is not a CONCORD sanctioned activity and may result in security status loss and a police response.', None, {u'{name}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'name'}, u'{corpNames}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corpNames'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259616, 'label': u'FleetRegroupBody'}(u'Regrouping to {leader}', None, {u'{leader}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'leader'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259617, 'label': u'FleetWarpBody'}(u'Following {leader} in warp', None, {u'{leader}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'leader'}})
 */