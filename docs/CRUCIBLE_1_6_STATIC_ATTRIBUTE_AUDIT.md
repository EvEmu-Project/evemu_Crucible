# Crucible 1.6 static attributes audit

Reference: Fuzzwork's archived conversion of the CCP Crucible 1.6 export, https://www.fuzzwork.co.uk/dump/old/Crucible/cru16/. CCP's 1.6.5 patch notes are at https://www.eveonline.com/news/view/patch-notes-for-crucible-1.6. The comparison is pinned to the archive, not a current SDE.

| Reference table | Compressed SHA-256 |
| --- | --- |
| invTypes | 2e76ad2893d6ddbd4c42bd62a92b24a4cd8b604c51e26238058346aa31b7fa5a |
| invGroups | 290678b36c82a9ebd260fffbefe620e8573d26dc0b5637e04223f19584b8b3d6 |
| dgmTypeAttributes | 397f8eb486f844cdb657800ff274d8f7d0f2f1d81910394387eb366f5c10e076 |
| dgmTypeEffects | 46eb626cd1a2e83c068f8d75664e21682aeae17b6e70c81a3b2a51b60c204588 |
| dgmAttributeTypes | 539591470669129d436e92f052c1911ab806a29fafde432d41ac9c6258ead347 |

The audit matched published rows by type ID and group ID in categories 6, 7, 8, 18 and 32. Coverage was 3,532 modules, 815 charges, 114 drones, 80 subsystems, and 259 ships across the published ship groups. It compared 104,651 numeric dogma attribute keys, allowing tiny floating-point conversion differences, as well as effect assignments, radius, mass, volume, capacity, portion size, and race. All matched module, charge, drone, and subsystem numeric dogma values agree apart from the emulator's additional module isOnline initialization. This includes turret tracking/optimal/falloff, missile velocity/flight time/explosion attributes, mining yield/range/cycle, and shield module amounts/capacity/resistances. Static agreement does not establish fitted stacking, modifier, or client behavior.

The migration corrects 446 guarded inventory scalar values and 30 guarded hull dogma values against the archive. It skips the fitting-limit changes for hull types with deployed fittings and preserves values already customized away from the bundled inputs. The six barge/exhumer ore holds and three other specialized bay differences were recorded for a separate live cargo audit. Removing those bays while they contain inventory can hide cargo. Warp attribute 600 uses different units in this fork and remains outside this change. Three effect assignment differences were recorded for gameplay verification. The branch and reference also differ in publication or presence for 19 IDs. This change does not add those types or change names or publication.

The runtime changes correct Deep Core laser group precedence and target selection and send shield updates for the ship receiving remote shield transfer. Before enabling the changes on an existing server, test on a database copy and restart to reload type data. Check ordinary ore, Mercoxit, ice and gas separately, then local shield boosting and remote transfer with two ships. Check fitted hulls before applying any fitting values the migration deliberately skipped. Player inventory is never moved by the migration.
