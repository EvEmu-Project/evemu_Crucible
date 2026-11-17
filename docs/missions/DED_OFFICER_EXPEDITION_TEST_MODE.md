# DED & Officer Expedition — Security Agent mission

**Added:** 17 November 2026 · **Synthetic mission ID:** `65534` · **Agent division:** the same Security Agent classes and levels 1–5 as the existing `65535` Blockade.

## Purpose

A separate, high-difficulty combat encounter cloned from the working Blockade offer/accept/pocket/kill/reward lifecycle. **The original Blockade remains available and unchanged.** Security Agents can randomly offer either mission. New characters qualifying for Level 0 Security Agent starter content may also be offered this challenge; it is deliberately difficult and can be declined.

## Elite NPC roster: 18 ships

The new mission selects one actual NPC type per inventory group from the installed Crucible static data. No invented NPC type IDs or schema inserts are used.

| Encounter | Groups and faction | Target hulls |
|---|---|---|
| 5 DED/deadspace ships | Angel `597`, Blood Raiders `605`, Guristas `613`, Sansha `620`, Serpentis `630` | Frigate, destroyer, cruiser, battlecruiser, battleship |
| 5 faction commanders | Angel `789`, Blood `796`, Guristas `798`, Sansha `807`, Serpentis `852` | Frigate through battleship |
| 5 pirate officers | Angel `553`, Blood `559`, Guristas `564`, Sansha `569`, Serpentis `574` | Actual officer classes in loaded data |
| 3 DED overseers | `819`, `820`, `821` | Frigate, cruiser, battleship |

NPC ownership, faction and corporation are set per selected faction, with ships dispersed around the encounter pocket, not stacked on top of the warp-in point.

## Loot and rewards

Each destroyed mission NPC spawns a wreck. An extra **one-unit premium module** is added to the wreck based on the NPC group's category: faction commander `invMetaGroups.metaGroupID=4`, pirate officer `5`, DED/deadspace and overseer `6`. Modules are randomly selected **from actual published, market-listed Module-category types** loaded into `StaticDataMgr` from the installed SDE; no fake loot is created. This is a mission-only premium drop in addition to any normal group loot triggered by the existing global drop chance. Premium item selection is by rarity tier, not guaranteed faction-matched naming.

If any rarity tier or required NPC group has no supported type in the installed static data, the encounter is refused at acceptance rather than replacing elite spawns with normal rats. If an individual NPC fails to load/spawn, the actual spawned count determines the objective, and the ordinary final-pocket-clear check lets the agent complete the mission without a stuck kill target. A loot item may fail to materialize if the item factory cannot spawn it; such failures are logged and must be tested.

**Mission payout:** 50,000,000 ISK, copied from Blockade, paid once on the normal agent turn-in path. **Timer:** 120 minutes from acceptance. Bounty, security standings, wreck ownership, salvage and mission persistence otherwise use the existing encounter engine.

## Safety and deployment

- No SQL migration, schema rewrite, NPC spawn-table rewrite, item database edits or player-data changes are required.
- Existing Blockade offers and already accepted encounters keep their original mission ID and NPC fleet.
- Only **new offers** can select the extra mission; after updating the server binary, restart the server and request a Security Agent mission.
- A character can hold at most one offer with a specific agent, but can work with other Security Agents; selecting the new mission from the same agent is random.
- Because five officers and several battleship-class NPCs may spawn together, consider it an **endgame challenge irrespective of the issuing agent's level**. Test on a backup/test character and avoid losing valuable ships.

## Test checklist

- [ ] Windows UCRT64 server builds; Linux GCC server builds; Docker succeeds
- [ ] `MissionLifecycleTest` and `tools/verify_elite_security_mission.py` pass
- [ ] Security Agent offers Blockade **and** DED & Officer Expedition as separate missions
- [ ] A non-Security Agent does not offer the synthetic elite mission
- [ ] Mission briefing and journal show the literal name, 18-kill objective, 50 million ISK reward and 120-minute limit
- [ ] Warp bookmark points to new pocket; NPCs spawn across all 5 hull classes / categories
- [ ] Every ship is the correct pirate faction; 5 commanders, 5 officers, 5 DED ships and 3 overseers present
- [ ] NPC kills generate their wreck, bounty and premium meta-group module type; ordinary loot/salvage remains available
- [ ] Kill counter advances, no respawns after kills, last kill completes objective
- [ ] Return to issuing agent and collect mission reward **once**, no duplicate wallet credit
- [ ] Original 12-ship Blockade remains playable and unchanged
- [ ] Restart/resume, decline/expire and multiple agents do not corrupt existing offers

**Verification scope:** Source tests and CI compile are not evidence of a successful 2012 Crucible client playtest. Record the build SHA, agent/mission/offer IDs, spawn log, wreck module typeIDs, and completed wallet transaction after in-game testing. Do not mark the mission fully verified until these checks pass.
