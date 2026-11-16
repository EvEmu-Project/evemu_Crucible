# Agent services and station portraits — source audit (15 November 2026)

## Scope and observed problem

The user reported missing portraits for agents in stations. The audit reviewed `AgentMgrService`, `AgentDB`, `ManagerDB::GetAgents`, `AgentBound`, `Agent`, and the legacy HTTP image server. This is a source audit; no live client HTTP capture or MariaDB runtime query was available during this pass.

## Confirmed source defects and changes

1. Station agent lists used `chrNPCCharacters.stationID`, which is nullable when legacy NPC character rows are incomplete or absent. The agent rowset now falls back to `staStations.stationID` using `agtAgents.locationID` and keeps expected column aliases and order. Agents genuinely operating in space are not falsely assigned to a station.
2. Individual `AgentDB::LoadAgentData` returned early when bloodline metadata was missing, even though the corresponding `agtAgents` row was valid. The primary identity now loads independently of optional NPC portrait/bloodline data, with fallback system/station and name values where available.
3. Agent loading reports true only when its primary database row was found, and `AgentData` starts initialized. This prevents invalid partially initialized agents being cached.
4. The image fallback pointed to the deprecated `http://image.eveonline.com/Character/{id}_{size}.jpg`. The server continues accepting Crucible-format incoming URLs but redirects missing NPC portraits to `https://images.evetech.net/characters/{id}/portrait?size=64` when the client requests a 50px thumbnail. Other categories map to modern type icons/renders and corporation/alliance logos.
5. Existing local portrait files under `image_cache/Character/{agentID}_50.jpg` or `{agentID}_512.jpg` still take precedence. The remote service does not necessarily host every historical NPC portrait; an unavailable portrait still requires a properly licensed local image.
6. Local PNG image responses no longer use `Content-Type: image/jpeg`. Zero IDs/size are rejected before a redirect starts.
7. Agent actions validate offer existence, legal state and expiry before accepting, completing, declining, deferring or quitting. Previously the server could act on a default-constructed offer or attempt a completion repeatedly. Bonus payment is now tied to acceptance time and minute duration, and the briefing preview displays `bonusISK` rather than a derived multiple of base reward.

## Regression tests

`AgentHelpersTest` tests modern URL paths and Crucible 50px compatibility, unsupported categories, 30-minute time bonuses, expired bonuses and no-reward cases. A successful Windows/Linux/Docker build does not prove images are present on the external CDN or that all legacy clients follow an HTTPS redirect.

## Runtime verification checklist

- Open station agents in a Crucible client, inspect image requests for `/Character/{agentID}_50.jpg`, confirm HTTPS redirects and view portraits; compare agents with complete and incomplete `chrNPCCharacters` rows.
- Supply a local portrait file to `image_cache/Character/{agentID}_50.jpg` and confirm local precedence over external redirects.
- Verify offered/deferred/accepted/completed/expired mission actions, exactly-once mission reward, and bonus payment just before and after its configured deadline.
- Inspect agent journal, station list, bloodline/gender metadata and research-agent skills; restart to verify offer persistence.

## Important limitations

This patch does not synthesize authentic NPC portraits or change NPC portrait identifiers. Some historical portraits may not exist on CCP's current image service. HTTPS following by a 2012 Crucible client needs direct testing, and multi-room mission logic, research agents and storyline systems remain incomplete. No database migration or live database update is part of the source changes.

References: [Current EVE image server](https://docs.esi.evetech.net/docs/image_server.html), [fork patch notes](../PATCH_NOTES.md).

## Offline station-agent portrait follow-up — 15 November 2026

The follow-up branch `fix/offline-agent-station-portraits-2026-11-15` removes external redirects for the Crucible NPC agent-ID range and packages a generic local JPEG fallback. A one-time optional agent portrait importer can cache available original CCP portraits from official imagery sources; no historical portraits are invented or automatically redistributed. See [offline rollout and verification](AGENT_LOCAL_PORTRAITS_2026-11-15.md). These are source changes; a successful CI run will not prove authentic NPC artwork coverage or live 2012-client presentation.

## Further agent UI and lookup optimization

- The Crucible agent conversation now sets `missionCompleted=true` in its out-of-band response only after an actual completion. Previously this field remained false on every successful completion.
- The Quit response flag is set only after a valid accepted mission is found; invalid quits no longer signal success.
- `Agent::CanUseAgent()` reuses the agent standing already fetched earlier in the same function, eliminating a duplicate standing query per agent access check.
