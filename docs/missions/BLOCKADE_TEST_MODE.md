# The Blockade security-agent encounter test

This is an isolated runtime test path using synthetic mission ID `65535`.

- Security agents can offer the encounter to eligible characters.
- The test fleet contains **24 Angel Cartel frigates** (inventory group ID `550`); no cruisers or battleships are included.
- NPCs are distributed around the pocket centre rather than stacked at one position.
- Accepting this test mission creates the encounter site and starts a **two-hour completion timer**. The offer itself retains the existing one-day offer window before acceptance.
- Each player/player-drone kill in the mission pocket increments the mission kill counter. When the final required frigate is destroyed, the counter reaches `24/24` and the agent dialogue exposes the existing **Complete** turn-in action.
- Returning to the issuing agent and choosing **Complete** marks the offer completed, removes the mission site, and pays **50,000,000 ISK** through the existing agent mission-reward wallet path.
- The server stores the completed state before issuing the payout, and the agent offer is removed after turn-in to prevent repeat completion through the normal dialogue flow.

This is still a synthetic single-wave test encounter, not the canonical multi-wave Blockade mission. It does not add the reported eight-step reinforcement sequence, canonical NPC mix, or official SDE mission ID.