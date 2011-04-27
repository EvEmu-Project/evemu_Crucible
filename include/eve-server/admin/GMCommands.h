//no macroguard on purpose


COMMAND( create, ROLE_ADMIN,
         "(itemID) [count] - Create count or 1 of the specified item." )
COMMAND( createitem, ROLE_ADMIN,
         "(itemID) [count] - Create count or 1 of the specified item.(from Insider)" )
COMMAND( search, ROLE_ADMIN,
         "(text) - Search for items matching the specified query" )
COMMAND( translocate, ROLE_ADMIN,
         "(entityID) - Translocate to the specified entity" )
COMMAND( tr, ROLE_ADMIN,
         "(who) (entityID) - Translocate the specified person to the specified entity" )
COMMAND( giveisk, ROLE_ADMIN,
         "(entityID) (amount) - Give the specified amount of cash to the specified character. 0=self." )
COMMAND( pop, ROLE_ADMIN,
         "(type) (key) (value) - Send an OnRemoteMessage" )
COMMAND( goto, ROLE_ADMIN,
         "(x) (y) (z) - Jump to the specified position in space. Stopped." )
COMMAND( spawnn, ROLE_ADMIN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( spawn, ROLE_ADMIN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( location, ROLE_ADMIN,
         "- Gives you back your current location in space." )
COMMAND( syncloc, ROLE_ADMIN,
         "- Synchonizes your location in client with location on server." )
COMMAND( setbpattr, ROLE_ADMIN,
         "(itemID) (copy) (materialLevel) (productivityLevel) (licensedProductionRunsRemaining) - Change blueprint's attributes." )
COMMAND( state, ROLE_ADMIN,
         "- Sends DoDestinyUpdate SetState." )
COMMAND( getattr, ROLE_ADMIN,
         "(itemID) (attributeID) - Retrieves attribute value." )
COMMAND( setattr, ROLE_ADMIN,
         "(itemID) (attributeID) (value) - Sets attributeID of itemID to value." )
COMMAND( fit, ROLE_ADMIN,
		"(itemID) - Fits selected item to active ship." )
COMMAND( giveskill, ROLE_ADMIN,
		"(itemID) - gives skills to fit item." )
COMMAND( online, ROLE_ADMIN,
		"(entityID) - online all modules on the ship of the entityID. entityID=me=>online my modules" )
COMMAND( unload, ROLE_ADMIN,
		"(entityID) (itemID) - unload module itemID from entityID (itemID=all=>unload all) (entityID=me=>my modules)" )
COMMAND( heal, ROLE_ADMIN,
		"(entityID) - heal the character with the entityID" )
COMMAND( repairmodules, ROLE_ADMIN,
		"(entityID) (itemID) - repair the modules of the character with the entityID" )
COMMAND( unspawn, ROLE_ADMIN,
		"(entityID) (itemID) - unload module itemID from entityID (itemID=all=>unload all) (entityID=me=>my modules)" )
COMMAND( giveskills, ROLE_ADMIN,
		"(itemID) - gives skills to character." )
COMMAND( dogma, ROLE_ADMIN,
		"(attribute) - change item attribute value" )
COMMAND( kick, ROLE_ADMIN,
		"(charName) - kicks [charName] from the server")
COMMAND( ban, ROLE_ADMIN,
		"(charName) - bans player's account from the server")
COMMAND( unban, ROLE_ADMIN,
		"(charName) - removes ban on player's account")
COMMAND( kenny, ROLE_ADMIN,
        "(ON,OFF,0,1) - enable/disable the Kenny Translator for your chatting entertainment!")
/*COMMAND( entity, ROLE_ADMIN,
		"(entityID) - unknown" )
COMMAND( chatban, ROLE_ADMIN,
		"(characterID) - bans character from channel" )
COMMAND( whois, ROLE_ADMIN,
		"(characterName) - returns information on character")
COMMAND( giveallskills, ROLE_ADMIN,
		"(characterID) - gives all skills to character")
*/
