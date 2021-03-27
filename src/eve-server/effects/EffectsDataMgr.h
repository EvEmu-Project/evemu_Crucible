/**
 * @name EffectsDataMgr.h
 *   This file is for retrieving, manipulating and managing effect data
 *   Copyright 2017  EVEmu Team
 *
 * @Author:    Allan
 * @date:      29 January 2017
 *
 */


#ifndef _EVE_FX_PROC_DATAMGR_H__
#define _EVE_FX_PROC_DATAMGR_H__

#include "effects/EffectsData.h"

class FxDataMgr
: public Singleton< FxDataMgr >
{
public:
    FxDataMgr();
    ~FxDataMgr();

    int Initialize();
    void Populate();
    bool isWarpSafe(uint16 eID);
    bool isOffensive(uint16 eID);
    bool isAssistance(uint16 eID);
    uint16 GetEffectID(std::string effectName);
    std::string GetEffectGuid(uint16 eID);
    std::string GetEffectName(uint16 eID);

    Effect GetEffect(uint16 eID);
    Operand GetOperand(uint16 oID);
    Expression GetExpression(uint16 eID);

    void GetTypeEffect(uint16 typeID, std::vector< TypeEffects >& typeEffMap);

    float GetFxTime()                                   { return m_time; }
    uint16 GetFxSize()                                  { return m_fxMap.size(); }

protected:
    void GetOperands(DBQueryResult& res);
    void GetDgmEffects(DBQueryResult& res);
    void GetExpressions(DBQueryResult& res);
    void GetDgmTypeEffects(DBQueryResult &res);

private:
    bool m_loaded;
    float m_time;

    // data maps
    effectMapType m_fxMap;   // k,v of effID, data   -to search by effect

    effectMapType m_effectMap;  //std::map<uint16, Effect>
    std::map<uint16, Operand> m_opMap;
    std::map<uint16, Expression> m_expMap;
    std::map<std::string, uint16> m_effectName;  // k,v of effectID, effectName.  maps all effectIDs to their name.
    std::unordered_multimap<uint16, TypeEffects> m_typeFxMap;  // k,v of typeID, data<effectID, isDefault>
};

#define sFxDataMgr \
( FxDataMgr::get() )

#endif  // _EVE_FX_PROC_DATAMGR_H__