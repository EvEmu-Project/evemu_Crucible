

 COMMAND( siglist, Acct::Role::CONTENT,
          " - lists all active signatures, with location, name, and type." )
 COMMAND( heal, Acct::Role::HEALSELF,
          "(entityID) - heal the character with the entityID" )
 COMMAND( healtarget, Acct::Role::HEALSELF,
          "(entityID) - heal the currently active target" )
 COMMAND( status, Acct::Role::PLAYER,
          " - note giving you detailed ship status information")
 COMMAND( secstatus, Acct::Role::PLAYER,
          " - note giving you this character's current security status")
 COMMAND( list, Acct::Role::PLAYER,
          " - gives a list of all dynamic entities and players and their destinyState in this bubble")
 COMMAND( commandlist, Acct::Role::PLAYER,
          " - gives a list of all game commands, required role, and a description")
 COMMAND( destinyvars, Acct::Role::GMH,
          " - shows some current destiny variables")
 COMMAND( shipvars, Acct::Role::GMH,
          " - shows other current destiny variables")
 COMMAND( shutdown, Acct::Role::GMH,
          " - save all items, kick all connections, and halt server. immediate command." )
 COMMAND( beltlist, Acct::Role::PLAYER,
          " - list all roids in current belt's inventory." )
 COMMAND( inventory, Acct::Role::PROGRAMMER,
          " - list all items in current location's inventory (either station or solsystem)." )
 COMMAND( cargo, Acct::Role::PLAYER,
          " - gives list of cargo contents and volumes in all holds")
 COMMAND( shipinventory, Acct::Role::PLAYER,
          " - list all items in current ship's inventory." )
 COMMAND( showsession, Acct::Role::PLAYER,
          " - list current session values." )
 COMMAND( skilllist, Acct::Role::PROGRAMMER,
          " - list all skills loaded for character." )
 COMMAND( shipdna, Acct::Role::PLAYER,
          " - show current ship DNA." )
 COMMAND( targlist, Acct::Role::PROGRAMMER,
          " - show current ship target list." )
 COMMAND( bubblelist, Acct::Role::PROGRAMMER,
          " - show current objects in bubble, their destiny state, and movement speed." )
 COMMAND( track, Acct::Role::PROGRAMMER,
          " - toggles current object movement tracking state." )
 COMMAND( attrlist, Acct::Role::PROGRAMMER,
          " - show current attributes for itemID given in args." )
 COMMAND( warpto, Acct::Role::GMH,
          " - warp to an object using its itemID." )
 COMMAND( fleetboost, Acct::Role::PLAYER,
          " - note giving you detailed fleet boost information")
 COMMAND( fleetinvite, Acct::Role::PLAYER,
          " - sets fleet invite scope to <arg> or none")
 COMMAND( getposition, Acct::Role::PLAYER,
          " - gets positional data between ship and moon (for pos placement testing)")
 COMMAND( bubbletrack, Acct::Role::PROGRAMMER,
          " - enable/disable bubble outline tracking." )
 COMMAND( players, Acct::Role::PLAYER,
          " - shows list of online players")
 COMMAND( showall, Acct::Role::GMH,
          " - shows all dynamic items in system on ship scanner.")
 COMMAND( autostop, Acct::Role::PLAYER,
          " - sets repair, boost, and ?? modules to stop when target full")
 COMMAND( bubblewarp, Acct::Role::GMH,
          " - begin warp to given bubbleID in current ship.")
 COMMAND( runtest, Acct::Role::PROGRAMMER,
          " - run testing::posTest()." )
 COMMAND( bindList, Acct::Role::PROGRAMMER,
          " - list of current bound objects (with clients)." )
 COMMAND( dropLoot, Acct::Role::PROGRAMMER,
          " - commands all npcs in bubble to jettison loot." )