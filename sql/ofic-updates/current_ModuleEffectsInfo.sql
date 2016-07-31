--  Original file and idea by Luck and Aknor Jaden.
--  Updates/Rewrites and Implementation by Allan

--  NOTE:  this is the currently working module effects (129 total)

--
-- Table structure for table `dgmEffectsInfo`
--

CREATE TABLE `dgmEffectsInfo` (
  `effectID` int(11) DEFAULT NULL,
  `sourceAttributeID` int(11) NOT NULL,
  `targetAttributeID` int(11) NOT NULL,
  `calculationTypeID` int(11) NOT NULL,
  `description` varchar(500) NOT NULL,
  `reverseCalculationTypeID` int(11) NOT NULL,
  `targetGroupIDs` varchar(500) NOT NULL,
  `stackingPenalty` int(11) NOT NULL,
  `effectState` int(11) NOT NULL,
  `targetType` int(11) NOT NULL,
  `targetGroup` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmEffectsInfo`
--

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- ID, src, targ, calc, des, rcalc, tgrpID, stack, state, targetType, targetGroup
-- common for all modules
(16, 30, 2, 1, 'PG -> PG_Used', 8, '6', 0, 2, 1, 0),
(16, 50, 49, 1, 'CPU -> CPU_Used', 8, '6', 0, 2, 1, 0),
(2663, 1153, 1152, 1, 'Cal -> Cal-Used', 8, '6', 0, 2, 1, 0),
-- hp
(60, 150, 9, 5, 'Reinforced Bulkhead - HP Bonus', 25, '6', 0, 2, 1, 78),
(60, 150, 9, 5, 'NF Structure - HP Bonus', 25, '6', 0, 2, 1, 763),
(3047, 150, 9, 5, 'Exp Cargohold - HP Penalty', 25, '6', 0, 2, 1, 765),
-- cpu output
(536, 202, 48, 5, 'Co-Processor - CPU Output Bonus', 25, '6', 0, 2, 1, 285),
-- pg
(56, 145, 11, 5, 'Power Diagnostic - PG', 25, '6', 0, 2, 1, 766),
(56, 145, 11, 5, 'Reactor Control - PG', 25, '6', 0, 2, 1, 769),
(627, 549, 11, 1, 'Auxiliary Power Core - PG', 8, '6', 0, 2, 1, 339),
-- cap capacity
(25, 67, 482, 1, 'Capacitor Battery - Cap Capacity', 8, '6', 0, 2, 1, 61),
(58, 147, 482, 5, 'AB/MWD Cap Penalty', 25, '6', 0, 2, 1, 46),
(58, 147, 482, 5, 'Cap Flux Coil - Cap Capacity', 25, '6', 0, 2, 1, 768),
(58, 147, 482, 5, 'Power Diagnostic - Cap Capacity', 25, '6', 0, 2, 1, 766),
-- cap recharge
(51, 144, 55, 5, 'Cap Flux Coil - Cap Recharge Rate', 25, '6', 0, 2, 1, 768),
(51, 144, 55, 5, 'Cap Power Relay - Cap Recharge Rate', 25, '6', 0, 2, 1, 767),
(51, 144, 55, 5, 'Cap Recharger - Cap Recharge Rate', 25, '6', 0, 2, 1, 43),
(51, 144, 55, 5, 'Power Diagnostic - Cap Recharge Rate', 25, '6', 0, 2, 1, 766),
(51, 144, 55, 5, 'Power Relay - Cap Recharge Rate', 25, '6', 0, 2, 1, 57),
-- inertia mods
(657, 169, 70, 56, 'NF Structure - Inertia Bonus', 57, '6', 1, 2, 1, 763),
(657, 169, 70, 56, 'Reinforced Bulkhead - Inertia Penalty', 57, '6', 1, 2, 1, 78),
(657, 169, 70, 56, 'IStab - Inertia Modifier', 57, '6', 1, 2, 1, 762),
-- sig radius mods
(1254, 554, 552, 54, 'MWD Sig Radius Penalty', 55, '6', 1, 12, 1, 46),
(2029, 983, 552, 1, 'Shield Extenders - Sig Radius Penality', 8, '6', 1, 2, 1, 38),
(2644, 554, 552, 54, 'IStab - Sig Radius Penalty', 55, '6', 1, 2, 1, 762),
-- mass mods
(1254, 796, 4, 1, 'AB/MWD Mass addition', 8, '6', 1, 12, 1, 46),
(1959, 796, 4, 1, 'Armor Plates - Mass Addition', 8, '6', 0, 2, 1, 329),
-- cargo cap
(59, 149, 38, 5, 'Exp Cargohold - Cargo Cap Bonus', 25, '6', 0, 2, 1, 765),
(59, 149, 38, 5, 'OD Injector - Cargo Cap Penalty', 25, '6', 0, 2, 1, 764),
-- velocity
(710, 20, 37, 54, 'AB/MWD Velocity Bonus', 55, '6', 1, 12, 1, 46),
(854, 306, 37, 5, 'Cloaking Device - Velocity Penalty', 25, '6', 1, 2, 1, 330),
(2865, 1076, 37, 54, 'NF Structure - Velocity Bonus', 55, '6', 1, 2, 1, 763),
(2859, 306, 37, 5, 'Reinforced Bulkhead - Velocity Penalty', 25, '6', 1, 2, 1, 78),
(2865, 1076, 37, 54, 'OD Injector - Velocity Bonus', 55, '6', 1, 2, 1, 764),
(3046, 306, 37, 5, 'Exp Cargohold - Velocity Penalty', 25, '6', 1, 2, 1, 765),
-- warp core str
(670, 105, 21, 1, 'Warp Stab - Strength Bonus', 8, '6', 0, 2, 1, 315),
(3725, 105, 21, 1, 'Warp Core Strength', 8, '6', 0, 2, 1, 52),
-- target range
(2646, 309, 76, 56, 'Warp Stab - Targeting Range Penalty', 57, '6', 1, 2, 1, 315),
-- scan res
(854, 565, 564, 5, 'Cloaking Device - Scan Resolution', 25, '6', 1, 2, 1, 330),
(2645, 565, 564, 5, 'Warp Stab - Scan Resolution Penalty', 25, '6', 1, 2, 1, 315),

-- Damage Control Unit
(2302, 267, 267, 30, 'Armor EM Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 268, 268, 30, 'Armor Exp Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 269, 269, 30, 'Armor Kin Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 270, 270, 30, 'Armor Therm Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 271, 271, 30, 'Shield EM Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 272, 272, 30, 'Shield Exp Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 273, 273, 30, 'Shield Kin Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 274, 274, 30, 'Shield Therm Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 974, 113, 30, 'Hull EM Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 975, 111, 30, 'Hull Exp Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 976, 109, 30, 'Hull Kin Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 977, 110, 30, 'Hull Therm Resistance Bonus', 31, '6', 0, 12, 1, 60);


-- shield effects data

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- shield cap
(21, 72, 263, 1, 'Extenders - HP', 8, '6', 0, 2, 1, 38),
(57, 146, 263, 5, 'PDU - HP', 25, '6', 0, 2, 1, 766),
(57, 146, 263, 0, 'Flux Coil - HP', 40, '6', 0, 2, 1, 770),
-- shield recharge
(50, 134, 479, 5, 'PDU - Recharge Rate', 25, '6', 1, 2, 1, 766),
(50, 134, 479, 5, 'Flux Coil - Recharge Rate', 25, '6', 1, 2, 1, 770),
(50, 134, 479, 5, 'Power Relay - Recharge Rate', 25, '6', 1, 2, 1, 57),
(50, 134, 479, 5, 'Recharger - Recharge Rate', 25, '6', 1, 2, 1, 39),
-- passive shield resists
(2052, 984, 271, 50, 'Amplifier - Passive EM Resist', 51, '6', 1, 2, 1, 295),
(2052, 985, 272, 50, 'Amplifier - Passive Exp Resist', 51, '6', 1, 2, 1, 295),
(2052, 986, 273, 50, 'Amplifier - Passive Kin Resist', 51, '6', 1, 2, 1, 295),
(2052, 987, 274, 50, 'Amplifier - Passive Therm Resist', 51, '6', 1, 2, 1, 295),
-- active shield resists
(2117, 994, 271, 50, 'Hardener - Passive EM Resist', 51, '6', 1, 2, 1, 77),
(2117, 995, 272, 50, 'Hardener - Passive Exp Resist', 51, '6', 1, 2, 1, 77),
(2117, 996, 273, 50, 'Hardener - Passive Kin Resist', 51, '6', 1, 2, 1, 77),
(2117, 997, 274, 50, 'Hardener - Passive Therm Resist', 51, '6', 1, 2, 1, 77),
(2118, 984, 271, 50, 'Hardener - Active EM Resist', 51, '6', 1, 12, 1, 77),
(2118, 985, 272, 50, 'Hardener - Active Exp Resist', 51, '6', 1, 12, 1, 77),
(2118, 986, 273, 50, 'Hardener - Active Kin Resist', 51, '6', 1, 12, 1, 77),
(2118, 987, 274, 50, 'Hardener - Active Therm Resist', 51, '6', 1, 12, 1, 77),
-- actives overload bonus
(3035, 1208, 271, 54, 'Hardener - Overload EM Resist', 55, '6', 0, 8, 1, 77),
(3035, 1208, 272, 54, 'Hardener - Overload Exp Resist', 55, '6', 0, 8, 1, 77),
(3035, 1208, 273, 54, 'Hardener - Overload Kin Resist', 55, '6', 0, 8, 1, 77),
(3035, 1208, 274, 54, 'Hardener - Overload Therm Resist', 55, '6', 0, 8, 1, 77);

-- armor effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- armor hp
(63, 148, 265, 5, 'Coating - HP', 25, '6', 0, 2, 1, 98),
(63, 148, 265, 5, 'Plating - HP', 25, '6', 0, 2, 1, 326),
(2837, 1159, 265, 1, 'Plates - HP', 8, '6', 0, 2, 1, 329),
-- passive armor resists
(2041, 984, 267, 50, 'Plating - Passive EM Resist', 51, '6', 1, 2, 1, 98),
(2041, 985, 268, 50, 'Plating - Passive Exp Resist', 51, '6', 1, 2, 1, 98),
(2041, 986, 269, 50, 'Plating - Passive Kin Resist', 51, '6', 1, 2, 1, 98),
(2041, 987, 270, 50, 'Plating - Passive Therm Resist', 51, '6', 1, 2, 1, 98),
(2041, 984, 267, 50, 'Coating - Passive EM Resist', 51, '6', 1, 2, 1, 326),
(2041, 985, 268, 50, 'Coating - Passive Exp Resist', 51, '6', 1, 2, 1, 326),
(2041, 986, 269, 50, 'Coating - Passive Kin Resist', 51, '6', 1, 2, 1, 326),
(2041, 987, 270, 50, 'Coating - Passive Therm Resist', 51, '6', 1, 2, 1, 326),
-- active armor resists
(2084, 994, 267, 50, 'Hardener - Passive EM Resist', 51, '6', 1, 2, 1, 328),
(2084, 995, 268, 50, 'Hardener - Passive Exp Resist', 51, '6', 1, 2, 1, 328),
(2084, 996, 269, 50, 'Hardener - Passive Kin Resist', 51, '6', 1, 2, 1, 328),
(2084, 997, 270, 50, 'Hardener - Passive Therm Resist', 51, '6', 1, 2, 1, 328),
(2098, 984, 267, 50, 'Hardener - Active EM Resist', 51, '6', 1, 12, 1, 328),
(2098, 985, 268, 50, 'Hardener - Active Exp Resist', 51, '6', 1, 12, 1, 328),
(2098, 986, 269, 50, 'Hardener - Active Kin Resist', 51, '6', 1, 12, 1, 328),
(2098, 987, 270, 50, 'Hardener - Active Therm Resist', 51, '6', 1, 12, 1, 328),
-- active overload bonus
(3029, 1208, 113, 56, 'Hardener - Overload EM Resist', 57, '6', 0, 8, 1, 328),
(3031, 1208, 111, 56, 'Hardener - Overload Exp Resist', 57, '6', 0, 8, 1, 328),
(3032, 1208, 109, 56, 'Hardener - Overload Kin Resist', 57, '6', 0, 8, 1, 328),
(3030, 1208, 110, 56, 'Hardener - Overload Therm Resist', 57, '6', 0, 8, 1, 328),
(4039, 1208, 113, 56, 'Hardener - Overload EM Resist', 57, '6', 0, 8, 1, 328),
(4039, 1208, 111, 56, 'Hardener - Overload Exp Resist', 57, '6', 0, 8, 1, 328),
(4039, 1208, 109, 56, 'Hardener - Overload Kin Resist', 57, '6', 0, 8, 1, 328),
(4039, 1208, 110, 56, 'Hardener - Overload Therm Resist', 57, '6', 0, 8, 1, 328);

-- rig effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- drawbacks
(2712, 1138, 265, 54, 'Rig Drawback - Armor HP', 55, '6', 0, 2, 1, 782),
(2713, 1138, 48, 54, 'Rig Drawback - CPU Output', 55, '6', 0, 2, 1, 778),
(2716, 1138, 552, 54, 'Rig Drawback - Sig Radius', 55, '6', 1, 2, 1, 774),
(2717, 1138, 37, 54, 'Rig Drawback - Max Velocity', 55, '6', 1, 2, 1, 773),
(2718, 1138, 263, 54, 'Rig Drawback - Shield Capacity', 55, '6', 0, 2, 1, 786),
(3528, 1138, 30, 54, 'Rig Drawback - Cap Recharge Rate', 55, '6', 0, 2, 1, 904),
-- cpu output
(397, 424, 48, 54, 'Engineering Rig - CPU Output Bonus', 55, '6', 0, 2, 1, 781),
-- cap recharge
(485, 314, 55, 54, 'Engineering Rig - Cap Recharge Rate', 55, '6', 0, 2, 1, 781),
-- pg
(490, 313, 11, 54, 'Engineering Rig - PG Output Bonus', 55, '6', 0, 2, 1, 781),
-- cap capacity
(2432, 1079, 482, 54, 'Engineering Rig - Cap Capacty', 55, '6', 0, 2, 1, 781),
-- velocity
(394, 315, 37, 54, 'Astronautic Rig - Velocity Bonus', 55, '6', 1, 2, 1, 782),
(3727, 1076, 37, 54, 'Astronautic Rig - Velocity Bonus', 55, '6', 1, 2, 1, 782),
-- inertia
(395, 151, 70, 54, 'Astronautic Rig - Inertia Bonus', 55, '6', 1, 2, 1, 782),
(3726, 169, 70, 54, 'Astronautic Rig - Inertia Bonus', 55, '6', 1, 2, 1, 782),
-- warp core
(494, 319, 153, 54, 'Astronautic Rig - Warp Core Bonus', 55, '6', 0, 2, 1, 782),
-- warp speed
(856, 624, 1281, 54, 'Astronautic Rig - Warp Speed Bonus', 55, '6', 0, 2, 1, 782),
-- cargo
(836, 614, 38, 54, 'Astronautic Rig - Cargohold Bonus', 55, '6', 0, 2, 1, 782),
-- shield rigs
(486, 338, 479, 56, 'Shield Rig - Shield Recharge Bonus', 57, '6', 0, 2, 1, 774),
(2795, 984, 271, 50, 'Shield Rig - EM Resist', 51, '6', 1, 2, 1, 774),
(2795, 985, 272, 50, 'Shield Rig - Exp Resist', 51, '6', 1, 2, 1, 774),
(2795, 986, 273, 50, 'Shield Rig - Kin Resist', 51, '6', 1, 2, 1, 774),
(2795, 987, 274, 50, 'Shield Rig - Therm Resist', 51, '6', 1, 2, 1, 774),
-- armor rigs
(271, 335, 265, 0, 'Armor Rig - Hp Bonus', 40, '6', 0, 2, 1, 773),
(2792, 984, 267, 50, 'Armor Rig - EM Resist', 51, '6', 1, 2, 1, 773),
(2792, 985, 268, 50, 'Armor Rig - Exp Resist', 51, '6', 1, 2, 1, 773),
(2792, 986, 269, 50, 'Armor Rig - Kin Resist', 51, '6', 1, 2, 1, 773),
(2792, 987, 270, 50, 'Armor Rig - Therm Resist', 51, '6', 1, 2, 1, 773);


