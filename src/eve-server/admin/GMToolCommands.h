//no macroguard on purpose
/* commands the Crucible client's GM menus send that the server didn't have.  see GMToolCommands.cpp */
COMMAND( loop, Acct::Role::VIP,
         "(count) (command...) - runs a slash command count times (max 100).  e.g. /loop 3 /fit me 2048" )
COMMAND( gag, Acct::Role::GML,
         "(charID|name) [\"reason\"] [minutes] - stops a character talking in chat channels (default 30 minutes)" )
COMMAND( ungag, Acct::Role::GML,
         "(charID|name) - lets a gagged character talk again" )
COMMAND( gagiskspammer, Acct::Role::LEGIONEER,
         "(charID) - gags an ISK spammer for 30 days" )
COMMAND( baniskspammer, Acct::Role::GML,
         "(charID) - gags an ISK spammer and bans their account" )
COMMAND( nukem, Acct::Role::ADMIN,
         "- kills every NPC in your bubble (same as /killallnpcs)" )
COMMAND( entity, Acct::Role::SPAWN,
         "deploy (count) (typeID|\"type name\") - spawns NPCs next to you" )
