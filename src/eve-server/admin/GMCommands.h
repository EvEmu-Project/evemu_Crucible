//no macroguard on purpose


COMMAND( spawnn, Acct::Role::ADMIN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( spawn, Acct::Role::SPAWN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( unspawn, Acct::Role::SPAWN,
         "(itemID) - remove and delete itemID" )
COMMAND( search, Acct::Role::VIP,
         "(text) - Search for items matching the specified query" )
COMMAND( giveisk, Acct::Role::GMH,
         "(entityID) (amount) - Give the specified amount of cash to the specified character. 0=self." )
COMMAND( pop, Acct::Role::ADMIN,
         "(type) (key) (value) - Send an OnRemoteMessage" )
COMMAND( setbpattr, Acct::Role::CONTENT,
         "(itemID) (copy) (materialLevel) (productivityLevel) (licensedProductionRunsRemaining) - Change blueprint's attributes." )
COMMAND( getattr, Acct::Role::VIP,
         "(itemID) (attributeID) - Retrieves attribute value." )
COMMAND( setattr, Acct::Role::CONTENT,
         "(itemID) (attributeID) (value) - Sets attributeID of itemID to value." )
COMMAND( fit, Acct::Role::VIP,
        "(itemID) - Fits selected item to active ship." )
COMMAND( giveallskills, Acct::Role::ADMIN,
        "['me'|<characterID>] - gives ALL skills to designated character or self" )
COMMAND( giveskill, Acct::Role::VIP,
         "(skillID) (level) - gives skillID to specified level." )
COMMAND( online, Acct::Role::VIP,
        "(entityID) - online all modules on the ship of the entityID. entityID=me=>online my modules" )
COMMAND( unload, Acct::Role::VIP,
        "(entityID) (itemID) - unload module itemID from entityID (itemID=all=>unload all) (entityID=me=>my modules)" )
COMMAND( repairmodules, Acct::Role::HEALSELF,
        "(entityID) (itemID) - repair the modules of the character with the entityID" )
COMMAND( giveskills, Acct::Role::ADMIN,
        "(itemID) - gives skills to character." )
COMMAND( dogma, Acct::Role::ADMIN,
        "(attribute) - change item attribute value" )
COMMAND( kick, Acct::Role::ADMIN,
        "(charName) - kicks [charName] from the server")
COMMAND( ban, Acct::Role::ADMIN,
        "(charName) - bans player's account from the server")
COMMAND( unban, Acct::Role::ADMIN,
        "(charName) - removes ban on player's account")
COMMAND( givelp, Acct::Role::ADMIN,
        "('me'|<characterID>) (corporationID) (amount) - adds LP to a character for a specific corporation")

/*
COMMAND( entity, Acct::Role::ADMIN,
        "(entityID) - unknown" )
COMMAND( chatban, Acct::Role::ADMIN,
        "(characterID) - bans character from channel" )
COMMAND( whois, Acct::Role::ADMIN,
        "(characterName) - returns information on character")
*/
