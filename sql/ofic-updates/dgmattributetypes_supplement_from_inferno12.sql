
--
-- Dumping data for table `dgmAttributeTypes`
--

INSERT INTO `dgmAttributeTypes` (`attributeID`, `attributeName`, `description`, `iconID`, `defaultValue`, `published`, `displayName`, `unitID`, `stackable`, `highIsGood`, `categoryID`) VALUES
(1839, 'damageDelayDuration', 'The delay in ms until the damage is done to the target. (Allows some FX to be played)', NULL, 10000, 1, NULL, 3, 1, 1, 7),
(1837, 'rookieShieldBoostBonus', 'Bonus to shield booster repair amount', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1836, 'rookieSPTDamageBonus', 'Bonus to Small Projectile Turret damage', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1835, 'rookieShipVelocityBonus', 'Bonus to ship velocity', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1834, 'rookieTargetPainterStrengthBonus', 'Bonus to target painter effectiveness', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1833, 'rookieArmorRepBonus', 'Bonus to armor repair amount', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1832, 'rookieDampStrengthBonus', 'Bonus to sensor damper effectiveness', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1831, 'rookieDroneBonus', 'Bonus to drone damage, HP and mining yield', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1830, 'rookieSHTDamageBonus', 'Bonus to Small Hybrid Turret damage', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1829, 'rookieShieldResistBonus', 'Shield resistance bonus', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1828, 'rookieECMStrengthBonus', 'ECM Strength Bonus', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1827, 'rookieMissileKinDamageBonus', 'Bonus to kinetic missile damage', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1826, 'rookieSHTOptimalBonus', 'Small Hybrid Turret optimal range bonus', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1825, 'rookieArmorResistanceBonus', 'Bonus to armor resistances', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1824, 'rookieWeaponDisruptionBonus', 'Bonus to tracking disruptor effectiveness', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1823, 'rookieSETDamageBonus', 'Energy turret damage bonus', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1822, 'rookieSETCapBonus', 'Reduction in energy turret capacitor use', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1821, 'skillAllyCostModifierBonus', '', NULL, 0, 1, 'Ally Cost Modifier Percentage Per Level', NULL, 1, 1, 7),
(1820, 'baseDefenderAllyCost', 'The base cost of hiring an ally into a war', NULL, 10000000, 0, NULL, 133, 1, 1, 7),
(1818, 'aurumConversionRate', '', NULL, 0, 0, NULL, NULL, 1, 1, NULL),
(1817, 'nosReflectAmountBonus', '', NULL, 1, 1, 'Energy Leech Reflect Amount', 105, 0, 1, NULL),
(1816, 'neutReflectAmountBonus', '', NULL, 1, 1, 'Neutralizer Reflect Amount', 105, 0, 1, NULL),
(1815, 'neutReflectAmount', '', NULL, 1, 1, NULL, 111, 1, 1, NULL),
(1814, 'nosReflectAmount', '', NULL, 1, 1, NULL, 111, 1, 1, NULL),
(1813, 'titanBonusScalingRadius', 'This attribute is used by the effect which sets titans'' turrets to reduce damage below a certain target signature radius.', NULL, 0, 0, 'Turret Damage Scaling Radius', 1, 1, 1, 7),
(1812, 'turretDamageScalingRadius', 'Turret damage is reduced when shooting at targets with a sig radius below this value.', NULL, 0, 1, 'Damage Reduction Threshold', 1, 1, 1, 7),
(1811, 'capAttackReflector', 'Chance of reflecting NOS/neut effect back on attacker', NULL, 0, 1, 'Capacitor Attack Reflect Chance', 111, 1, 1, NULL),
(1809, 'neutReflector', '', NULL, 1, 1, NULL, NULL, 0, 1, NULL),
(1808, 'nosReflector', '', NULL, 1, 1, NULL, NULL, 0, 1, NULL);
