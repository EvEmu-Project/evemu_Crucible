/**
 * @name EffectsData.h
 *   This file is data containers used with EffectsProcessor (FxProc)
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 * @update:     19 June 2019 (revised namespaces)
 *
 */

#ifndef _EVE_FX_PROC_DATA_H__
#define _EVE_FX_PROC_DATA_H__

#include "../eve-server.h"


// POD for effect data in mem objects
struct Effect {
    bool isOffensive;
    bool isAssistance;
    bool disallowAutoRepeat;
    bool isWarpSafe;
    uint8 effectState;
    uint16 effectID;
    uint16 preExpression;
    uint16 postExpression;
    uint16 npcUsageChanceAttributeID;
    uint16 npcActivationChanceAttributeID;
    uint16 fittingUsageChanceAttributeID;
    uint16 durationAttributeID;
    uint16 trackingSpeedAttributeID;
    uint16 dischargeAttributeID;
    uint16 rangeAttributeID;
    uint16 falloffAttributeID;
    float rangeChance;
    float electronicChance;
    float propulsionChance;
    std::string effectName;
    std::string guid;
};

struct Expression {
    int8 operandID;
    uint16 id;
    uint16 arg1;
    uint16 arg2;
    uint16 expressionTypeID;
    uint16 expressionGroupID;
    uint16 expressionAttributeID;
    std::string expressionValue;
    std::string description;
    std::string expressionName;
};

struct Operand {
    uint8 resultCategoryID;
    uint16 arg1categoryID;
    uint16 arg2categoryID;
    std::string operandKey;
    std::string format;
};

struct TypeEffects {
    bool isDefault;     // used to calculate capacitor stability in fit window using dischargeAttributeID
    uint16 effectID;
};

struct fxData {
    int8 math;          // math used on data
    int8 fxSrc;         // effect source location
    int8 targLoc;       // effect target location
    uint8 action;       // effect *DOES* something (module action aside from modification)
    uint16 targAttr;
    uint16 srcAttr;
    uint16 grpID;       // used to define items in env grouped by item groupID
    uint16 typeID;      // used to define items in env grouped by skill requirement
    InventoryItemRef srcRef;   // source item ref, if required
};

typedef std::map<uint16, Effect> effectMapType;

// these tables are used to decode fields in Effects table
namespace FX {

    namespace Source {    // formally known as domain
        enum {
            Invalid          = -1,
            //  these define the location for group-, skill-, gang-, and owner-required effects
            Self             = 0,
            Skill            = 1,
            Ship             = 2,
            Owner            = 3,
            Gang             = 4,
            Group            = 5,
            Target           = 6
        };
    }

    namespace Target {    //formally known as environment
        enum {
            Invalid         = -1,
            // these define the item containing the attribute to be modified
            //  these are found (as text) in the expressionValue field of dgmExpressions table and may need to merge with Association, or test with it
            Self            = 0,
            Char            = 1,
            Ship            = 2,
            Target          = 3,
            Other           = 4,
            Area            = 5,
            PowerCore       = 6,  //defined but not used
            Charge          = 7,
            MaxTargLocation = 7
        };
    }

    namespace State {   // formally known as category
        enum {
            Invalid        = -1,
            // these are the effectCategory in dgmEffects table to denote when this effect is applied or removed
            Passive        = 0, //Applied when item is present and online - implants, skills, modules, charges
            Active         = 1, //Applied when active module is activated
            Target         = 2, //Applied onto selected target when module is activated
            Area           = 3, //defined but not used
            Online         = 4, //Applied when module is onlined
            Overloaded     = 5, //Applied when module is overloaded and activated
            Dungeon        = 6, //Dungeon effects, several effects exist in this category, but not assigned to any item  -passive
            System         = 7  //System-wide effects, like WH and incursion  -passive
        };
    }

    namespace Math {    // formally known as association
        enum {
            Invalid        = -1,
            // these define how the data is manipulated according to the format field in dgmOperands table
            PreAssignment  = 0,
            PreMul         = 1,
            PreDiv         = 2,
            ModAdd         = 3,
            ModSub         = 4,
            PostMul        = 5,
            PostDiv        = 6,
            PostPercent    = 7,
            PostAssignment = 8,
            SkillCheck     = 9,
            /* no data or expressions with the next two */
            AddRate        = 10,
            SubRate        = 11,
            RevPostPercent = 12,
            MaxMathMethod  = 12
        };
    }

    namespace Action {  // this are coded and are applied on the fly as needed.
        enum {
            Invalid                  = 0,
            ATTACK                   = 13,
            CARGOSCAN                = 14,
            CHEATTELEDOCK            = 15,
            CHEATTELEGATE            = 16,
            DECLOAKWAVE              = 19,
            ECMBURST                 = 30,
            EMPWAVE                  = 32,
            LAUNCH                   = 44,
            LAUNCHDEFENDERMISSILE    = 45,
            LAUNCHDRONE              = 46,
            LAUNCHFOFMISSILE         = 47,
            MINE                     = 50,
            POWERBOOST               = 53,   //effectID 48  - Consumes power booster charges to increase the available power in the capacitor.
            SHIPSCAN                 = 66,
            SURVEYSCAN               = 69,
            TARGETHOSTILES           = 70,
            TARGETSILENTLY           = 71,
            TOOLTARGETSKILLS         = 72,
            VERIFYTARGETGROUP        = 74,
            /* unique/special to EVEmu */
            SPEEDBOOST               = 75    //effectID 14  - prop mod to call destiny speed updates
        };
    }

    /*  old shit
             case CALC_NONE:                            return val1;
             case CALC_ADD:                             return val1 + val2;
             case CALC_SUBTRACT:                        return val1 - val2;
             case CALC_MULTIPLY:                        return val1 * val2;
             case CALC_DIVIDE:                          return ((val2 != 0) ? val1 / val2 : val1);
             case CALC_PERCENTAGE:                      return val1 * (1 + (val2 / 100));
             case CALC_REV_PERCENTAGE:                  return val1 / (1 + (val2 / 100));
             case CALC_ADD_PERCENT:                     return val1 + (val2 / 100);
             case CALC_SUBTRACT_PERCENT:                return val1 - (val2 / 100);
             case CALC_ADD_RESIST:                      return val1 - (1 - val2);
             case CALC_SUBTRACT_RESIST:                 return val1 + (1 - val2);
     */

    namespace Operands {
        enum {
            // @note  '//*' denotes implemented
            ADD = 1,             //*
            AGGM = 2,            //*
            AGSM = 3,            //*
            AGORSM = 4,          //*
            AGRSM = 5,           //*
            AIM = 6,             //*
            ALGM = 7,            //*
            ALM = 8,             //*
            ALRSM = 9,           //*
            AND = 10,            //*
            AORSM = 11,          //*
            ATT = 12,            //*
            ATTACK = 13,
            CARGOSCAN = 14,
            CHEATTELEDOCK = 15,
            CHEATTELEGATE = 16,
            COMBINE = 17,        //*
            DEC = 18,            //*
            DECLOAKWAVE = 19,
            DECN = 20,           //*
            DEFASSOCIATION = 21, //*
            DEFATTRIBUTE = 22,   //*
            DEFBOOL = 23,        //*
            DEFENVIDX = 24,      //*
            DEFFLOAT = 25,       //*
            DEFGROUP = 26,       //*
            DEFINT = 27,         //*
            DEFSTRING = 28,      //*
            DEFTYPEID = 29,      //*
            ECMBURST = 30,
            EFF = 31,            //*
            EMPWAVE = 32,
            EQ = 33,             //*
            GA = 34,             //*
            GET = 35,            //*
            GETTYPE = 36,        //*
            GM = 37,             //*
            GT = 38,             //*
            GTE = 39,            //*
            IA = 40,             //*
            IF = 41,             //*
            INC = 42,            //*
            INCN = 43,           //*
            LAUNCH = 44,
            LAUNCHDEFENDERMISSILE = 45,
            LAUNCHDRONE = 46,
            LAUNCHFOFMISSILE = 47,
            LG = 48,             //*
            SRLG = 49,           //*
            MINE = 50,
            MUL = 51,            //*
            OR = 52,             //*
            POWERBOOST = 53,
            RGGM = 54,           //*
            RGSM = 55,           //*
            RGORSM = 56,         //*
            RGRSM = 57,          //*
            RIM = 58,            //*
            RLGM = 59,           //*
            RLM = 60,            //*
            RLRSM = 61,          //*
            RORSM = 62,          //*
            RS = 63,             //*
            RSA = 64,            //*
            SET = 65,            //*
            SHIPSCAN = 66,
            SKILLCHECK = 67,     //*
            SUB = 68,            //*
            SURVEYSCAN = 69,
            TARGETHOSTILES = 70,
            TARGETSILENTLY = 71,
            TOOLTARGETSKILLS = 72,
            UE = 73,             //*
            VERIFYTARGETGROUP = 74,
            SPEEDBOOST = 75      //*
        };
    }
}


/*  not sure what these are...
 * dgmEffActivation = 1
 * dgmEffArea = 3
 * dgmEffOnline = 4
 * dgmEffPassive = 0
 * dgmEffTarget = 2
 * dgmEffOverload = 5
 * dgmEffDungeon = 6
 * dgmEffSystem = 7
 * dgmPassiveEffectCategories = (dgmEffPassive, dgmEffDungeon, dgmEffSystem)
 *
 *    dgmAttributesByIdx = {
 *        1: attributeIsOnline,
 *        2: attributeDamage,
 *        3: attributeCharge,
 *        4: attributeSkillPoints,
 *        5: attributeArmorDamage,
 *        6: attributeShieldCharge,
 *        7: attributeIsIncapacitated}
 *
 *    UserErrors
 *        UE_OWNERID = 2
 *        UE_LOCID = 3
 *        UE_TYPEID = 4
 *        UE_TYPEID2 = 5
 *        UE_TYPEIDL = 29
 *        UE_BPTYPEID = 6
 *        UE_GROUPID = 7
 *        UE_GROUPID2 = 8
 *        UE_CATID = 9
 *        UE_CATID2 = 10
 *        UE_DGMATTR = 11
 *        UE_DGMFX = 12
 *        UE_DGMTYPEFX = 13
 *        UE_AMT = 18
 *        UE_AMT2 = 19
 *        UE_AMT3 = 20
 *        UE_DIST = 21
 *        UE_TYPEIDANDQUANTITY = 24
 *        UE_OWNERIDNICK = 25
 *        UE_ISK = 28
 *        UE_AUR = 30
 */
/*  flags for ???
 *    dgmExprSkip = 0
 *    dgmExprOwner = 1
 *    dgmExprShip = 2
 *    dgmExprOwnerAndShip = 3
 */

/* these are the operandID field in the dgmExpressions table
 *
 *  will need to hand-write code for these operands.
 * (operandID, operandKey, description, format, arg1categoryID, arg2categoryID, resultCategoryID, pythonFormat)
 *
 * (1, 'ADD', 'add two numbers', '(%(arg1)s)+(%(arg2)s)', 4, 4, 4, '(%(arg1)s)+(%(arg2)s)')
 * (2, 'AGGM', 'add gang group modifier', '[%(arg1)s].AGGM(%(arg2)s)', 5, 2, 4, 'dogma.AddGangGroupModifier(env,%(arg1)s, %(arg2)s)')
 * (3, 'AGIM', 'add gang ship modifier', '[%(arg1)s].AGIM(%(arg2)s)', 5, 2, 4, 'dogma.AddGangShipModifier(env,%(arg1)s, %(arg2)s)')
 * (4, 'AGORSM', 'add gang owner required skill modifier', '[%(arg1)s].AGORSM(%(arg2)s)', 5, 2, 4, 'dogma.AddGangOwnerRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (5, 'AGRSM', 'add gang required skill modifier', '[%(arg1)s].AGRSM(%(arg2)s)', 5, 2, 4, 'dogma.AddGangRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (6, 'AIM', 'add item modifier', '(%(arg1)s).AddItemModifier (%(arg2)s)', 5, 2, 4, 'dogma.AddItemModifier(env,%(arg1)s, %(arg2)s)')
 * (7, 'ALGM', 'add location group modifier', '(%(arg1)s).AddLocationGroupModifier (%(arg2)s)', 5, 2, 4, 'dogma.AddLocationGroupModifier(env,%(arg1)s, %(arg2)s)')
 * (8, 'ALM', 'add location modifier', '(%(arg1)s).AddLocationModifier (%(arg2)s)', 5, 2, 4, 'dogma.AddLocationModifier(env,%(arg1)s, %(arg2)s)')
 * (9, 'ALRSM', 'add location required skill modifier', '(%(arg1)s).ALRSM(%(arg2)s)', 5, 2, 4, 'dogma.AddLocationRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (10, 'AND', 'logical and operation', '(%(arg1)s) AND (%(arg2)s)', 4, 4, 4, '(%(arg1)s and %(arg2)s)')
 * (11, 'AORSM', 'add owner required skill modifier', '(%(arg1)s).AORSM(%(arg2)s)', 5, 2, 4, 'dogma.AddOwnerRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (12, 'ATT', 'attribute', '%(arg1)s->%(arg2)s', 6, 2, 3, '(%(arg1)s, %(arg2)s)')
 * (13, 'ATTACK', 'attack given ship', 'Attack', 2, 0, 4, 'Attack(env, %(arg1)s, %(arg2)s)')
 * (14, 'CARGOSCAN', 'Scans the cargo of the targeted ship.', 'CargoScan', 0, 0, 4, 'CargoScan(env, None, None)')
 * (15, 'CHEATTELEDOCK', 'Instantly enter a station.', 'CheatTeleDock()', 0, 0, 4, 'CheatTeleDock(env, None, None)')
 * (16, 'CHEATTELEGATE', 'Automatically invoke a stargate destination from remote distances.', 'CheatTeleGate()', 0, 0, 4, 'dogma.CheatTeleGate(env, None, None)')
 * (17, 'COMBINE', 'executes two statements', '%(arg1)s);     (%(arg2)s', 4, 4, 4, '%(arg1)s %(arg2)s')
 * (18, 'DEC', 'decreases an item-attribute by the value of another attribute', '%(arg1)s-=self.%(arg2)s', 3, 2, 4, ' ')
 * (19, 'DECLOAKWAVE', 'broadcasts a decloaking wave', 'DecloakWave', 0, 0, 4, 'DecloakWave(env, None, None)')
 * (20, 'DECN', 'decreases an item-attribute by number', '%(arg1)s-=%(arg2)s', 3, 4, 4, ' ')
 * (21, 'DEFASSOCIATION', 'define attribute association type', '%(value)s', 0, 0, 1, 'const.dgmMath%(value)s')
 * (22, 'DEFATTRIBUTE', 'define attribute', '%(value)s', 0, 0, 2, '%(value)s')
 * (23, 'DEFBOOL', 'define bool constant', 'Bool(%(value)s)', 0, 0, 4, '%(value)s')
 * (24, 'DEFENVIDX', 'define environment index', 'Current%(value)s', 0, 0, 6, 'env[const.dgmTargLoc%(value)s]')
 * (25, 'DEFFLOAT', 'defines a float constant', 'Float(%(value)s)', 0, 0, 4, ' ')
 * (26, 'DEFGROUP', 'define group', '%(value)s', 0, 0, 8, ' ')
 * (27, 'DEFINT', 'defines an int constant', 'Int(%(value)s)', 0, 0, 4, '%(value)s')
 * (28, 'DEFSTRING', 'defines a string constant', '\"%(value)s\"', 0, 0, 4, '\"%(value)s\"')
 * (29, 'DEFTYPEID', 'define a type ID', 'Type(%(value)s)', 0, 0, 9, ' ')
 * (30, 'ECMBURST', 'Clears all targets on all ships(excluding self) wihin range. ', 'ECMBurst()', 0, 0, 4, 'dogma.ECMBurst(env, None, None)')
 * (31, 'EFF', 'define association type', '(%(arg2)s).(%(arg1)s)', 1, 3, 5, '(%(arg1)s, %(arg2)s)')
 * (32, 'EMPWAVE', 'broadcasts an EMP wave', 'EMPWave', 0, 0, 4, 'EMPWave(env, None, None)')
 * (33, 'EQ', 'checks for equality', '%(arg1)s == %(arg2)s', 4, 4, 4, '(%(arg1)s == %(arg2)s)')
 * (34, 'GA', 'attribute on a module group', '%(arg1)s.%(arg2)s', 8, 2, 3, '(%(arg1)s, %(arg2)s)')
 * (35, 'GET', 'calculate attribute', '%(arg1)s.%(arg2)s()', 6, 2, 4, 'dogmaLM.GetAttributeValue(%(arg1)s, %(arg2)s)')
 * (36, 'GETTYPE', 'gets type of item', '%(arg1)s.GetTypeID()', 6, 0, 9, 'env.itemTypeID')
 * (37, 'GM', 'get a module of a given groupID from a given location (ship or player)', '%(arg1)s.%(arg2)s', 6, 8, 6, 'dogma.GetModule(env,%(arg1)s,%(arg2)s)')
 * (38, 'GT', 'checks whether expression 1  is greater than expression 2', '%(arg1)s> %(arg2)s', 4, 4, 4, '(%(arg1)s > %(arg2)s)')
 * (39, 'GTE', 'checks whether an expression is greater than or equal to another', '%(arg1)s>=%(arg2)s', 4, 4, 4, '(%(arg1)s >= %(arg2)s)')
 * (40, 'IA', 'generic attribute', '%(arg1)s', 2, 0, 3, '%(arg1)s')
 * (41, 'IF', 'if construct', 'If(%(arg1)s), Then (%(arg2)s)', 4, 4, 4, 'if %(arg1)s:%(arg2)s')
 * (42, 'INC', 'increases an item-attribute by the value of another attribute', '%(arg1)s+=self.%(arg2)s', 3, 2, 4, '<handled in code>')
 * (43, 'INCN', 'increases an item-attribute by a number', '%(arg1)s+=%(arg2)s', 3, 4, 4, ' ')
 * (44, 'LAUNCH', 'launches a missile', 'LaunchMissile()', 0, 0, 4, 'dogma.Launch(env, None, None)')
 * (45, 'LAUNCHDEFENDERMISSILE', 'launches a defender missile', 'LaunchDefenderMissile()', 0, 0, 4, 'dogma.LaunchDefenderMissile(env, None, None)')
 * (46, 'LAUNCHDRONE', 'launches a drone.', 'LaunchDrone()', 0, 0, 4, 'dogma.Launch(env, None, None)')
 * (47, 'LAUNCHFOFMISSILE', 'launches an FOF missile', 'LaunchFOFMissile()', 0, 0, 4, 'dogma.LaunchFOFMissile(env, None, None)')
 * (48, 'LG', 'specify a group in a location', '%(arg1)s..%(arg2)s', 6, 8, 6, '(%(arg1)s, %(arg2)s)')
 * (49, 'LS', 'location - skill required item group', '%(arg1)s[%(arg2)s]', 6, 9, 6, '(%(arg1)s, %(arg2)s)')
 * (50, 'MINE', 'mines an asteroid', 'Mine', 0, 0, 4, 'Mine(env, None, None)')
 * (51, 'MUL', 'multiplies two numbers', '(%(arg1)s)*(%(arg2)s)', 4, 4, 4, '(%(arg1)s * %(arg2)s)')
 * (52, 'OR', 'logical or operation', '%(arg1)s OR %(arg2)s', 4, 4, 4, '(%(arg1)s or %(arg2)s)')
 * (53, 'POWERBOOST', '', 'PowerBoost', 0, 0, 4, 'dogma.PowerBoost(env, None, None)')
 * (54, 'RGGM', 'remove gang group modifier', '[%(arg1)s].RGGM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveGangGroupModifier(env,%(arg1)s, %(arg2)s)')
 * (55, 'RGIM', 'remove gang ship modifier', '[%(arg1)s].RGIM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveGangShipModifier(env,%(arg1)s, %(arg2)s)')
 * (56, 'RGORSM', 'remove a gang owner required skill modifier', '[%(arg1)s].RGORSM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveGangOwnerRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (57, 'RGRSM', 'remove a gang required skill modifier', '[%(arg1)s].RGRSM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveGangRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (58, 'RIM', 'remove  item modifier', '(%(arg1)s).RemoveItemModifier (%(arg2)s)', 5, 2, 4, 'dogma.RemoveItemModifier(env,%(arg1)s, %(arg2)s)')
 * (59, 'RLGM', 'remove location group modifier', '(%(arg1)s).RemoveLocationGroupModifier (%(arg2)s)', 5, 2, 4, 'dogma.RemoveLocationGroupModifier(env,%(arg1)s, %(arg2)s)')
 * (60, 'RLM', 'remove location modifier', '(%(arg1)s).RemoveLocationModifier (%(arg2)s)', 5, 2, 4, 'dogma.RemoveLocationModifier(env,%(arg1)s, %(arg2)s)')
 * (61, 'RLRSM', 'remove a  required skill modifier', '(%(arg1)s).RLRSM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveLocationRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (62, 'RORSM', 'remove an owner required skill modifier', '(%(arg1)s).RORSM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveOwnerRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (63, 'RS', 'true if arg1 requires arg2', '%(arg1)s.Requires(%(arg2)s)', 6, 9, 4, 'dogma.RequireSkill(env, %(arg1)s, %(arg2)s)')
 * (64, 'RSA', 'attribute on modules that have required skill', '%(arg1)s.%(arg2)s', 9, 2, 3, '(%(arg1)s, %(arg2)s)')
 * (65, 'SET', 'sets an item attribute', '%(arg1)s := %(arg2)s', 3, 4, 4, '<handled in code>')
 * (66, 'SHIPSCAN', 'scans a ship', 'ShipScan()', 0, 0, 4, 'ShipScan(env, None, None)')
 * (67, 'SKILLCHECK', '', 'SkillCheck(%(arg1)s)', 4, 0, 4, 'dogma.SkillCheck(env, %(arg1)s, %(arg2)s)')
 * (68, 'SUB', 'subtracts a number from another one', '%(arg1)s-%(arg2)s', 4, 4, 4, ' ')
 * (69, 'SURVEYSCAN', 'scans an asteroid for information', 'SurveyScan()', 0, 0, 4, 'SurveyScan(env, None, None)')
 * (70, 'TARGETHOSTILES', 'Targets any hostile ships within range (assuming electronics have capability).', 'TargetHostiles()', 0, 0, 4, 'dogma.TargetHostiles(env, None, None)')
 * (71, 'TARGETSILENTLY', '', 'TargetSilently()', 0, 0, 4, 'dogmaLM.AddTargetEx(shipID,targetID, silent=1, tasklet=1)')
 * (72, 'TOOLTARGETSKILLS', ' ', 'CheckToolTargetSkills', 0, 0, 4, 'dogma.CheckToolTargetSkills(env,None,None)')
 * (73, 'UE', 'raises an user error', 'UserError(%(arg1)s)', 4, 0, 4, 'raise UserError(%(arg1)s)')
 * (74, 'VERIFYTARGETGROUP', 'raises a user error if incorrect target group', 'VerifyTargetGroup()', 0, 0, 4, 'dogma.VerifyTargetGroup(env, None, None)');
 */

#endif  // _EVE_FX_PROC_DATA_H__
