

COMMAND( create, Acct::Role::VIP,
         "(itemID) [count] - Create count or 1 of the specified item." )
COMMAND( createitem, Acct::Role::ADMIN,
         "(itemID) [count] - Create count or 1 of the specified item.(from Insider)" )
COMMAND( goto, Acct::Role::VIP,
         "(x) (y) (z) - Jump to the specified position in space. Stopped." )
COMMAND( translocate, Acct::Role::VIP,
         "(entityID) - Translocate to the specified entity" )
COMMAND( tr, Acct::Role::VIP,
         "(who) (entityID) - Translocate the specified person to the specified entity" )
COMMAND( halt, Acct::Role::VIP,
         "- Immediately stops ship, setting Destiny::State = dstball_halt.")
COMMAND( location, Acct::Role::PLAYER,
         "- Gives you back your current location in space." )
COMMAND( syncloc, Acct::Role::PLAYER,
         "- Synchronizes your location in client with location on server." )
COMMAND( syncpos, Acct::Role::PLAYER,
         "- Synchronizes locations of all dynamic entities in current bubble with location on server for all players in callers bubble." )
COMMAND( update, Acct::Role::PLAYER,
         "- Sets Current Position according to Server's DestinyManager, then Sends Bubble AddBalls and Destiny SetState. (resets spaceview with current server data)" )
COMMAND( sendstate, Acct::Role::CONTENT,
         "- Sends DoDestinyUpdate SetState." )
COMMAND( addball, Acct::Role::PLAYER,
         "- Sends BubbleManager AddBalls.")
COMMAND( addball2, Acct::Role::PLAYER,
         "- Sends BubbleManager AddBalls2.")
COMMAND( kill, Acct::Role::VIP,
         "(entityID) - insta-pops a destroyable ship, drone, structure, if applicable")
COMMAND( killallnpcs, Acct::Role::ADMIN,
         " - insta-pops all NPC ships in the current bubble")
COMMAND( cloak, Acct::Role::VIP,
         " - instantly and unconditionally toggles cloak state of your vessel")
COMMAND( sov, Acct::Role::CONTENT,
         " - unknown at this time")
COMMAND( pos, Acct::Role::PROGRAMMER,
         " - commands item to Immediately anchor")
COMMAND( hop, Acct::Role::CONTENT,
         " - unknown at this time")
