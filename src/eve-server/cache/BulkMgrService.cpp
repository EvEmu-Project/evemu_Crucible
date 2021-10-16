/**
 * @name BulkMgrService.cpp
 *   This file is for processing bulkdata
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    ozatomic (hacked for static client data)
 * @Author:    Allan (added calls and (hacked) updates for new dgm data)
 * @date:      24 January 2017
 *
 */


/** @todo  this system will need to work with objectCache, as those ARE the bulkdata files sent to the client.
 *  they are already packed and loaded into the server.  this system ive created for bulkData is redundant.
 * will have to look into sorting/sending to client eventually, using hash comparison to determine what files to send. (if possible)
 * for now, we'll leave this as-is, because it seems to be working ok, and i *MAY NOT* have complete data for all the bulkfiles
 */

/** @update  the client install has default bulkdata files found in the eve/bulkdata directory.  need to get these files and compare with
 * what we have to see what needs to be updated, and go from there.
 * next comment has all data used by client (found in code)
 */

/** @todo  @update  used by client...
 *
        self.dgmunits = self.LoadBulkIndex('dgmunits', const.cacheDogmaUnits, 'unitID')
        self.invbptypes = self.LoadBulkIndex('invbptypes', const.cacheInvBlueprintTypes, 'blueprintTypeID')
        self.invcategories = self.LoadBulkIndex('invcategories', const.cacheInvCategories, 'categoryID', InvCategory)
        self.invmetagroups = self.LoadBulkIndex('invmetagroups', const.cacheInvMetaGroups, 'metaGroupID', InvMetaGroup)
        self.invgroups = self.LoadBulkIndex('invgroups', const.cacheInvGroups, 'groupID', InvGroup)
        self.invtypes = self.LoadBulkIndex('invtypes', const.cacheInvTypes, 'typeID', InvType)
        self.invtypereactions = self.LoadBulkFilter('invtypereactions', const.cacheInvTypeReactions, 'reactionTypeID')
        self.invmetatypes = self.LoadBulkIndex('invmetatypes', const.cacheInvMetaTypes, 'typeID')
        self.invmetatypesByParent = self.LoadBulkFilter('invmetatypesByParent', const.cacheInvMetaTypes, 'parentTypeID')
        invcontrabandtypes = self.LoadBulk(None, const.cacheInvContrabandTypes)

        npccharacters = self.LoadBulkIndex(None, const.cacheChrNpcCharacters, 'characterID')

        self.regions = self.LoadBulkIndex('regions', const.cacheMapRegionsTable, 'regionID', Region)
        self.constellations = self.LoadBulkIndex('constellations', const.cacheMapConstellationsTable, 'constellationID', Constellation)
        self.solarsystems = self.LoadBulkIndex('solarsystems', const.cacheMapSolarSystemsTable, 'solarSystemID', SolarSystem)
        self.stations = self.LoadBulk('stations', const.cacheStaStationsStatic, Recordset(sys.Row, 'stationID', 'GetStationEx', 'GetMultiStationEx'))

        self.messages = self.LoadBulkIndex('messages', const.cacheEveMessages, 'messageKey')
        self.graphics = self.LoadBulkIndex('graphics', const.cacheResGraphics, 'graphicID')
        self.icons = self.LoadBulkIndex('icons', const.cacheResIcons, 'iconID')
        self.sounds = self.LoadBulkIndex('sounds', const.cacheResSounds, 'soundID')
        self.detailMeshes = self.LoadBulkIndex('detailMeshes', const.cacheResDetailMeshes, 'detailGraphicID')
        self.detailMeshesByTarget = self.LoadBulkFilter('detailMeshesByTarget', const.cacheResDetailMeshes, 'targetGraphicID')
        self._worldspacesDistrictsCache = {}
        self.worldspaces = self.LoadBulkIndex('worldspaces', const.cacheWorldSpaces, 'worldSpaceTypeID', sys.Worldspaces)
        self.entitySpawns = self.LoadBulkIndex('entitySpawns', const.cacheEntitySpawns, 'spawnID')
        self.entitySpawnsByWorldSpaceID = self.LoadBulkFilter('entitySpawnsByWorldSpaceID', const.cacheEntitySpawns, 'worldSpaceTypeID')
        self.entitySpawnsByRecipeID = self.LoadBulkFilter('entitySpawnsByRecipeID', const.cacheEntitySpawns, 'recipeID')
        self.entityIngredientInitialValues = self.LoadBulkFilter('entityIngredientInitialValues', const.cacheEntityIngredientInitialValues, 'ingredientID')
        self.entityIngredientsByParentID = self.LoadBulkFilter('entityIngredientsByParentID', const.cacheEntityIngredients, 'parentID')
        self.entityIngredientsByRecipeID = self.LoadBulkFilter('entityIngredientsByRecipeID', const.cacheEntityIngredients, 'recipeID')
        self.ingredients = self.LoadBulkIndex('ingredients', const.cacheEntityIngredients, 'ingredientID')
        self.entitySpawnGroups = self.LoadBulkIndex('entitySpawnGroups', const.cacheEntitySpawnGroups, 'spawnGroupID')
        self.entitySpawnsBySpawnGroup = self.LoadBulkFilter('entitySpawnsBySpawnGroup', const.cacheEntitySpawnGroupLinks, 'spawnGroupID')
        self.recipes = self.LoadBulkIndex('recipes', const.cacheEntityRecipes, 'recipeID')
        self.recipesByParentRecipeID = self.LoadBulkFilter('recipesByParentRecipeID', const.cacheEntityRecipes, 'parentRecipeID')
        self.treeNodes = self.LoadBulkIndex('treeNodes', const.cacheTreeNodes, 'treeNodeID')
        self.treeLinks = self.LoadBulkFilter('treeLinks', const.cacheTreeLinks, 'parentTreeNodeID')
        self.treeNodeProperties = self.LoadBulkIndex('treeNodeProperties', const.cacheTreeProperties, 'propertyID')
        self.actionTreeSteps = self.LoadBulkFilter('actionTreeSteps', const.cacheActionTreeSteps, 'actionID')
        self.actionTreeProcs = self.LoadBulkFilter('actionTreeProcs', const.cacheActionTreeProcs, 'actionID')
        self.actionObjects = self.LoadBulkIndex('actionObjects', const.cacheActionObjects, 'actionObjectID')
        self.actionStations = self.LoadBulkIndex('actionStations', const.cacheActionStations, 'actionStationTypeID')
        self.actionStationActions = self.LoadBulkFilter('actionStationActions', const.cacheActionStationActions, 'actionStationTypeID')
        self.actionObjectStations = self.LoadBulkFilter('actionObjectStations', const.cacheActionObjectStations, 'actionObjectID')
        self.actionObjectExits = self.LoadBulkFilter('actionObjectExits', const.cacheActionObjectExits, ['actionObjectID', 'actionStationInstanceID', True])
        self.paperdollModifierLocations = self.LoadBulkIndex('paperdollModifierLocations', const.cachePaperdollModifierLocations, 'modifierLocationID')
        self.paperdollResources = self.LoadBulkIndex('paperdollResources', const.cachePaperdollResources, 'paperdollResourceID')
        self.paperdollSculptingLocations = self.LoadBulkIndex('paperdollSculptingLocations', const.cachePaperdollSculptingLocations, 'sculptLocationID')
        self.paperdollColors = self.LoadBulkIndex('paperdollColors', const.cachePaperdollColors, 'colorID')
        self.paperdollColorNames = self.LoadBulkIndex('paperdollColorNames', const.cachePaperdollColorNames, 'colorNameID')
        self.paperdollColorRestrictions = self.LoadBulkFilter('paperdollColorRestrictions', const.cachePaperdollColorRestrictions, 'colorNameID')
        self.perceptionSenses = self.LoadBulkIndex('perceptionSenses', const.cachePerceptionSenses, 'senseID')
        self.perceptionStimTypes = self.LoadBulkIndex('perceptionStimTypes', const.cachePerceptionStimTypes, 'stimTypeID')
        self.perceptionSubjects = self.LoadBulkIndex('perceptionSubjects', const.cachePerceptionSubjects, 'subjectID')
        self.perceptionTargets = self.LoadBulkIndex('perceptionTargets', const.cachePerceptionTargets, 'targetID')
        self.perceptionBehaviorSenses = self.LoadBulk('perceptionBehaviorSenses', const.cachePerceptionBehaviorSenses)
        self.perceptionBehaviorDecays = self.LoadBulk('perceptionBehaviorDecays', const.cachePerceptionBehaviorDecays)
        self.perceptionBehaviorFilters = self.LoadBulk('perceptionBehaviorFilters', const.cachePerceptionBehaviorFilters)
        self.encounters = self.LoadBulkIndex('encounters', const.cacheEncounterEncounters, 'encounterID')
        self.encounterCoordinateSets = self.LoadBulkIndex('encounterCoordinateSets', const.cacheEncounterCoordinateSets, 'coordinateSetID')
        self.encounterCoordinatesBySet = self.LoadBulkFilter('encounterCoordinatesBySet', const.cacheEncounterCoordinates, 'coordinateSetID')
        self.encounterCoordinates = self.LoadBulkIndex('encounterCoordinates', const.cacheEncounterCoordinates, 'coordinateID')
        self.encounterCoordinateSetsByWorldSpaceID = self.LoadBulkFilter('encounterCoordinateSetsByWorldSpaceID', const.cacheEncounterCoordinateSets, 'worldSpaceTypeID')
        self.encountersByCoordinateSet = self.LoadBulkFilter('encountersByCoordinateSet', const.cacheEncounterEncounters, 'coordinateSetID')

 *
 */

#include "eve-server.h"

#include "PyServiceCD.h"
#include "cache/BulkDB.h"
#include "cache/BulkMgrService.h"
#include "packets/BulkDataPkts.h"

PyCallable_Make_InnerDispatcher(BulkMgrService)

BulkMgrService::BulkMgrService( PyServiceMgr *mgr )
: PyService(mgr, "bulkMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BulkMgrService, GetChunk);
    PyCallable_REG_CALL(BulkMgrService, UpdateBulk);
    PyCallable_REG_CALL(BulkMgrService, GetVersion);
    PyCallable_REG_CALL(BulkMgrService, GetFullFiles);
    PyCallable_REG_CALL(BulkMgrService, GetAllBulkIDs);
    PyCallable_REG_CALL(BulkMgrService, GetFullFilesChunk);
    PyCallable_REG_CALL(BulkMgrService, GetUnsubmittedChunk);
    PyCallable_REG_CALL(BulkMgrService, GetUnsubmittedChanges);

}

BulkMgrService::~BulkMgrService() {
    delete m_dispatch;
}
/*
BULKDATA__ERROR=1
BULKDATA__WARNING=0
BULKDATA__MESSAGE=0
BULKDATA__DEBUG=0
BULKDATA__INFO=0
BULKDATA__TRACE=0
BULKDATA__DUMP=0
*/
PyResult BulkMgrService::Handle_UpdateBulk(PyCallArgs &call)
{
    /*
    sLog.White( "BulkMgrService::Handle_UpdateBulk()", "size=%li", call.tuple->size());
    call.Dump(BULKDATA__DUMP);
    updateData = self.bulkMgr.UpdateBulk(changeID, hashValue, branch)

        updateType = updateData['type']
        self.allowUnsubmitted = updateData['allowUnsubmitted']
        if 'version' in updateData:
            serverVersion = updateData['version']
        if 'data' in updateData:        -- list of bulkdata fileID 'numbers' that have changed.
            updateInfo = updateData['data']
    */

    Call_UpdateBulk args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    PyDict* res = new PyDict();
    // bulkDataChangeID found in eve-common/EVE_Defines.h and defines the serverVersion of this set of bulkdata
    if (args.changeID != bulkDataChangeID) {
        res->SetItemString("type", new PyInt(updateBulkStatusTooManyRevisions));
    } else if (args.branch != bulkDataBranch) {
        res->SetItemString("type", new PyInt(updateBulkStatusWrongBranch));
    } else if (args.hashValue->IsNone()) {  //241bfba3c85c1bb4680be745e6c7d1ee
        // not right response, but easiest to hack, as it compares servers fileIDs to local fileIDs and removes matching ids
        res->SetItemString("type", new PyInt(updateBulkStatusHashMismatch));
        // make list of fileIDs to send to client.
        PyList* list = new PyList();
            list->AddItem(new PyInt(800002));   //cacheDogmaOperands
            list->AddItem(new PyInt(800003));   //cacheDogmaExpressions
            list->AddItem(new PyInt(800004));   //cacheDogmaAttributes
            list->AddItem(new PyInt(800005));   //cacheDogmaEffects
            list->AddItem(new PyInt(800006));   //cacheDogmaTypeAttributes
            list->AddItem(new PyInt(800007));   //cacheDogmaTypeEffects
        res->SetItemString("data", list);
    } else {
        res->SetItemString("type", new PyInt(updateBulkStatusOK));
    }

    res->SetItemString("version", new PyInt(bulkDataChangeID));
    res->SetItemString("allowUnsubmitted", PyStatic.NewFalse());

    /*
    res->SetItemString("data", new PyList(0));
        data is PyDict of 'chunkCount','chunk','changedTablesKeys','toBeDeleted','changedTablesKeys','branch' when 'type' = updateBulkStatusNeedToUpdate
        */
    if (is_log_enabled(BULKDATA__TRACE))
        res->Dump(BULKDATA__TRACE, "  ");

    return res;
}

PyResult BulkMgrService::Handle_GetFullFiles(PyCallArgs &call)
{
    /*
    sLog.White( "BulkMgrService::Handle_GetFullFiles()", "size=%li", call.tuple->size());
    call.Dump(BULKDATA__DUMP);
        toBeChanged, bulksEndingInChunk, numberOfChunks, chunkSetID, self.allowUnsubmitted = self.bulkMgr.GetFullFiles(toGet)
        -- toGet is sent as PyList of fileIDs server should send back

    -- response
    [PyTuple 1 items]
      [PySubStream 151253 bytes]
        [PyTuple 5 items]
          [PyDict 3 kvp]        << toBeChanged
            [PyInt 800001]      << fileID
            [PyObjectEx Type2]  << file data
              [PyTuple 2 items]
                [PyTuple 1 items]
                  [PyToken dbutil.CRowset]
                [PyDict 1 kvp]
                [PyString "header"]
          [PyList 2 items]      << bulksEndingInChunk
            [PyInt 800001]
            [PyInt 800002]
          [PyInt 197]           << numberOfChunks
          [PyInt 0]             << chunkSetID
          [PyBool False]        << allowUnsubmitted
        */
    Call_GetFullFiles args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    PyTuple* response = new PyTuple(5);
    //  toBeChanged is populated with k,v of bulkFileID, CRowset
    PyDict* toBeChanged = new PyDict();
    // bulksEndingInChunk is populated when the last data of a file has been sent.
    //   each complete (or completed) data file's ID is put into this list.
    //   multiple files can be sent in this call, with their listIDs inserted into bulksEndingInChunk list.
    //  fileIDs here tell the client to save the file in it's cache
    PyList* bulksEndingInChunk = new PyList();  // bulksEndingIn(this)Chunk

    if (args.toGet->IsNone()) {
        // toGet = null.  this means get all bulkdata files
        toBeChanged->SetItem(new PyInt(800002), sBulkDB.GetBulkData(0));
        bulksEndingInChunk->AddItem(new PyInt(800002));
        toBeChanged->SetItem(new PyInt(800004), sBulkDB.GetBulkData(1));
        bulksEndingInChunk->AddItem(new PyInt(800004));
        toBeChanged->SetItem(new PyInt(800005), sBulkDB.GetBulkData(2));
        bulksEndingInChunk->AddItem(new PyInt(800005));
        // will have to determine what files are needed using hash, and then how to arrange and send this data correctly
        response->SetItem(2, new PyInt(sBulkDB.GetNumChunks()));    //numberOfChunks
        response->SetItem(3, PyStatic.NewZero());                   //chunkSetID
    } else if (args.toGet->IsList()) {
        PyList::const_iterator itr = args.toGet->AsList()->begin(), end = args.toGet->AsList()->end();
        uint8 setID(1);
        while (itr != end) {
            switch (PyRep::IntegerValueU32(*itr)) {
                case 800002: {
                    toBeChanged->SetItem(new PyInt(800002), sBulkDB.GetBulkData(0));
                    bulksEndingInChunk->AddItem(new PyInt(800002));
                } break;
                case 800004: {
                    toBeChanged->SetItem(new PyInt(800004), sBulkDB.GetBulkData(1));
                    bulksEndingInChunk->AddItem(new PyInt(800004));
                } break;
                case 800005: {
                    toBeChanged->SetItem(new PyInt(800005), sBulkDB.GetBulkData(2));
                    bulksEndingInChunk->AddItem(new PyInt(800005));
                } break;
                // these are hacked, but this whole system is...however, these *shouldnt* be called
                case 800003: {
                    setID = 2;
                    toBeChanged->SetItem(new PyInt(800003), sBulkDB.GetBulkDataChunks(0, 1));
                } break;
                case 800006: {
                    setID = 3;
                    toBeChanged->SetItem(new PyInt(800006), sBulkDB.GetBulkDataChunks(0, 7));
                } break;
                case 800007: {
                    setID = 4;
                    toBeChanged->SetItem(new PyInt(800007), sBulkDB.GetBulkDataChunks(0, 3));
                } break;
            }
            ++itr;
        }
        // will have to determine what files are needed, and how to arrange this data correctly
        response->SetItem(2, new PyInt(sBulkDB.GetNumChunks(setID)));   //numberOfChunks
        response->SetItem(3, new PyInt(setID));    //chunkSetID
    } else {
        _log(BULKDATA__ERROR, "BulkMgrService::Handle_GetFullFiles(): args.toGet->TypeString() is %s", args.toGet->TypeString());
        return PyStatic.NewNone();
    }

    response->SetItem(0, toBeChanged);

    //  if bulksEndingInChunk is empty, a PyNone is returned, stating this is only partial file data
    if (bulksEndingInChunk->size() > 0) {
        response->SetItem(1, bulksEndingInChunk);
    } else {
        response->SetItem(1, PyStatic.NewNone());
    }

    response->SetItem(4, PyStatic.NewFalse());     //allowUnsubmitted isnt supported (yet)

    if (is_log_enabled(BULKDATA__TRACE))
        response->Dump(BULKDATA__TRACE, "  ");

    return response;
}

PyResult BulkMgrService::Handle_GetFullFilesChunk(PyCallArgs &call)
{
    /*
    sLog.White( "BulkMgrService::Handle_GetFullFilesChunk()", "size=%li", call.tuple->size());
    call.Dump(BULKDATA__DUMP);
        toBeChanged, bulksEndingInChunk = self.bulkMgr.GetFullFilesChunk(chunkSetID, chunkNumber)
            this breaks files up into ?kb chunks for sending to client.  client requests "chunkSetID" and "chunkNumber", where chunkSetID is ???
     */
    Call_GetFullFilesChunk args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    PyTuple* response = new PyTuple(2);
    PyDict* toBeChanged = new PyDict();
    int32 bulkFileID = sBulkDB.GetFileIDfromChunk(args.chunkSetID, args.chunkNumber);
    if (bulkFileID < 0) {
        _log(BULKDATA__ERROR, "BulkMgrService::Handle_GetFullFilesChunk(): chunkSetID: %u, chunkNumber: %u, bulkFileID: %i", args.chunkSetID, args.chunkNumber, bulkFileID);
        // make and send client error also.  may be able to throw here.
        return PyStatic.NewNone();
    }

    _log(BULKDATA__INFO, "BulkMgrService::Handle_GetFullFilesChunk(): bulkFileID: %i, chunkSetID: %u, chunkNumber: %u", bulkFileID, args.chunkSetID, args.chunkNumber);
    toBeChanged->SetItem(new PyInt(bulkFileID), sBulkDB.GetBulkDataChunks(args.chunkSetID, args.chunkNumber));

    // 2, 4, 36
    if (args.chunkSetID == 0) {
        if (args.chunkNumber == 2) {
            PyList* bulksEndingInChunk = new PyList();
            bulksEndingInChunk->AddItem(new PyInt(bulkFileID));
            response->SetItem(1, bulksEndingInChunk);
        } else if (args.chunkNumber == 6) {
            PyList* bulksEndingInChunk = new PyList();
            bulksEndingInChunk->AddItem(new PyInt(bulkFileID));
            response->SetItem(1, bulksEndingInChunk);
        } else if (args.chunkNumber == 42) {
            PyList* bulksEndingInChunk = new PyList();
            bulksEndingInChunk->AddItem(new PyInt(bulkFileID));
            response->SetItem(1, bulksEndingInChunk);
        } else {
            response->SetItem(1, PyStatic.NewNone());
        }
    } else if (args.chunkSetID == 1) {

    } else if (args.chunkSetID == 2) {

    } else if (args.chunkSetID == 3) {

    }

    response->SetItem(0, toBeChanged);
    return response;
}

PyResult BulkMgrService::Handle_GetVersion(PyCallArgs &call)
{
    // changeID, branch = self.bulkMgr.GetVersion()
/*
    sLog.White( "BulkMgrService::Handle_GetVersion()", "size=%li", call.tuple->size());
    call.Dump(BULKDATA__DUMP);
*/
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(bulkDataChangeID));
        tuple->SetItem(1, new PyInt(bulkDataBranch));
    return tuple;
}

PyResult BulkMgrService::Handle_GetAllBulkIDs(PyCallArgs &call)
{
    /*
    sLog.White( "BulkMgrService::Handle_GetAllBulkIDs()", "size=%li", call.tuple->size());
    call.Dump(BULKDATA__DUMP);
     *    serverBulkIDs = self.bulkMgr.GetAllBulkIDs()
     *        PyList of fileIDs of updated data files to be sent to client in bulk
     */

    // hard-code a list of 'new' dgm fileIDs here. (updated and edited dogma data)
    // this can also be used to update other data files as needed
    PyList* list = new PyList();
        list->AddItem(new PyInt(800002));   //cacheDogmaOperands
        list->AddItem(new PyInt(800003));   //cacheDogmaExpressions
        list->AddItem(new PyInt(800004));   //cacheDogmaAttributes
        list->AddItem(new PyInt(800005));   //cacheDogmaEffects
        list->AddItem(new PyInt(800006));   //cacheDogmaTypeAttributes
        list->AddItem(new PyInt(800007));   //cacheDogmaTypeEffects
    return list;
}

PyResult BulkMgrService::Handle_GetChunk(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetChunk()", "size=%li", call.tuple->size());
    call.Dump(BULKDATA__DUMP);
    /*
     *    toBeChanged = self.bulkMgr.GetChunk(changeID, chunkNumber)
     *    changeID is from GetVersion()
     *    chunkNumber is incremented during loop when bulkdata return 'type' =  updateBulkStatusNeedToUpdate
     *    need more info to properly implement this
     */
    Call_GetChunk args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }
    /*
     *    args.changeID;
     *    args.chunkNumber;
     */
    return PyStatic.NewNone();
}

PyResult BulkMgrService::Handle_GetUnsubmittedChunk(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetUnsubmittedChunk()", "size=%li", call.tuple->size());
    call.Dump(BULKDATA__DUMP);
    /*
                toBeChanged = self.bulkMgr.GetUnsubmittedChunk(chunkNumber)
    need more info to properly implement this
     */
    Call_GetUnsubmittedChunk args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }
    //args.chunkNumber;

    return PyStatic.NewNone();
}

PyResult BulkMgrService::Handle_GetUnsubmittedChanges(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetUnsubmittedChanges()", "size=%li", call.tuple->size());
    call.Dump(BULKDATA__DUMP);
    /*
        unsubmitted = self.bulkMgr.GetUnsubmittedChanges()
        PyDict of 'toBeChanged','toBeDeleted','changedTablesKeys','chunkCount'
          this one is complicated.  will need work if we're allowing unsubmitted (whatever that means)
    need more info to properly implement this
     */
    return PyStatic.NewNone();
}
