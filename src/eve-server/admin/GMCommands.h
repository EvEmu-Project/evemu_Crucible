//no macroguard on purpose


COMMAND( create, ROLE_SPAWN,
         "(itemID) [count] - Create count or 1 of the specified item." )
COMMAND( createitem, ROLE_SPAWN,
         "(itemID) [count] - Create count or 1 of the specified item.(from Insider)" )
COMMAND( search, ROLE_SPAWN,
         "(text) - Search for items matching the specified query" )
COMMAND( translocate, ROLE_SPAWN,
         "(entityID) - Translocate to the specified entity" )
COMMAND( tr, ROLE_SPAWN,
         "(who) (entityID) - Translocate the specified person to the specified entity" )
COMMAND( giveisk, ROLE_SPAWN,
         "(entityID) (amount) - Give the specified amount of cash to the specified character. 0=self." )
COMMAND( pop, ROLE_SPAWN,
         "(type) (key) (value) - Send an OnRemoteMessage" )
COMMAND( goto, ROLE_SPAWN,
         "(x) (y) (z) - Jump to the specified position in space. Stopped." )
COMMAND( spawnn, ROLE_SPAWN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( spawn, ROLE_SPAWN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( location, ROLE_SPAWN,
         "- Gives you back your current location in space." )
COMMAND( syncloc, ROLE_SPAWN,
         "- Synchonizes your location in client with location on server." )
COMMAND( setbpattr, ROLE_SPAWN,
         "(itemID) (copy) (materialLevel) (productivityLevel) (licensedProductionRunsRemaining) - Change blueprint's attributes." )
COMMAND( state, ROLE_SPAWN,
         "- Sends DoDestinyUpdate SetState." )
COMMAND( getattr, ROLE_SPAWN,
         "(itemID) (attributeID) - Retrieves attribute value." )
COMMAND( setattr, ROLE_SPAWN,
         "(itemID) (attributeID) (value) - Sets attributeID of itemID to value." )
COMMAND( fit, ROLE_SPAWN,
        "(itemID) - Fits selected item to active ship." )
COMMAND( giveallskills, ROLE_SPAWN,
        "['me'|<characterID>] - gives ALL skills to designated character or self" )
COMMAND( giveskill, ROLE_SPAWN,
        "(itemID) - gives skills to fit item." )
COMMAND( online, ROLE_SPAWN,
        "(entityID) - online all modules on the ship of the entityID. entityID=me=>online my modules" )
COMMAND( unload, ROLE_SPAWN,
        "(entityID) (itemID) - unload module itemID from entityID (itemID=all=>unload all) (entityID=me=>my modules)" )
COMMAND( heal, ROLE_SPAWN,
        "(entityID) - heal the character with the entityID" )
COMMAND( repairmodules, ROLE_SPAWN,
        "(entityID) (itemID) - repair the modules of the character with the entityID" )
COMMAND( unspawn, ROLE_SPAWN,
        "(entityID) (itemID) - unload module itemID from entityID (itemID=all=>unload all) (entityID=me=>my modules)" )
COMMAND( giveskills, ROLE_SPAWN,
        "(itemID) - gives skills to character." )
COMMAND( dogma, ROLE_SPAWN,
        "(attribute) - change item attribute value" )
COMMAND( kick, ROLE_SPAWN,
        "(charName) - kicks [charName] from the server")
COMMAND( ban, ROLE_SPAWN,
        "(charName) - bans player's account from the server")
COMMAND( unban, ROLE_SPAWN,
        "(charName) - removes ban on player's account")
COMMAND( kenny, ROLE_SPAWN,
        "(ON,OFF,0,1) - enable/disable the Kenny Translator for your chatting entertainment!")
COMMAND( kill, ROLE_SPAWN,
        "(entityID) - insta-pops a destroyable ship, drone, structure, if applicable")
COMMAND( killallnpcs, ROLE_SPAWN,
        " - insta-pops all NPC ships in the current bubble")
COMMAND( cloak, ROLE_SPAWN,
		" - instantly and unconditionally toggles cloak state of your vessel")
/*COMMAND( entity, ROLE_SPAWN,
        "(entityID) - unknown" )
COMMAND( chatban, ROLE_SPAWN,
        "(characterID) - bans character from channel" )
COMMAND( whois, ROLE_SPAWN,
        "(characterName) - returns information on character")
*/
