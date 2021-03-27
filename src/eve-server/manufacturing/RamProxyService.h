/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Zhur
    Updates:    Allan
*/

#ifndef __RAM_PROXY_SERVICE__H__
#define __RAM_PROXY_SERVICE__H__

#include "PyService.h"


class RamProxyService : public PyService {
public:
    RamProxyService(PyServiceMgr *mgr);
    virtual ~RamProxyService();

private:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetJobs2);
    PyCallable_DECL_CALL(InstallJob);
    PyCallable_DECL_CALL(CompleteJob);
    PyCallable_DECL_CALL(AssemblyLinesGet);
    PyCallable_DECL_CALL(AssemblyLinesSelect);
    PyCallable_DECL_CALL(GetRelevantCharSkills);
    PyCallable_DECL_CALL(AssemblyLinesSelectCorp);
    PyCallable_DECL_CALL(AssemblyLinesSelectPublic);
    PyCallable_DECL_CALL(AssemblyLinesSelectPrivate);
    PyCallable_DECL_CALL(AssemblyLinesSelectAlliance);
    PyCallable_DECL_CALL(UpdateAssemblyLineConfigurations);

};

#endif

/* InstallJob data
 *
 *
 *        if needInputFlag:
 *            ownerInput, flagInput = quoteData.ownerInputAndflagInput
 *        else:
 *            ownerInput, flagInput = None, const.flagNone
 *        if needOutputFlag:
 *            ownerOutput, flagOutput = quoteData.ownerOutputAndflagOutput
 *        else:
 *            ownerOutput, flagOutput = None, const.flagNone
 *        settings.user.ui.Set('rmInputCombo', (ownerInput, flagInput))
 *        settings.user.ui.Set('rmOutputCombo', (ownerOutput, flagOutput))
 *        if util.IsStation(quoteData.containerID):
 *            invLocationGroupID = const.groupStation
 *        elif quoteData.containerID == session.shipid:
 *            invLocationGroupID = [const.groupSolarSystem, const.groupStation][session.locationid == session.stationid]
 *        else:
 *            invLocationGroupID = const.groupSolarSystem
 *        invLocation = []
 *        if ownerInput == session.charid:
 *            if flagInput == const.flagCargo:
 *                invLocation = [session.locationid, invLocationGroupID]
 *                bomLocationData = [[session.locationid, invLocationGroupID], [[quoteData.containerID, session.charid, flagInput]], []]
 *            elif flagInput == const.flagHangar:
 *                invLocation = [quoteData.containerID, invLocationGroupID]
 *                bomLocationData = [[quoteData.containerID, invLocationGroupID], [[quoteData.containerID, session.charid, flagInput]], []]
 *            else:
 *                raise UserError('RamInstalledItemBadLocation')
 *        elif ownerInput == session.corpid:
 *            if flagInput == const.flagCargo:
 *                raise UserError('RamCorpInstalledItemNotInCargo')
 *            elif flagInput in (const.flagHangar,
 *             const.flagCorpSAG2,
 *             const.flagCorpSAG3,
 *             const.flagCorpSAG4,
 *             const.flagCorpSAG5,
 *             const.flagCorpSAG6,
 *             const.flagCorpSAG7):
 *                if not quoteData.containerID:
 *                    raise UserError('RamPleasePickAnInstalltion')
 *                if util.IsStation(quoteData.containerID):
 *                    offices = sm.GetService('corp').GetMyCorporationsOffices().SelectByUniqueColumnValues('stationID', [quoteData.containerID])
 *                    officeFolderID = None
 *                    officeID = None
 *                    if offices and len(offices):
 *                        for office in offices:
 *                            if quoteData.containerID == office.stationID:
 *                                officeFolderID = office.officeFolderID
 *                                officeID = office.officeID
 *
 *                    if officeID is None:
 *                        raise UserError('RamCorpBOMItemNoSuchOffice', {'location': quoteData.containerID})
 *                    invLocation = [quoteData.containerID, const.groupStation]
 *                    path = []
 *                    path.append([quoteData.containerID, const.ownerStation, 0])
 *                    path.append([officeFolderID, session.corpid, const.flagHangar])
 *                    path.append([officeID, session.corpid, flagInput])
 *                    bomLocationData = [invLocation, path, []]
 *                elif util.IsSolarSystem(quoteData.containerLocationID):
 *                    invLocation = [quoteData.containerLocationID, const.groupSolarSystem]
 *                    path = []
 *                    path.append([quoteData.containerID, session.corpid, flagInput])
 *                    bomLocationData = [invLocation, path, []]
 *                else:
 *                    raise UserError('RamPleasePickAnInstalltion')
 *            else:
 *                raise UserError('RamNotYourItemToInstall')
 *        else:
 *            bomLocationData = None
 *            if not needInputFlag:
 *                if ownerID in (session.charid, session.corpid):
 *                    invLocation = [quoteData.containerID, invLocationGroupID]
 *                    bomLocationData = [[quoteData.containerID, invLocationGroupID], [[const.flagNone]], []]
 *            if bomLocationData is None:
 *                raise UserError('RamNotYourItemToInstall')
 *        installedItemLocationData = sm.GetService('manufacturing').GetPathToItem(quoteData.blueprint)
 *        installationLocationData = [invLocation, [], [quoteData.containerID]]
 *        installationLocationData[2].append(quoteData.assemblyLineID)
 *        calculateQuoteLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/CalculatingQuote')
 *        gettingQuoteLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/GettingQuoteFromInstallation')
 *        sm.GetService('loading').ProgressWnd(calculateQuoteLabel, gettingQuoteLabel, 1, 3)
 *        blue.pyos.synchro.SleepWallclock(0)
 *        quote = None
 *        try:
 *            job = sm.ProxySvc('ramProxy').InstallJob(installationLocationData, installedItemLocationData, bomLocationData, flagOutput, quoteData.buildRuns, quoteData.activityID, quoteData.licensedProductionRuns, not quoteData.ownerFlag, 'blah', quoteOnly=1, installedItem=quoteData.blueprint, maxJobStartTime=quoteData.assemblyLine.nextFreeTime + 1 * MIN, inventionItems=quoteData.inventionItems, inventionOutputItemID=quoteData.inventionItems.outputType)
 *            quote = job.quote
 *            if job.installedItemID:
 *                quoteData.blueprint.itemID = job.installedItemID
 *                installedItemLocationData[2] = [job.installedItemID]
 *        except UserError as e:
 *            cancelingQuoteLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/CancellingQuote')
 *            quoteNotAcceptedLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/QuoteNotAccepted')
 *            sm.GetService('loading').ProgressWnd(cancelingQuoteLabel, quoteNotAcceptedLabel, 1, 1)
 *            raise
 */

/* personal job
 * 21:48:04 [Service] ramProxy::InstallJob()
 * 21:48:04 [ManufDump] RamProxyService::Handle_InstallJob() - size 9
 * 21:48:04 [ManufDump]   Call Arguments:
 * 21:48:04 [ManufDump]      Tuple: 9 elements
 * 21:48:04 [ManufDump]       [ 0]   List: 3 elements                       AssemblyLineData
 * 21:48:04 [ManufDump]       [ 0]   [ 0]   List: 2 elements                  location,
 * 21:48:04 [ManufDump]       [ 0]   [ 0]   [ 0]    Integer: 60014140           lineLocationID  (station, ss, ship)
 * 21:48:04 [ManufDump]       [ 0]   [ 0]   [ 1]    Integer: 15                 lineLocationGroupID
 * 21:48:04 [ManufDump]       [ 0]   [ 1]   List: Empty                       [],  (empty)
 * 21:48:04 [ManufDump]       [ 0]   [ 2]   List: 2 elements                  container
 * 21:48:04 [ManufDump]       [ 0]   [ 2]   [ 0]    Integer: 60014140           lineContainerID
 * 21:48:04 [ManufDump]       [ 0]   [ 2]   [ 1]    Integer: 117701             AssemblyLineID
 * 21:48:04 [ManufDump]       [ 1]   List: 3 elements                       bpData
 * 21:48:04 [ManufDump]       [ 1]   [ 0]   List: 2 elements                  bpLocationData,
 * 21:48:04 [ManufDump]       [ 1]   [ 0]   [ 0]    Integer: 60014140           bpLocationID
 * 21:48:04 [ManufDump]       [ 1]   [ 0]   [ 1]    Integer: 15                 bpLocationGroupID
 * 21:48:04 [ManufDump]       [ 1]   [ 1]   List: 1 elements                  bpLocPath
 * 21:48:04 [ManufDump]       [ 1]   [ 1]   [ 0]   List: 3 elements             bp PathElement
 * 21:48:04 [ManufDump]       [ 1]   [ 1]   [ 0]   [ 0]    Integer: 60014140      locationID
 * 21:48:04 [ManufDump]       [ 1]   [ 1]   [ 0]   [ 1]    Integer: 90000000      ownerID
 * 21:48:04 [ManufDump]       [ 1]   [ 1]   [ 0]   [ 2]    Integer: 4             flagID
 * 21:48:04 [ManufDump]       [ 1]   [ 2]   List: 1 elements                  bpItemData
 * 21:48:04 [ManufDump]       [ 1]   [ 2]   [ 0]    Integer: 140024212          bpItemID
 * 21:48:04 [ManufDump]       [ 2]   List: 3 elements                       bomData (personal)
 * 21:48:04 [ManufDump]       [ 2]   [ 0]   List: 2 elements                  bomLocation,
 * 21:48:04 [ManufDump]       [ 2]   [ 0]   [ 0]    Integer: 60014140           bomLocationID
 * 21:48:04 [ManufDump]       [ 2]   [ 0]   [ 1]    Integer: 15                 bomLocationGroupID
 * 21:48:04 [ManufDump]       [ 2]   [ 1]   List: 1 elements                  bomLocPath
 * 21:48:04 [ManufDump]       [ 2]   [ 1]   [ 0]   List: 3 elements             bom PathElement
 * 21:48:04 [ManufDump]       [ 2]   [ 1]   [ 0]   [ 0]    Integer: 60014140      locationID
 * 21:48:04 [ManufDump]       [ 2]   [ 1]   [ 0]   [ 1]    Integer: 90000000      ownerID
 * 21:48:04 [ManufDump]       [ 2]   [ 1]   [ 0]   [ 2]    Integer: 4             flagID
 * 21:48:04 [ManufDump]       [ 2]   [ 2]   List: Empty                       empty in station
 * 21:48:04 [ManufDump]       [ 3]    Integer: 4                            outputFlag
 * 21:48:04 [ManufDump]       [ 4]    Integer: 10                           jobRuns
 * 21:48:04 [ManufDump]       [ 5]    Integer: 1                            activityID
 * 21:48:04 [ManufDump]       [ 6]    Integer: 0                            copyRuns (populated on copy)
 * 21:48:04 [ManufDump]       [ 7]    Boolean: false                        isCorpJob
 * 21:48:04 [ManufDump]       [ 8]     String: 'blah'                       this is hard-coded in client
 */
/* corp job
 * 00:32:22 [Service] ramProxy::InstallJob()
 * 00:32:22 [ManufDump] RamProxyService::Handle_InstallJob() - size 9
 * 00:32:22 [ManufDump]   Call Arguments:
 * 00:32:22 [ManufDump]      Tuple: 9 elements
 * 00:32:22 [ManufDump]       [ 0]   List: 3 elements                       AssemblyLineData
 * 00:32:22 [ManufDump]       [ 0]   [ 0]   List: 2 elements                  location
 * 00:32:22 [ManufDump]       [ 0]   [ 0]   [ 0]    Integer: 60014140           lineLocationID  (station, ss, ship)
 * 00:32:22 [ManufDump]       [ 0]   [ 0]   [ 1]    Integer: 15                 lineLocationGroupID
 * 00:32:22 [ManufDump]       [ 0]   [ 1]   List: Empty                       [],  (empty)
 * 00:32:22 [ManufDump]       [ 0]   [ 2]   List: 2 elements                  container
 * 00:32:22 [ManufDump]       [ 0]   [ 2]   [ 0]    Integer: 60014140           lineContainerID
 * 00:32:22 [ManufDump]       [ 0]   [ 2]   [ 1]    Integer: 117703             AssemblyLineID
 * 00:32:22 [ManufDump]       [ 1]   List: 3 elements                       bpData
 * 00:32:22 [ManufDump]       [ 1]   [ 0]   List: 2 elements                  bpLocationData
 * 00:32:22 [ManufDump]       [ 1]   [ 0]   [ 0]    Integer: 60014140           bpLocationID
 * 00:32:22 [ManufDump]       [ 1]   [ 0]   [ 1]    Integer: 15                 bpLocationGroupID
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   List: 3 elements                  bpLocPath PathElement
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 0]   List: 3 elements             pathLocationData
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 0]   [ 0]    Integer: 60014140      pathLocationID
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 0]   [ 1]    Integer: 4             pathFlagID/ownerStation
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 0]   [ 2]    Integer: 0             zero
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 1]   List: 3 elements             officeLocationData
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 1]   [ 0]    Integer: 66014140      officeFolderID
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 1]   [ 1]    Integer: 98000001      officeLocCorpID
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 1]   [ 2]    Integer: 4             officeLocFlagID
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 2]   List: 3 elements             officeData
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 2]   [ 0]    Integer: 100000000     officeID
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 2]   [ 1]    Integer: 98000001      officeCorpID
 * 00:32:22 [ManufDump]       [ 1]   [ 1]   [ 2]   [ 2]    Integer: 4             officeFlagID
 * 00:32:22 [ManufDump]       [ 1]   [ 2]   List: 1 elements                  bpItemData
 * 00:32:22 [ManufDump]       [ 1]   [ 2]   [ 0]    Integer: 140024209          bpItemID
 * 00:32:22 [ManufDump]       [ 2]   List: 3 elements                       bomData (corp)
 * 00:32:22 [ManufDump]       [ 2]   [ 0]   List: 2 elements                  bomLocation
 * 00:32:22 [ManufDump]       [ 2]   [ 0]   [ 0]    Integer: 60014140           bomLocationID
 * 00:32:22 [ManufDump]       [ 2]   [ 0]   [ 1]    Integer: 15                 bomLocationGroupID
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   List: 3 elements                  bomLocPath PathElement
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 0]   List: 3 elements             pathLocationData
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 0]   [ 0]    Integer: 60014140      pathLocationID
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 0]   [ 1]    Integer: 4             pathFlagID
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 0]   [ 2]    Integer: 0             zero
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 1]   List: 3 elements             officeLocationData
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 1]   [ 0]    Integer: 66014140      officeFolderID
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 1]   [ 1]    Integer: 98000001      officeLocCorpID
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 1]   [ 2]    Integer: 4             officeLocFlagID
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 2]   List: 3 elements             officeData
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 2]   [ 0]    Integer: 100000000     officeID
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 2]   [ 1]    Integer: 98000001      officeCorpID
 * 00:32:22 [ManufDump]       [ 2]   [ 1]   [ 2]   [ 2]    Integer: 4             officeFlagID
 * 00:32:22 [ManufDump]       [ 2]   [ 2]   List: Empty                       empty for station
 * 00:32:22 [ManufDump]       [ 3]    Integer: 4                            outputFlag
 * 00:32:22 [ManufDump]       [ 4]    Integer: 10                           jobRuns
 * 00:32:22 [ManufDump]       [ 5]    Integer: 1                            activityID
 * 00:32:22 [ManufDump]       [ 6]    Integer: 0                            copyRuns (populated on copy)
 * 00:32:22 [ManufDump]       [ 7]    Boolean: true                         isCorpJob
 * 00:32:22 [ManufDump]       [ 8]     String: 'blah'
 */
