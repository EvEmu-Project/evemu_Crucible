# The Blockade security-agent encounter test

This is an isolated runtime test path using synthetic mission ID `65535`.

- Security agents can offer the encounter to eligible characters.
- New mission pockets spawn **12 Angel Cartel NPC ships**: **8 cruisers** (inventory group `551`) and **4 battlecruisers** (group `576`), with no frigates, destroyers or battleships.
- NPCs are distributed around the pocket centre rather than stacked at one position.
- Accepting this test mission creates the encounter site and starts a **two-hour completion timer**. The offer itself retains the existing one-day offer window before acceptance.
- Each player/player-drone kill in the mission pocket increments the mission kill counter. When the twelfth ship is destroyed, progress reaches `12/12` and the agent dialogue exposes the existing **Complete** turn-in action.
- Returning to the issuing agent and choosing **Complete** marks the offer completed, removes the mission site, and pays **50,000,000 ISK** through the existing agent mission-reward wallet path.
- The server stores the completed state before issuing the payout, and the agent offer is removed after turn-in to prevent repeat completion through the normal dialogue flow.

**Deployment note:** The new fleet is applied to newly created mission pockets after deploying the updated server. Old pending (unaccepted) 24-ship offers are upgraded to a 12-kill objective when the agent loads them. Already-accepted 24-ship encounters retain their existing ship composition and kill target; finish or abandon one before requesting a new 12-ship encounter. No live database is changed by this patch.

This is still a synthetic single-wave test encounter, not the canonical multi-wave Blockade mission. It does not add the reported eight-step reinforcement sequence, canonical NPC mix, or official SDE mission ID.