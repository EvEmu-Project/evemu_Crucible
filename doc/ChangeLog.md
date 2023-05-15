*** 0.8.5 ***
- [FEAT] Dungeon Editor
- [FEAT] Cynosural Field Generators
- [FEAT] Jump Drives
- [FEAT] Outpost creation and destruction
- [FEAT] Wormholes
- [FEAT] Loyalty Points, mission rewards and store
- Support aarch64 architecture
- Complete service rewrite
- Many minor bugfixes and overall improvements

*** 0.8.4 ***
- Added support for aarch64 with docker-compose
- [FEAT] Alliances Implemented
- [FEAT] Sovereignty Implemented
- Fixed bug in PI database causing segmentation fault
- Fixed sales tax default to 1%
- CustomError system reimplemented
- Updated ImageServer to allow for Alliance logos to be uploaded correctly
- Replaced liveupdates with updated versions
- Implemented new Database management tool (EVEDBTool) for migration versioning
- Implemented automatic market database seeding

*** 0.8.3 ***
- Fleet Implemented

*** 0.8.2 ***
- [FEAT] PI Implemented
- [FEAT] Basic POS Implemented

*** 0.8.1 ***
- [FEAT] Agents and Courier Missions Implemented

*** 0.8.0 (26/03/2021) ***
This release brings in massive new core system updates to EVEmu. These changes will dramatically increase the rate of change to this repository and make future contributions easier.

- Returned to numbered versioning
- Large new update to all core systems from Alasiya EvE (all credits to Allan for his fantastic work here)
- Database has been updated to reflect core changes to the server code
- Repository has been organized for quality of life
- CI configuration has been added for automated building of EVEmu binaries to be distributed
- Docker Compose support has been added to make the setting up of a new server quick and easy

*** 22/07/2008 ***
- Added autotools

*** 03/05/2008 ***
- Eliminate invBlueprints table by pulling data from other existing tables. (Bloody.Rabbit)
- Implemented bubble manager to handle in-space items which are not visible system wide.
- Partial support for boarding an empty ship in space.
- Properly undock to the station's dock location (firefoxpdm)
- Properly use ships position in space from the DB.
- Store ship position in space when logging out.
- Improved support for fitting charges with weapons.
- Improved NPC AI to make them consistently follow, orbit and attack.
- Hybrid and Laser modules consistently fire on NPCs now.
- Implemented damage code including basic messages.
- Implemented NPC death logic.
- Implement code to support asteroids in space.
- Add new command to spawn an asteroid: /roid (typeID) (radius)
- Implemented graphics for mining lasers (they do not yet actually mine though)
- Load up space junk (roids, items, etc) from DB ay system boot.
- Implement runtime memory error exception handler to help prevent one bad action from crashing the whole server.

*** 02/15/2008 ***
- Load tutorial information from DB (Bloody.Rabbit)

*** 02/03/2008 ***
- Celestial Statistics support (firefoxpdm)

*** 11/24/2007 ***
- Reworked bind ID storage code (firefoxpdm)
- More coporation cleanup (firefoxpdm)
- Fix issue with the wallet (firefoxpdm)
- Fixed "show info" tab for DoGetStation (firefoxpdm)
- Fixed station backgrounds (firefoxpdm)
- Implemented people and places lookup (firefoxpdm)
- Fix for owner notes (GuristaSyndicate)
alter table chrOwnerNote CHANGE content note TEXT NOT NULL;

*** Release 229 ***

*** 10/20/2007 ***
- Intial market implementation:
  - Major work on special market packet encodings.
  - Browse market, see pending orders.
  - Initial price history implementation.
  - Place sell orders (builds the market)
  - Place buy orders
  - Place immediate sell orders
  - Place immediate buy orders
  - you prolly need to log out&in before seeing new orders right now.
- Major destiny (space) work:
  - Much improved event broadcasting to allow players to
    observe the actions of another player.
  - New module manager to handle different weapon types
  - Improved timer support for module activation/deactivation.
  - Can now activate weapons and generate fake hits on targets.
  - Initial infrastructure for damage propigation.
- A bit more corporation creation work. (firefoxpdm)
- Initial NPC AI work. Very very simple right now.
- Added /spawn (typeID) command to spawn an NPC.
- Started reworking chat channels a bit. It is prolly more broken
  than it was previously right now, but should improve over time.
- Start rework on chat channels to support dynamic channels.
- Initial XML packet support for none-value duality handling.
- Added queue for destiny updates to properly batch them up.
Update SQL:
ALTER TABLE market_orders DROP `isBuy`;
ALTER TABLE market_orders CHANGE `bid` `bid` tinyint(3) unsigned NOT NULL default '0';
ALTER TABLE market_orders CHANGE `issued` `issued` bigint(20) unsigned NOT NULL default '0';
ALTER TABLE market_orders CHANGE `issued` `issued` bigint(20) unsigned NOT NULL default '0';
ALTER TABLE market_transactions CHANGE `transactionDateTime` `transactionDateTime` bigint(20) unsigned NOT NULL default '0';
ALTER TABLE market_transactions ADD `regionID` int(10) unsigned NOT NULL default '0';
New Table: billsPayable
New Table: market_history_old

*** 07/25/2007 ***
- Major revelation with destiny updates, fixes 'jumpy' movement in space.
- Much better understanding of the binary destiny update structure.
- Significant work on the destiny manager. Server now tracks client movement for
  warp, goto direction, and follow movement modes (orbit is the only major mode pending).
- Some initial drone launching code (not working yet)
- A bunch of makefile cleanup.
- Added support for the <files><cache>.. element in config file to cache the cache files
- Implemented a keepalive ping to hold idle sessions open.
- Added support for server originated messsages: error, infomodal, and notify
- Made persistent attributes auto-store.
- Fix for crash when jumping between systems.
- Owner Note implemented. (firefoxpdm)
- Corp channel creation and logo fixes (firefoxpdm)
- A bunch of other corporation creation stuff (firefoxpdm)

*** 5/01/2007 - 182 ***
- Initial targeting system
- Some support for training new skills from a skill book. It may work some times.
- added new command /search which searches items for summoning purposes
- added functionality to support the server sending you evemails (like for long search results)
- Initial standing history support (firefoxpdm)
- Initial corp creation code (firefoxpdm)
- Reworked the database priming technique.
- Merged the agtAgents2 table into agtAgents
- A few fixes for static owner querying.
- Initial ability to equip and activate modules (may not persist properly)

SQL Changes:
new table: chrStandingChanges
ALTER TABLE `corporation` CHANGE `corporationID` `corporationID` INT( 10 ) UNSIGNED NOT NULL AUTO_INCREMENT;
ALTER TABLE `corporation`  AUTO_INCREMENT =2000001;

*** 4/22/2007 - 169 ***
Major increase in error checking of generated xmlp objects
New FastEncode() operation on generated xmlp objects to reduce copying
A new mode of element operations in xmlp (non-pointer)
Major reworking of all item stuff to use the new inventory system


alter table entity AUTO_INCREMENT=140000000;
alter table dgmAttributeTypes CHANGE attributeID attributeID INTEGER UNSIGNED;
new table entity_attributes

*** 04/15/2007 ***
- Implemented initial employment history support (firefoxpdm)
- Implemented Character Note (LSMoura)
- Got evetool building on win32, still needs some work though.
- Added `unmarsahal` command to evetool to decode strings from logserver.
- Dramatic reworking of service calling mechanism to support names args and exceptions.
- A few fixes pointed out by sgi's audit tool.
- Got the market to display items, and active orders (cannot place or view orders though).
- More work on agents, still no significant progress
- Initial rev of EVEMail (FireFoX)
- Char creation information loading based on selected values (FireFoX)
- An initial implementation of /tr for some GM menus.

- Implemented soft dictionaries (to address os.hashid issue)
- Implemented character delete (no timer)
- Implemented simple cached methods (no arguments, return cached objects)
- Fixed agent listing.
- Fixed issue with boot-on-demand systems and NPC spawning.
- Fixed several memory leaks

0.5.153:
- Implemented an initial NPC spawn system, supporting static spawns with possibly randomized locations.
- Fixed ship display while in space.
- Fixed a few cache related issues in order to support NPCs.
- Implemented the beginings of an inventory system, supporting items and their attributes.
- Added custom build tool to win32 project to support xml packet generation
- Several patches from the forums.

