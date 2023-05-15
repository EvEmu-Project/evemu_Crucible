
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



#include "fleet/FleetObject.h"
#include "fleet/FleetBound.h"
#include "services/ServiceManager.h"

FleetObject::FleetObject(EVEServiceManager& mgr) :
    BindableService("fleetObjectHandler", mgr)
{
    this->Add("CreateFleet", &FleetObject::CreateFleet);
}

BoundDispatcher* FleetObject::BindObject(Client* client, PyRep* bindParameters)
{
    if (is_log_enabled(FLEET__BIND_DUMP)) {
        _log( FLEET__BIND_DUMP, "FleetObject bind request for:" );
        bindParameters->Dump( FLEET__BIND_DUMP, "    " );
    }

    if (!bindParameters->IsInt()) {
        _log(FLEET__ERROR, "%s Service: invalid bind argument type %s", GetName().c_str(), bindParameters->TypeString());
        return nullptr;
    }

    uint32 fleetID = bindParameters->AsInt()->value();
    auto it = this->m_instances.find (fleetID);

    if (it != this->m_instances.end ())
        return it->second;

    FleetBound* bound = new FleetBound(this->GetServiceManager(), *this, bindParameters->AsInt()->value());

    this->m_instances.insert_or_assign (fleetID, bound);

    return bound;
}

void FleetObject::BoundReleased (FleetBound* bound) {
    auto it = this->m_instances.find (bound->GetFleetID());

    if (it == this->m_instances.end ())
        return;

    this->m_instances.erase (it);
}

// FOH::CreateFleet, FOH::
PyResult FleetObject::CreateFleet(PyCallArgs &call) {
    //self.fleet = sm.RemoteSvc('fleetObjectHandler').CreateFleet()
    FleetBindRSP fbr;
        fbr.nodeID = this->GetServiceManager().GetNodeID();    // may have to update this later, or use dedicated fleet node
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