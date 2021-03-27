
 /**
  * @name EVE_Damage.h
  *     Damage Notifications and killmail notes for EVEmu
  *
  * @Author:        Allan
  * @date:          2 January 2018
  *
  */


#ifndef EVE_DAMAGE_H
#define EVE_DAMAGE_H


/*  these are based on client settings of damage notification.
 * msg packets are
 *   " "  - to others
 *   "R"  - received
 *   "RD  - received with details
 */

namespace Dmg {
    namespace Msg {

        static const char* Given[9] = {
            "AttackMiss1",   //misses completely
            "AttackMiss2",   //barely misses
            "AttackMiss3",   //glances off
            "AttackHit1",    //barely scratches
            "AttackHit2",    //lightly hits
            "AttackHit3",    //hits
            "AttackHit4",    //aims well
            "AttackHit5",    //places an excellent hit
            "AttackHit6"     //strikes perfectly, wrecking
        };

        static const char* Taken[9] = {
            "AttackMiss1R",   //misses completely
            "AttackMiss2R",   //barely misses
            "AttackMiss3R",   //glances off
            "AttackHit1R",    //barely scratches
            "AttackHit2R",    //lightly hits
            "AttackHit3R",    //hits
            "AttackHit4R",    //aims well at you
            "AttackHit5R",    //places an excellent hit
            "AttackHit6R"     //strikes you perfectly, wrecking
        };

        static const char* Detail[9] = {
            "AttackMiss1RD",   //misses completely
            "AttackMiss2RD",   //barely misses
            "AttackMiss3RD",   //glances off
            "AttackHit1RD",    //barely scratches
            "AttackHit2RD",    //lightly hits
            "AttackHit3RD",    //hits
            "AttackHit4RD",    //aims well at you
            "AttackHit5RD",    //places an excellent hit
            "AttackHit6RD"     //strikes you perfectly, wrecking
        };

        //'banked' means 'grouped weapons' and is for shooter only.
        // also in ship.modules.GetTurretSets in MakeSlimItem()
        static const char* Banked[9] = {
            "AttackMiss1Banked",   //misses completely
            "AttackMiss2Banked",   //barely misses
            "AttackMiss3Banked",   //glances off
            "AttackHit1Banked",    //barely scratches
            "AttackHit2Banked",    //lightly hits
            "AttackHit3Banked",    //hits
            "AttackHit4Banked",    //aims well
            "AttackHit5Banked",    //places an excellent hit
            "AttackHit6Banked"     //strikes perfectly, wrecking
        };

    }
}

/*
 * {'messageID': 260410, 'label': u'AttackMiss1Body'}(u'Your {[item]weapon.name} misses {target} completely.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'messageID': 260413, 'label': u'AttackMiss2Body'}(u'Your {[item]weapon.name} barely misses {target}.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'messageID': 260418, 'label': u'AttackMiss3Body'}(u'Your {[item]weapon.name} glances off {target}, causing no real damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'messageID': 260383, 'label': u'AttackHit1Body'}(u'<color=0xffbbbb00>Your {[item]weapon.name} barely scratches {target}, causing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260387, 'label': u'AttackHit2Body'}(u'<color=0xffbbbb00>Your {[item]weapon.name} lightly hits {target}, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260391, 'label': u'AttackHit3Body'}(u'<color=0xffbbbb00>Your {[item]weapon.name} hits {target}, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260394, 'label': u'AttackHit4Body'}(u'<color=0xffbbbb00>Your {[item]weapon.name} is well aimed at {target}, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260397, 'label': u'AttackHit5Body'}(u'<color=0xffbbbb00>Your {[item]weapon.name} places an excellent hit on {target}, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260400, 'label': u'AttackHit6Body'}(u'<color=0xffbbbb00>Your {[item]weapon.name} perfectly strikes {target}, wrecking for {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})

 * {'messageID': 260411, 'label': u'AttackMiss1RBody'}(u'{source} misses you completely.', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}})
 * {'messageID': 260414, 'label': u'AttackMiss2RBody'}(u'{source} barely misses you.', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}})
 * {'messageID': 260419, 'label': u'AttackMiss3RBody'}(u'{source} lands a hit on you which glances off, causing no real damage.', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}})
 * {'messageID': 260385, 'label': u'AttackHit1RBody'}(u'<color=0xffbb6600>{source} barely scratches you, causing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260388, 'label': u'AttackHit2RBody'}(u'<color=0xffbb6600>{source} lightly hits you, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260392, 'label': u'AttackHit3RBody'}(u'<color=0xffbb6600>{source} hits you, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260395, 'label': u'AttackHit4RBody'}(u'<color=0xffbb6600>{source} aims well at you, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260398, 'label': u'AttackHit5RBody'}(u'<color=0xffbb6600>{source} places an excellent hit on you, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260401, 'label': u'AttackHit6RBody'}(u'<color=0xffbb6600>{source} strikes you perfectly, wrecking for {[numeric]damage, decimalPlaces=1} damage.', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})

 * {'messageID': 260412, 'label': u'AttackMiss1RDBody'}(u'{[item]weapon.name} belonging to {owner} misses you completely.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'messageID': 260417, 'label': u'AttackMiss2RDBody'}(u'{[item]weapon.name} belonging to {owner} barely misses you.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'messageID': 260420, 'label': u'AttackMiss3RDBody'}(u'{[item]weapon.name} belonging to {owner} lands a hit on you which glances off, causing no real damage.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'messageID': 260386, 'label': u'AttackHit1RDBody'}(u'<color=0xffbb6600>{[item]weapon.name} belonging to {owner} barely scratches you, causing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260390, 'label': u'AttackHit2RDBody'}(u'<color=0xffbb6600>{[item]weapon.name} belonging to {owner} lightly hits you, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260393, 'label': u'AttackHit3RDBody'}(u'<color=0xffbb6600>{[item]weapon.name} belonging to {owner} hits you, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260396, 'label': u'AttackHit4RDBody'}(u'<color=0xffbb6600>{[item]weapon.name} belonging to {owner} heavily hits you, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260399, 'label': u'AttackHit5RDBody'}(u'<color=0xffbb6600>{[item]weapon.name} belonging to {owner} places an excellent hit on you, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 260402, 'label': u'AttackHit6RDBody'}(u'<color=0xffbb6600>{[item]weapon.name} belonging to {owner} strikes you perfectly, wrecking for {[numeric]damage, decimalPlaces=1} damage.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})

 * {'messageID': 257671, 'label': u'AttackMiss1BankedBody'}(u'Your group of {[item]weapon.name} misses {target} completely.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'messageID': 257677, 'label': u'AttackMiss2BankedBody'}(u'Your group of {[item]weapon.name} barely misses {target}.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'messageID': 257678, 'label': u'AttackMiss3BankedBody'}(u'Your group of {[item]weapon.name} glances off {target}, causing no real damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'messageID': 257589, 'label': u'AttackHit1BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} barely scratches {target}, causing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 257590, 'label': u'AttackHit2BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} lightly hits {target}, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 257591, 'label': u'AttackHit3BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} hits {target}, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 257592, 'label': u'AttackHit4BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} is well aimed at {target}, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 257593, 'label': u'AttackHit5BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} places an excellent hit on {target}, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'messageID': 257594, 'label': u'AttackHit6BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} perfectly strikes {target}, wrecking for {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 */

/*
 *    def OnDamageMessage(self, msgKey, args):
 *        try:
 *            michelle = sm.GetService('michelle')
 *            hitQuality = 0
 *            if msgKey.startswith('AttackHit'):
 *                try:
 *                    hitQuality = int(msgKey[9])
 *                except ValueError as e:
 *                    hitQuality = 5
 *
 *            banked = msgKey.find('Banked') > 0
 *            if args.has_key('target'):
 *                ship = michelle.GetBall(session.shipid)
 *                if ship and hasattr(ship, 'turrets'):
 *                    self.DamageMessageTurretPropogation(session.shipid, ship, hitQuality, args, banked)
 *            elif args.has_key('owner'):
 *                ownerData, ownerID = args['owner']
 *                targetSvc = sm.GetService('target')
 *                ship = None
 *                shipID = targetSvc.ownerToShipIDCache.get(ownerID, None)
 *                if shipID is not None:
 *                    ship = michelle.GetBall(shipID)
 *                else:
 *                    self.LogWarn('Turret hit/miss - failed to find shipID for ownerID ' + str(ownerID))
 *                if shipID and ship:
 *                    self.DamageMessageTurretPropogation(shipID, ship, hitQuality, args, banked)
 *            elif args.has_key('source'):
 *                shipID = args['source']
 *                ship = michelle.GetBall(shipID)
 *                if hasattr(ship, 'turrets'):
 *                    self.DamageMessageTurretPropogation(shipID, ship, hitQuality, args, banked)
 *                elif hasattr(ship, 'GetModel') and hasattr(ship.GetModel(), 'turretSets'):
 *                    self.DamageMessageGrouped(ship.GetModel().turretSets, hitQuality)
 *        except Exception as e:
 *            self.LogError('Error setting hit/miss state on a turret: %s %s' % (msgKey, str(args)))
 *            self.LogError(str(e) + ' ' + str(e.args))
 *
 *        if not settings.user.ui.Get('damageMessages', 1):
 *            return
 *        if args.has_key('damage') and args['damage'] == 0.0 and not settings.user.ui.Get('damageMessagesNoDamage', 1):
 *            return
 *        if args.has_key('owner') or args.has_key('source'):
 *            sm.ScatterEvent('OnShipDamage', args.get('damageTypes', {}))
 *            if not settings.user.ui.Get('damageMessagesEnemy', 1):
 *                return
 *        elif not settings.user.ui.Get('damageMessagesMine', 1):
 *            return
 *        if settings.user.ui.Get('damageMessagesSimple', 0):
 *            msgKey += '_Simple'
 *        for argName in ('source', 'target'):
 *            if args.has_key(argName):
 *                bracketName = sm.GetService('bracket').GetBracketName(args[argName])
 *                if not len(bracketName):
 *                    ballpark = sm.GetService('michelle').GetBallpark()
 *                    if ballpark is None:
 *                        self.LogWarn('OnDamageMessage: No ballpark, not showing damage message.')
 *                        return
 *                    slimItem = ballpark.GetInvItem(args[argName])
 *                    if slimItem:
 *                        bracketName = uix.GetSlimItemName(slimItem) or 'Some object'
 *                if not len(bracketName):
 *                    self.LogError('Failed to display message', msgKey, args)
 *                    return
 *                args[argName] = bracketName
 *
 *        msgKey = msgKey.replace('BankedR', 'R')
 *        sm.GetService('logger').AddCombatMessage(msgKey, args)
 *
 */

/*  killmail shit.  not sure how to implement it yet.
(251144, `<br>
Victim: {[character] victim.name}<br>
Corp: {corporation}<br>
Alliance: {alliance}<br>
Faction: {faction}<br>
Destroyed: {[item] target.name}<br>
System: {[location] system.name}<br>
Security: {[numeric] security, decimalPlaces=1}<br>
Damage Taken: {[numeric] damage, decimalPlaces=0}<br>
<br>
Involved parties:<br>
<br>`)
(251145, `<br>
Corp: {corporation}<br>
Alliance: {alliance}<br>
Faction: {faction}<br>
Destroyed: {[item] target.name}<br>
System: {[location] system.name}<br>
Security: {[numeric] security, decimalPlaces=1}<br>
Damage Taken: {[numeric] damage, decimalPlaces=0}<br>
<br>
Involved parties:<br>
<br>`)
(251147, `Name: {attacker} (laid the final blow)<br>
Security: {[numeric] security, decimalPlaces=2}<br>
Corp: {corporation}<br>
Alliance: {alliance}<br>
Faction: {faction}<br>
Ship: {ship}<br>
Weapon: {weapon}<br>
Damage Done: {[numeric] damage, decimalPlaces=0}<br>
<br>`)
(251148, `Name: {attacker}<br>
Security: {[numeric] security, decimalPlaces=2}<br>
Corp: {corporation}<br>
Alliance: {alliance}<br>
Faction: {faction}<br>
Ship: {ship}<br>
Weapon: {weapon}<br>
Damage Done: {[numeric] damage, decimalPlaces=0}<br>
<br>`)
(251149, `Name: {attacker} / {owner} (laid the final blow)<br>
Damage Done: {[numeric] damage, decimalPlaces=0}<br>
<br>
`)
(251150, `Name: {attacker} / {owner}<br>
Damage Done: {[numeric] damage, decimalPlaces=0}<br>
<br>`)
*/


#endif  // EVE_DAMAGE_H