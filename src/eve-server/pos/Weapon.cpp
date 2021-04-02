
/**
 * @name Weapon.cpp
 *   Class for POS Weapon Modules.
 *
 * @Author:         Allan
 * @date:   28 December 17
 */

/*
 * POS__ERROR
 * POS__WARNING
 * POS__MESSAGE
 * POS__DUMP
 * POS__DEBUG
 * POS__DESTINY
 * POS__SLIMITEM
 * POS__TRACE
 */


#include "pos/Weapon.h"


/*
 * 752     controlTowerProjectileOptimalBonus  NULL    50
 * 753     controlTowerProjectileFallOffBonus  NULL    50
 * 754     controlTowerProjectileROFBonus  NULL    -25
 * 872     disallowOffensiveModifiers  NULL    1
 */
WeaponSE::WeaponSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{

}

void WeaponSE::Init()
{
    StructureSE::Init();
}

void WeaponSE::Process()
{
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process state changes  */
    StructureSE::Process();
    /** @todo (Allan)  will need some form of AI to engage defensive modules if/when any structure is attacked */
}


/*  small arty battery
 * 9   hp  100000  NULL
 * 30  power   101250  NULL
 * 50  cpu     0   NULL
 * 51  speed   15750   NULL
 * 54  maxRange    187500  NULL
 * 56  chargeRate  NULL    1
 * 64  damageMultiplier    NULL    44
 * 109     kineticDamageResonance  NULL    1
 * 110     thermalDamageResonance  NULL    1
 * 111     explosiveDamageResonance    NULL    1
 * 113     emDamageResonance   NULL    1
 * 128     chargeSize  2   NULL
 * 136     uniformity  NULL    1
 * 154     proximityRange  330000  NULL
 * 156     incapacitationRatio     NULL    0.001
 * 158     falloff     NULL    168000
 * 160     trackingSpeed   NULL    0.00704
 * 182     requiredSkill1  11584   NULL
 * 192     maxLockedTargets    1   NULL
 * 208     scanRadarStrength   NULL    200
 * 209     scanLadarStrength   NULL    200
 * 210     scanMagnetometricStrength   NULL    200
 * 211     scanGravimetricStrength     NULL    200
 * 245     gfxTurretID     NULL    488
 * 263     shieldCapacity  NULL    25000
 * 265     armorHP     375000  NULL
 * 277     requiredSkill1Level     1   NULL
 * 416     entityFlyRange  187500  NULL
 * 466     entityReactionFactor    NULL    0.5
 * 475     entityAttackDelayMin    7500    NULL
 * 476     entityAttackDelayMax    30000   NULL
 * 479     shieldRechargeRate  NULL    2500000
 * 484     shieldUniformity    NULL    0.75
 * 524     armorUniformity     NULL    0.75
 * 525     structureUniformity     NULL    1
 * 552     signatureRadius     NULL    3000
 * 556     anchoringDelay  NULL    5000
 * 564     scanResolution  NULL    250
 * 604     chargeGroup1    83  NULL
 * 620     optimalSigRadius    NULL    125
 * 676     unanchoringDelay    NULL    5000
 * 677     onliningDelay   NULL    120000
 * 691     targetSwitchDelay   60000   NULL
 * 771     ammoCapacity    NULL    10000
 * 872     disallowOffensiveModifiers  1   NULL
 * 1165    controlTowerMinimumDistance     5000    NULL
 * 1167    posPlayerControlStructure   1   NULL
 * 1768    typeColorScheme     NULL    11342
 */

/*  cruise missile battery
 * 9   hp  100000  NULL
 * 30  power   40000   NULL
 * 50  cpu     40  NULL
 * 109     kineticDamageResonance  NULL    1
 * 110     thermalDamageResonance  NULL    1
 * 111     explosiveDamageResonance    NULL    1
 * 113     emDamageResonance   NULL    1
 * 136     uniformity  NULL    1
 * 154     proximityRange  350000  NULL
 * 156     incapacitationRatio     NULL    0.001
 * 182     requiredSkill1  11584   NULL
 * 192     maxLockedTargets    1   NULL
 * 208     scanRadarStrength   NULL    200
 * 209     scanLadarStrength   NULL    200
 * 210     scanMagnetometricStrength   NULL    200
 * 211     scanGravimetricStrength     NULL    200
 * 212     missileDamageMultiplier     NULL    1
 * 263     shieldCapacity  NULL    25000
 * 265     armorHP     375000  NULL
 * 277     requiredSkill1Level     1   NULL
 * 416     entityFlyRange  175000  NULL
 * 466     entityReactionFactor    NULL    0.5
 * 475     entityAttackDelayMin    7500    NULL
 * 476     entityAttackDelayMax    30000   NULL
 * 479     shieldRechargeRate  NULL    2500000
 * 484     shieldUniformity    NULL    0.75
 * 506     missileLaunchDuration   NULL    24000
 * 524     armorUniformity     NULL    0.75
 * 525     structureUniformity     NULL    1
 * 552     signatureRadius     NULL    3000
 * 556     anchoringDelay  NULL    5000
 * 564     scanResolution  NULL    250
 * 604     chargeGroup1    386     NULL
 * 645     missileEntityVelocityMultiplier     NULL    2
 * 646     missileEntityFlightTimeMultiplier   NULL    2.5
 * 676     unanchoringDelay    NULL    5000
 * 677     onliningDelay   NULL    120000
 * 691     targetSwitchDelay   60000   NULL
 * 771     ammoCapacity    NULL    10000
 * 858     missileEntityAoeCloudSizeMultiplier     NULL    0.25
 * 859     missileEntityAoeVelocityMultiplier  NULL    1.5
 * 872     disallowOffensiveModifiers  1   NULL
 * 1165    controlTowerMinimumDistance     5000    NULL
 * 1167    posPlayerControlStructure   1   NULL
 */

/*  torpedo battery
 * 9   hp  100000  NULL
 * 30  power   80000   NULL
 * 50  cpu     80  NULL
 * 109     kineticDamageResonance  NULL    1
 * 110     thermalDamageResonance  NULL    1
 * 111     explosiveDamageResonance    NULL    1
 * 113     emDamageResonance   NULL    1
 * 136     uniformity  NULL    1
 * 154     proximityRange  350000  NULL
 * 156     incapacitationRatio     NULL    0.001
 * 182     requiredSkill1  11584   NULL
 * 192     maxLockedTargets    1   NULL
 * 208     scanRadarStrength   NULL    200
 * 209     scanLadarStrength   NULL    200
 * 210     scanMagnetometricStrength   NULL    200
 * 211     scanGravimetricStrength     NULL    200
 * 212     missileDamageMultiplier     NULL    2.5
 * 263     shieldCapacity  NULL    50000
 * 265     armorHP     750000  NULL
 * 277     requiredSkill1Level     1   NULL
 * 416     entityFlyRange  175000  NULL
 * 466     entityReactionFactor    NULL    0.5
 * 475     entityAttackDelayMin    7500    NULL
 * 476     entityAttackDelayMax    30000   NULL
 * 479     shieldRechargeRate  NULL    5000000
 * 484     shieldUniformity    NULL    0.75
 * 506     missileLaunchDuration   NULL    24000
 * 524     armorUniformity     NULL    0.75
 * 525     structureUniformity     NULL    1
 * 552     signatureRadius     NULL    3000
 * 556     anchoringDelay  NULL    5000
 * 564     scanResolution  NULL    75
 * 604     chargeGroup1    89  NULL
 * 645     missileEntityVelocityMultiplier     NULL    2.5
 * 646     missileEntityFlightTimeMultiplier   NULL    5
 * 676     unanchoringDelay    NULL    5000
 * 677     onliningDelay   NULL    120000
 * 691     targetSwitchDelay   60000   NULL
 * 771     ammoCapacity    NULL    10000
 * 858     missileEntityAoeCloudSizeMultiplier     NULL    0.75
 * 859     missileEntityAoeVelocityMultiplier  NULL    1.5
 * 872     disallowOffensiveModifiers  1   NULL
 * 1165    controlTowerMinimumDistance     5000    NULL
 * 1167    posPlayerControlStructure   1   NULL
 */