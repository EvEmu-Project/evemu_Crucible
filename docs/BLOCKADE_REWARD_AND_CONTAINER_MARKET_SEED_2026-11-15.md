# Blockade test payout and Crucible container market seed — 15 November 2026

## Scope and compatibility

This is a narrow maintenance update for the **synthetic, non-production Blockade Level 0 combat test mission**, not a rebalance of official Crucible missions. Combat, drone movement, warp tunnel synchronization and historic item attributes are unchanged.

## Combat mission

The shared \`Mission::kBlockadeSecurityTestRewardISK\` is now **50,000,000 ISK** (previously 150,000). All five Blockade agent conversation, offer and journal reward descriptions use 50,000,000 ISK. The existing \`NormalizeBlockadeTestOffer()\` synchronizes the reward value when a previously persisted Blockade offer is looked up and saves the updated offer. Any already-finished payout will not be adjusted retroactively. The reward fits in the existing \`uint32\` payout and \`agtOffers.rewardISK\` representation.

## Why containers were absent

The standard market seed intentionally whitelisted inventory categories but omitted the Celestial category (ID 2). Buyable container types belong to Celestial inventory groups. Do **not** add all Celestial objects to the seed; that would include stations, gates and other world assets.

Instead, allow only historical container inventory groups:
- 12 — Cargo Container
- 340 — Secure Cargo Container
- 448 — Audit Log Secure Container
- 649 — Freight Container

For these four groups, the seed additionally requires a published \`invTypes\` record and a non-null \`marketGroupID\`. Standard permitted category behavior is unchanged. The extra group filter is applied both when generating orders and when removing old NPC corporation sell orders for a \`-force\` rebuild. It is also included in the two legacy SQL market seed scripts.

## How to apply

1. Back up the live MariaDB EVEmu database. Install a built release containing this patch (after build verification), not just the updated C++ source.
2. Check \`sql/evedb.yaml\`: verify \`seed-regions\` includes the character's market region. \`seed-saturation\` selects a fraction of stations; values under 100 can omit a particular station.
3. From the packaged \`sql\` directory, preview the SQL with \`evedbtool seed -force -dryrun\` (or \`evedbtool.exe seed -force -dryrun\` on Windows).
4. Apply with \`evedbtool seed -force\` (Windows: \`evedbtool.exe seed -force\`). Force replaces eligible **NPC corporation sell orders** in configured regions, while the SQL explicitly preserves orders that do not belong to the station corporation; review the effects before running on an established server.
5. Check the Crucible market browser in a seeded region and at a chosen station. Inspect \`mktOrders\` for group 12/340/448/649 items with actual published market group IDs.

For SQL validation on the database, query:

\`\`\`sql
SELECT it.typeID, it.typeName, ig.groupID, it.marketGroupID
FROM invTypes AS it
JOIN invGroups AS ig ON ig.groupID = it.groupID
WHERE it.published = 1
  AND it.marketGroupID IS NOT NULL
  AND ig.groupID IN (12, 340, 448, 649)
ORDER BY ig.groupID, it.typeName;
\`\`\`

## Regression coverage and limitations

- \`AgentHelpersTest\` asserts the new Blockade reward constant.
- Windows/Linux EVEDBTool smoke tests require both the normal seed and \`-force\` cleanup SQL to include the container group filters and the marketGroupID guard.
- These checks validate generated SQL and compilation, **not live market purchases**, item delivery, stock availability at every station, or full Crucible client UI functionality.
- Active persisted offers are normalized on lookup; historic completed mission rewards are not backpaid. DB or client tests are still needed.
