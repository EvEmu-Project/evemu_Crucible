/**
 * @name BulkDB.cpp
 *   This file is for loading bulkdata from the db
 *   Copyright 2017  EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 *
 */


#include "cache/BulkDB.h"


BulkDB::BulkDB()
{
    m_bulkData.clear();
    m_bulkDataChunks.clear();

    // will need to make this dynamic at some point.
    //  dunno how yet.
    m_chunks = 43;
    m_loaded = false;
}

BulkDB::~BulkDB()
{
    //Close();
}

void BulkDB::Close()
{
    for (auto cur : m_bulkData)
        PyDecRef(cur.second);

    for (auto cur : m_bulkDataChunks)
        PyDecRef(cur.second);

    m_bulkData.clear();
    m_bulkDataChunks.clear();
    sLog.Warning("      BulkDataMgr", "Bulk Data Manager has been closed." );
}


/* updated files to send to client in bulkdata
 *
    dgmOperands = 800002,                -74
    dgmExpressions = 800003,             -17758
    dgmAttributes = 800004,              -1791
    dgmEffects = 800005,                 -2854
    dgmTypeAttributes = 800006,          -353290
    dgmTypeEffects = 800007,             -33777
    dgmUnits = 800009,                   -57
    invBlueprintTypes = 1200001          -3292      // may not need this one...no, data is updated thru server call
    actKeyTypes = 2001100001             -22
    jnlEntryTypeIDs = 2001100002         -63

    these are split in chunks of ~10k with hard-coded chunk numbers
    as such, current (hacked) system only allows for all-or-none bulkdata file sets
    code has been started for data 'sets' but isnt finished as i dont understand how to determine needed files by hash
    */

void BulkDB::Initialize()
{
    if (m_loaded)
        return;

    double start = GetTimeMSeconds();

    m_bulkData.insert(std::pair<uint8, PyRep*>(0, GetOperands()));
    m_bulkData.insert(std::pair<uint8, PyRep*>(1, GetDogmaAttribs()));
    m_bulkData.insert(std::pair<uint8, PyRep*>(2, GetDogmaEffects()));

    for (int i = 1; i < m_chunks; ++i) {
        switch (i) {
            case 1:
            case 2: {
                m_bulkDataChunks.insert(std::pair<uint8, PyRep*>(i, GetExpressions(i+1)));
            } break;
            case 3:
            case 4:
            case 5:
            case 6: {
                uint8 chunk = i - 2;  // 1 to 4
                m_bulkDataChunks.insert(std::pair<uint8, PyRep*>(i, GetDogmaTypeEffects(chunk)));
            } break;
            default: {
                uint8 chunk = i - 6;  // 1 to 36
                if (chunk > 36)
                    assert(true);  // make error here for wrong chunkID
                m_bulkDataChunks.insert(std::pair<uint8, PyRep*>(i, GetDogmaTypeAttribs(chunk)));
            } break;
        }
    }
    if (m_bulkDataChunks.size() > 0)
        m_loaded = true;

    sLog.Cyan("      BulkDataMgr", "%u BulkData Chunks loaded in %.3fms.", m_bulkDataChunks.size(), (GetTimeMSeconds() - start));
}

uint8 BulkDB::GetNumChunks(uint8 setID /*0*/)
{
    switch (setID) {
        case 0: {
            return m_chunks;
        }
        case 1: {
            return 1;
        }
        case 2: {   // dgmExpressions
            return 2;
        }
        case 3: {   // dgmTypeAttributes
            return 36;
        }
        case 4: {   // dgmTypeEffects
            return 4;
        }
    }

    return 0;
}
int32 BulkDB::GetFileIDfromChunk(uint8 setID, uint8 chunkID)
{
    if (chunkID > m_chunks) {
        // error.  caught in bulkMgr
        return -1;
    }
    switch (setID) {
        case 0: {
            switch (chunkID) {
                case 1:
                case 2: {
                    return 800003;  //dgmExpressions
                } break;
                case 3:
                case 4:
                case 5:
                case 6: {
                    return 800007;  //dgmTypeEffects
                } break;
                default: {
                    return 800006;  //dgmTypeAttributes
                } break;
            }
        } break;
        case 1: {
            return 800003;  //dgmExpressions
        } break;
        case 2: {
            return 800007;  //dgmTypeEffects
        } break;
        case 3: {
            return 800006;  //dgmTypeAttributes
        } break;
    }

    // error.  caught in bulkMgr
    return -1;
}

/** @todo  update this to use setIDs, and consolidate all data and calls */
PyRep* BulkDB::GetBulkData(uint8 chunkID)
{
    if (!m_loaded)
        Initialize();

    std::map<uint8, PyRep*>::const_iterator itr = m_bulkData.find(chunkID);
    if (itr != m_bulkData.end())
        return itr->second;
    return nullptr;
}

PyRep* BulkDB::GetBulkDataChunks(uint8 setID, uint8 chunkID)
{
    if (!m_loaded)
        Initialize();

    /** @todo  need to fix this for separate chunks vs sets */
    switch (setID) {
        case 0: {
            std::map<uint8, PyRep*>::const_iterator itr = m_bulkDataChunks.find(chunkID);
            if (itr != m_bulkDataChunks.end())
                return itr->second;
        } break;
        case 1: {
            return GetExpressions(chunkID);
        } break;
        case 2: {
            return GetDogmaTypeEffects(chunkID);
        } break;
        case 3: {
            return GetDogmaTypeAttribs(chunkID);
        } break;
    }

    // make error here.  should not reach this point.
    return nullptr;
}

PyRep* BulkDB::GetOperands()
{   //74
    DBQueryResult res;
    if ( !sDatabase.RunQuery(res,
        " SELECT operandID, operandKey, description, format, arg1categoryID, arg2categoryID, resultCategoryID, pythonFormat FROM dgmOperands"))
    {
        codelog(DATABASE__ERROR, "Error in GetOperands: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* BulkDB::GetDogmaAttribs()
{   //1791
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT attributeID, attributeName, attributeCategory, description, maxAttributeID, attributeIdx, "
        "chargeRechargeTimeID, defaultValue, published, unitID, displayName, displayNameID, stackable, highIsGood, iconID, dataID FROM dgmAttributeTypes"))
    {
        _log(DATABASE__ERROR, "Error in GetDogmaAttribs: %s",res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* BulkDB::GetDogmaEffects()
{   //2854
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT effectID, effectName, displayNameID, descriptionID, dataID, effectCategory, preExpression, postExpression,"
        " description, guid, isOffensive, isAssistance, durationAttributeID, trackingSpeedAttributeID, dischargeAttributeID,"
        " rangeAttributeID, falloffAttributeID, disallowAutoRepeat, published, displayName, isWarpSafe, rangeChance,"
        " electronicChance, propulsionChance, distribution, sfxName, npcUsageChanceAttributeID, "
        " npcActivationChanceAttributeID, fittingUsageChanceAttributeID, iconID, modifierInfo FROM dgmEffects"))
    {
        _log(DATABASE__ERROR, "Error in GetDogmaEffects: %s",res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
    /*  this doesnt work right.... AttributeError: EveConfig instance has no attribute 'dgmeffects'
    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        PyDict* dict = new PyDict();
        dict->SetItemString("effectID",                         new PyInt(row.GetInt(0)));
        dict->SetItemString("effectName",                       new PyString(row.GetText(1)));
        dict->SetItemString("displayNameID",                    new PyInt(row.GetInt(2)));
        dict->SetItemString("descriptionID",                    new PyInt(row.GetInt(3)));
        dict->SetItemString("dataID",                           new PyInt(row.GetInt(4)));
        dict->SetItemString("effectCategory",                   new PyInt(row.GetInt(5)));
        dict->SetItemString("preExpression",                    new PyInt(row.GetInt(6)));
        dict->SetItemString("postExpression",                   new PyInt(row.GetInt(7)));
        dict->SetItemString("description",                      new PyString(row.GetText(8)));
        dict->SetItemString("guid",                             new PyString(row.GetText(9)));
        dict->SetItemString("isOffensive",                      new PyBool(row.GetBool(10)));
        dict->SetItemString("isAssistance",                     new PyBool(row.GetBool(11)));
        dict->SetItemString("disallowAutoRepeat",               new PyBool(row.GetBool(17)));
        dict->SetItemString("published",                        new PyBool(row.GetBool(18)));
        dict->SetItemString("displayName",                      new PyString(row.GetText(19)));
        dict->SetItemString("isWarpSafe",                       new PyBool(row.GetBool(20)));
        dict->SetItemString("rangeChance",                      new PyBool(row.GetBool(21)));
        dict->SetItemString("electronicChance",                 new PyBool(row.GetBool(22)));
        dict->SetItemString("propulsionChance",                 new PyBool(row.GetBool(23)));
        dict->SetItemString("distribution",                     new PyInt(row.GetInt(24)));
        dict->SetItemString("sfxName",                          new PyString(row.GetText(25)));
        dict->SetItemString("npcUsageChanceAttributeID",        new PyInt(row.GetInt(26)));
        dict->SetItemString("npcActivationChanceAttributeID",   new PyInt(row.GetInt(27)));
        dict->SetItemString("fittingUsageChanceAttributeID",    new PyInt(row.GetInt(28)));
        dict->SetItemString("iconID",                           new PyInt(row.GetInt(29)));
        dict->SetItemString("modifierInfo",                     new PyString(row.GetText(30)));

        // these can be null
        if (row.IsNull(12))
            dict->SetItemString("durationAttributeID",          PyStatic.NewNone());
        else
            dict->SetItemString("durationAttributeID",          new PyInt(row.GetInt(12)));
        if (row.IsNull(13))
            dict->SetItemString("trackingSpeedAttributeID",     PyStatic.NewNone());
        else
            dict->SetItemString("trackingSpeedAttributeID",     new PyInt(row.GetInt(13)));
        if (row.IsNull(14))
            dict->SetItemString("dischargeAttributeID",         PyStatic.NewNone());
        else
            dict->SetItemString("dischargeAttributeID",         new PyInt(row.GetInt(14)));
        if (row.IsNull(15))
            dict->SetItemString("rangeAttributeID",             PyStatic.NewNone());
        else
            dict->SetItemString("rangeAttributeID",             new PyInt(row.GetInt(15)));
        if (row.IsNull(16))
            dict->SetItemString("falloffAttributeID",           PyStatic.NewNone());
        else
            dict->SetItemString("falloffAttributeID",           new PyInt(row.GetInt(16)));

        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    return list;
    */
}

PyRep* BulkDB::GetExpressions(uint8 chunkID)    // 2 chunks
{   //17758
    DBQueryResult res;
    std::ostringstream q;
    q << " SELECT expressionID, operandID, arg1, arg2, expressionValue, description, expressionName, expressionTypeID, expressionGroupID, expressionAttributeID";
    q << " FROM dgmExpressions";
    switch (chunkID) {
        case 1: {
            q << " LIMIT 0, 9000";
        } break;
        case 2: {
            q << " LIMIT 9000, 18000";
        } break;
    };

    if ( !sDatabase.RunQuery(res, q.str().c_str()))  // start at row 0 and get next 8k rows
    {
        codelog(DATABASE__ERROR, "Error in GetExpressions: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* BulkDB::GetDogmaTypeEffects(uint8 chunkID)   // 4 chunks
{   //33777
    DBQueryResult res;
    std::ostringstream q;
    q << "SELECT typeID,effectID,isDefault FROM dgmTypeEffects";
    switch (chunkID) {
        case 1: {
            q << " LIMIT 0, 9000";
        } break;
        case 2: {
            q << " LIMIT 9000, 9000";
        } break;
        case 3: {
            q << " LIMIT 18000, 9000";
        } break;
        case 4: {
            q << " LIMIT 27000, 9000";
        } break;
    };

    if (!sDatabase.RunQuery(res, q.str().c_str()))
    {
        _log(DATABASE__ERROR, "Error in GetDogmaTypeEffects: %s",res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* BulkDB::GetDogmaTypeAttribs(uint8 chunkID)   // 36 chunks
{   //353290
    DBQueryResult res;
    std::ostringstream q;
    q << "SELECT typeID, attributeID, IF(valueInt IS NULL, valueFloat, valueInt) AS value FROM dgmTypeAttributes";
    switch (chunkID) {
        case 1: {
            q << " LIMIT 0, 10000";
        } break;
        case 2: {
            q << " LIMIT 10000, 10000";
        } break;
        case 3: {
            q << " LIMIT 20000, 10000";
        } break;
        case 4: {
            q << " LIMIT 30000, 10000";
        } break;
        case 5: {
            q << " LIMIT 40000, 10000";
        } break;
        case 6: {
            q << " LIMIT 50000, 10000";
        } break;
        case 7: {
            q << " LIMIT 60000, 10000";
        } break;
        case 8: {
            q << " LIMIT 70000, 10000";
        } break;
        case 9: {
            q << " LIMIT 80000, 10000";
        } break;
        case 10: {
            q << " LIMIT 90000, 10000";
        } break;
        case 11: {
            q << " LIMIT 100000, 10000";
        } break;
        case 12: {
            q << " LIMIT 110000, 10000";
        } break;
        case 13: {
            q << " LIMIT 120000, 10000";
        } break;
        case 14: {
            q << " LIMIT 130000, 10000";
        } break;
        case 15: {
            q << " LIMIT 140000, 10000";
        } break;
        case 16: {
            q << " LIMIT 150000, 10000";
        } break;
        case 17: {
            q << " LIMIT 160000, 10000";
        } break;
        case 18: {
            q << " LIMIT 170000, 10000";
        } break;
        case 19: {
            q << " LIMIT 180000, 10000";
        } break;
        case 20: {
            q << " LIMIT 190000, 10000";
        } break;
        case 21: {
            q << " LIMIT 200000, 10000";
        } break;
        case 22: {
            q << " LIMIT 210000, 10000";
        } break;
        case 23: {
            q << " LIMIT 220000, 10000";
        } break;
        case 24: {
            q << " LIMIT 230000, 10000";
        } break;
        case 25: {
            q << " LIMIT 240000, 10000";
        } break;
        case 26: {
            q << " LIMIT 250000, 10000";
        } break;
        case 27: {
            q << " LIMIT 260000, 10000";
        } break;
        case 28: {
            q << " LIMIT 270000, 10000";
        } break;
        case 29: {
            q << " LIMIT 280000, 10000";
        } break;
        case 30: {
            q << " LIMIT 290000, 10000";
        } break;
        case 31: {
            q << " LIMIT 300000, 10000";
        } break;
        case 32: {
            q << " LIMIT 310000, 10000";
        } break;
        case 33: {
            q << " LIMIT 320000, 10000";
        } break;
        case 34: {
            q << " LIMIT 330000, 10000";
        } break;
        case 35: {
            q << " LIMIT 340000, 10000";
        } break;
        case 36: {
            q << " LIMIT 350000, 10000";
        } break;
    };

    if (!sDatabase.RunQuery(res, q.str().c_str()))
    {
        _log(DATABASE__ERROR, "Error in GetDogmaTypeAttribs: %s",res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}
