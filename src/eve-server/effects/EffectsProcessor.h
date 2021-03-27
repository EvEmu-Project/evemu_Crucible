/**
 * @name EffectsProcessor.h
 *   This file is for decoding and processing the effect data
 *   Copyright 2017  EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 *
 */


#ifndef _EVE_FX_PROC_H__
#define _EVE_FX_PROC_H__

#include "effects/EffectsDataMgr.h"
#include "inventory/InventoryItem.h"

class Character;
class GenericModule;
class ShipItem;

class FxProc
: public Singleton<FxProc>
{
public:
    FxProc()        { /* do nothing here */ }
    ~FxProc()       { /* do nothing here */ }

    // pItem is modifier container
    void            ApplyEffects(InventoryItem* pItem, Character* pChar, ShipItem* pShip, bool update=false);
    // pItem is modifier container
    void            ParseExpression(InventoryItem* pItem, Expression expression, fxData& data, GenericModule* pMod=nullptr);
    int8            GetEnvironmentEnum(const std::string& domain);
    int8            GetAssociationEnum(const std::string& association);

    const char*     GetSourceName(int8 id);
    const char*     GetMathMethodName(int8 id);
    const char*     GetTargLocName(int8 id);
    const char*     GetStateName(int8 id);

    EvilNumber      CalculateAttributeValue(EvilNumber val1, EvilNumber val2, /*FX::Math*/int8 method);

    void DecodeEffects(const uint16 fxID);
protected:
    void EvaluateExpression(const uint16 expID, const char* type);
    void DecodeExpression(Expression expression, fxData& data);

private:

};

#define sFxProc \
( FxProc::get() )

#endif  // _EVE_FX_PROC_H__

/** @todo  list of possible improvements
 * populate data.typeID with the skillID of skills that affect modules on location(ship)  (better conditional)
 *
 *
 *
 */

/*
            if operand.operandID in (
             const.operandALRSM,
             const.operandRLRSM,
             const.operandALGM,
             const.operandRLGM,
             const.operandAORSM,
             const.operandRORSM):
                return 'dogmaLM.%s(%s, %s, %s, %s, itemID, %s)' % (funcName,
                 arg1[0],
                 arg1[1][0][0],
                 arg1[1][0][1],
                 arg1[1][1],
                 arg2)
            if operand.operandID in (const.operandAGRSM, const.operandRGRSM):
                return 'dogmaLM.%s(shipID, %s, %s, %s, itemID, %s)' % (funcName,
                 arg1[0],
                 arg1[1][0],
                 arg1[1][1],
                 arg2)
            if operand.operandID in (const.operandAGIM, const.operandRGIM):
                return 'dogmaLM.%s(shipID, %s, %s, itemID, %s)' % (funcName,
                 arg1[0],
                 arg1[1],
                 arg2)
            if operand.operandID in (const.operandALM, const.operandRLM):
                return 'dogmaLM.%s(%s, %s, %s, itemID, %s)' % (funcName,
                 arg1[0],
                 arg1[1][0],
                 arg1[1][1],
                 arg2)

*/

/*
    def AddOwnerRequiredSkillModifier(self, env, arg1, arg2):
        affectingModuleID = env.itemID
        affectingAttributeID = arg2
        affectedOwnerID = arg1[1][0][0]
        affectedSkillID = arg1[1][0][1]
        affectedAttributeID = arg1[1][1]
        affectType = arg1[0]

    def AddLocationRequiredSkillModifier(self, env, arg1, arg2):
        affectingModuleID = env.itemID
        affectingAttributeID = arg2
        affectedLocationID = arg1[1][0][0]
        affectedSkillID = arg1[1][0][1]
        affectedAttributeID = arg1[1][1]
        affectType = arg1[0]

    def AddGangRequiredSkillModifier(self, env, arg1, arg2):
        affectingModuleID = env.itemID
        affectingAttributeID = arg2
        affectingCharID = env.charID
        affectingShipID = env.shipID
        affectedSkillID = arg1[1][0]
        affectedAttributeID = arg1[1][1]
        affectType = arg1[0]

    def AddGangGroupModifier(self, env, arg1, arg2):
        affectingModuleID = env.itemID
        affectingAttributeID = arg2
        affectingCharID = env.charID
        affectingShipID = env.shipID
        affectedGroupID = arg1[1][0]
        affectedAttributeID = arg1[1][1]
        affectType = arg1[0]

    def AddGangShipModifier(self, env, arg1, arg2):
        affectingModuleID = env.itemID
        affectingAttributeID = arg2
        affectingCharID = env.charID
        affectingShipID = env.shipID
        affectedAttributeID = arg1[1]
        affectType = arg1[0]

    def AddItemModifier(self, env, arg1, arg2):
        affectingModuleID = env.itemID
        if not arg2:
            raise RuntimeError('Expression is wrong.  AIM(%s, %s) - NULL/None value is not allowed' % (arg1, arg2))
        affectingAttributeID = arg2
        affectedModuleID = arg1[1][0]
        affectedAttributeID = arg1[1][1]
        affectType = arg1[0]
        if affectedModuleID == 0:
            return 1
        if affectedModuleID is None:
            self.LogWarn('AffectedModuleID is None.  Env:', env)
            return

    def AddLocationModifier(self, env, arg1, arg2):
        affectingModuleID = env.itemID
        affectingAttributeID = arg2
        affectedLocationID = arg1[1][0]
        affectedAttributeID = arg1[1][1]
        affectType = arg1[0]

    def AddLocationGroupModifier(self, env, arg1, arg2):
        affectingModuleID = env.itemID
        affectingAttributeID = arg2
        affectedLocationID = arg1[1][0][0]
        affectedGroupID = arg1[1][0][1]
        affectedAttributeID = arg1[1][1]
        affectType = arg1[0]

*/