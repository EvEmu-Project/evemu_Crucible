//no macroguard on purpose

COMMAND( summon, ROLE_ADMIN,
         "(itemID) [count] - Summon count or 1 of the specified item." )
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
COMMAND( spawn, ROLE_ADMIN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( location, ROLE_ADMIN,
         "- Gives you back your current location in space." )
COMMAND( clearlog, ROLE_ADMIN,
		 "- Clears server log." )
COMMAND( getlog, ROLE_ADMIN,
		 "- Sends server log." )
COMMAND( setbpattr, ROLE_ADMIN,
         "(itemID) (copy) (materialLevel) (productivityLevel) (licensedProductionRunsRemaining) - Change blueprint's attributes." )
COMMAND( state, ROLE_ADMIN,
		 "- Sends DoDestinyUpdate SetState." )




