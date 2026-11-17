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

## Offer selection correction (17 November 2026)

The original PR #36 selector called `MakeRandomInt(0, candidates.size() - 1)`. That helper truncates a floating-point value, so a two-entry Blockade/Expedition pool almost always selected its first candidate (Blockade). The mission selector now uses a thread-local `std::mt19937` with an inclusive `std::uniform_int_distribution` to give both offers a real chance. The expedition's premium-module selector uses the same corrected helper so loot pools are not stuck on their first item. This is intentionally scoped to these two PR #36 paths; other legacy random selection is unchanged.

Existing Blockade offers are preserved; restarting the server does not replace them. Request a **new offer** from another eligible Security Agent, or finish the current offer, to exercise the corrected selection. `agtOffers.missionID=65534` identifies the elite mission. No player or mission SQL rewrite is required.

## Mission acceptance: missing faction loot fix (17 November 2026)

A live PR #36 client test reached mission ID **65534**, offer **90**, but acceptance refused the encounter with `missing market-listed module loot for meta group 4`. The initial loot cache demanded `invTypes.published = 1` **and** non-NULL `marketGroupID`, even for valid `invMetaTypes.metaGroupID = 4, 5, 6` modules. This was too strict for older Crucible SDE datasets.

The cache now prefers published, market-listed modules when available. If a particular tier has no such modules, it falls back to actual Module-category (`invGroups.categoryID=7`) types carrying the exact rarity `invMetaTypes.metaGroupID`. It never fabricates type IDs or silently substitutes ordinary items. A missing rarity tier **even after fallback** remains a clear error requiring the installed SDE data to be checked. The server startup log prints per-tier counts and warnings when fallback is in use. No player or SQL data is modified.

Read-only diagnostic SQL for your live database (run in the `EvE` database):

```sql
SELECT mt.metaGroupID, COUNT(*) AS moduleTypes,
       SUM(CASE WHEN t.published = 1 AND t.marketGroupID IS NOT NULL
                     AND t.marketGroupID <> 0 THEN 1 ELSE 0 END) AS marketListed,
       SUM(CASE WHEN t.published = 0 OR t.marketGroupID IS NULL
                     OR t.marketGroupID = 0 THEN 1 ELSE 0 END) AS fallbackCandidates
FROM invMetaTypes AS mt
JOIN invTypes AS t ON t.typeID = mt.typeID
JOIN invGroups AS g ON g.groupID = t.groupID
WHERE mt.metaGroupID IN (4, 5, 6) AND g.categoryID = 7
GROUP BY mt.metaGroupID ORDER BY mt.metaGroupID;
```

After building the patched branch, restart the server and check startup logs for `Elite mission loot modules loaded:`. For a newly generated offer, acceptance should succeed only when all three tier counts are nonzero and all required NPC groups are available. Reopen an existing **unaccepted** offer normally; no manual `agtOffers` edits are needed.

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
