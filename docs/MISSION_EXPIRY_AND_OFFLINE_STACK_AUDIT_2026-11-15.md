# Mission expiry and offline mission-item cleanup — 15 November 2026

## Scope

Source audit of the existing Crucible mission-offer lifecycle: AgentBound completion, Agent::MakeOffer, MissionDataMgr expiry/active indexes, and MissionDB offline inventory cleanup. This is a source review; it is not evidence of a live 2012 client or MariaDB test.

## Confirmed source defects

1. MissionDataMgr::Process dereferenced a possibly missing Agent pointer from EntityList::GetAgent while expiring offers.
2. The agent-index cache removed the first offer for an agent ID, even when other characters had missions with the same agent.
3. Deferred offers are stored in the Allocated state, but the expiry handler only transitioned Offered offers to Expired.
4. Completing a mission left the old offer in the global active indexes until its deadline; same-agent follow-up offers could be overwritten or removed by cache methods that did not compare persistent offer IDs.
5. Agent::MakeOffer cached a zero-ID offer when agtOffers INSERT failed, losing persistence across restart.
6. Offline mission-item cleanup stored item stack counts as uint16 and wrote the requested removal amount as the remaining stack size.

## Changes

- Guard missing agents and continue durable expiry-state updates even when no client notification is possible.
- Match cached offers by characterID and persistent offerID, including removal during expiry and updates after mission acceptance.
- Archive successful completions directly to the existing historical-offer collection.
- Expire deferred/Allocated offers along with newly Offered offers.
- Reject mission creation if the database returned no offer ID.
- Use uint32 item quantities, correct subtraction, zero-quantity guard, ordered item selection, and stop on SQL mutation errors.
- Add a deterministic MissionLifecycleTest for offered/deferred states, per-character offer selection, exact/partial stack consumption, and quantities exceeding uint16.

## Verification

Previously green master build 43747566528 predates these changes. Run Windows UCRT64, Linux GCC, Docker and CTest on this branch before claiming compilation. Required in-game scenarios: two characters using the same agent, completion followed by an immediate new request, deferred-offer expiry, restarting between state transitions, and expired courier cleanup for an offline character with multiple item stacks.

## Remaining risks

The existing offline query still identifies inventory by owner and type, not mission-specific item provenance or location. The changes do not implement transactional, exactly-once payout or full mission rewards rollback after a database failure. No warp/navigation subsystem is changed.
