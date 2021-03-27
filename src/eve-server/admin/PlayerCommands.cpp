
    /* Custom config file options
     * current settings displayed on console at start-up
     *   -allan 7June2015
     */
    sLog.Blue("     ServerConfig", "World Switches");
    if (sConfig.world.saveOnMove) {
        sLog.Green("     Save on Move","Enabled.");
    } else {
        sLog.Warning("     Save on Move","Disabled.");
    }
    if (sConfig.world.saveOnUpdate) {
        sLog.Green("   Save on Update","Enabled.");
    } else {
        sLog.Warning("   Save on Update","Disabled.");
    }
    if (sConfig.world.StationDockDelay) {
        sLog.Green("    Docking Delay","Enabled.");
    } else {
        sLog.Warning("    Docking Delay","Disabled.");
    }
    if (sConfig.world.gridUnload) {
        sLog.Green("   Grid Unloading","Enabled.  Grids will unload after %u seconds of inactivity.", sConfig.world.gridUnloadTime);
    } else {
        sLog.Warning("   Grid Unloading","Disabled.");
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Rate Modifiers");
    if (sConfig.rates.secRate != 1.0) {
        sLog.Yellow("        SecStatus","Modified at %.0f%%.", (sConfig.rates.secRate *100) );
    } else {
        sLog.Green("        SecStatus","Normal.");
    }
    if (sConfig.rates.npcBountyMultiply != 1.0) {
        sLog.Yellow("          Bountys","Modified at %.0f%%.", (sConfig.rates.npcBountyMultiply *100) );
    } else {
        sLog.Green("          Bountys","Normal.");
    }
    if (sConfig.rates.DropItem != 1) {
        sLog.Yellow("       Item Drops","Modified at %ux.", sConfig.rates.DropItem );
    } else {
        sLog.Green("       Item Drops","Normal.");
    }
    if (sConfig.rates.DropSalvage != 1) {
        sLog.Yellow("    Salvage Drops","Modified at %ux.", sConfig.rates.DropSalvage );
    } else {
        sLog.Green("    Salvage Drops","Normal.");
    }
    if (sConfig.rates.DropMoney != 1.0) {
        sLog.Yellow("      Isk Rewards","Modified at %.0f%%.", (sConfig.rates.DropMoney *100) );
    } else {
        sLog.Green("      Isk Rewards","Normal.");
    }
    if (sConfig.rates.damageRate != 1.0) {
        sLog.Yellow("      All Damages","Modified at %.0f%%.", (sConfig.rates.damageRate *100) );
    } else {
        sLog.Green("      All Damages","Normal.");
    }
    if (sConfig.rates.turretDamage != 1.0) {
        sLog.Yellow("       Turret Dmg","Modified at %.0f%%.", (sConfig.rates.turretDamage *100) );
    } else {
        sLog.Green("       Turret Dmg","Normal.");
    }
    if (sConfig.rates.turretRoF != 1.0) {
        sLog.Yellow("       Turret ROF","Modified at %.0f%%.", (sConfig.rates.turretRoF *100) );
    } else {
        sLog.Green("       Turret ROF","Normal.");
    }
    if (sConfig.rates.missileDamage != 1.0) {
        sLog.Yellow("      Missile Dmg","Modified at %.0f%%.", (sConfig.rates.missileDamage *100) );
    } else {
        sLog.Green("      Missile Dmg","Normal.");
    }
    if (sConfig.rates.missileRoF != 1.0) {
        sLog.Yellow("      Missile ROF","Modified at %.0f%%.", (sConfig.rates.missileRoF *100) );
    } else {
        sLog.Green("      Missile ROF","Normal.");
    }
    if (sConfig.rates.missileTime != 1.0) {
        sLog.Yellow("     Missile Time","Modified at %.0f%%.", (sConfig.rates.missileTime *100) );
    } else {
        sLog.Green("     Missile Time","Normal.");
    }
    std::printf("\n");     // spacer
    sLog.Blue("     ServerConfig", "R.A.M. Rate Modifiers");
    if (sConfig.ram.ResPE != 1.0) {
        sLog.Yellow(" PE Research Time","Modified at %.0f%%.", (sConfig.ram.ResPE *100) );
    } else {
        sLog.Green(" PE Research Time","Normal.");
    }
    if (sConfig.ram.ResME != 1.0) {
        sLog.Yellow(" ME Research Time","Modified at %.0f%%.", (sConfig.ram.ResME *100) );
    } else {
        sLog.Green(" ME Research Time","Normal.");
    }
    if (sConfig.ram.MatMod != 1.0) {
        sLog.Yellow("Material Modifier","Modified at %.0f%%.", (sConfig.ram.MatMod *100) );
    } else {
        sLog.Green("Material Modifier","Normal.");
    }
    if (sConfig.ram.CopyTime != 1.0) {
        sLog.Yellow("        Copy Time","Modified at %.0f%%.", (sConfig.ram.CopyTime *100) );
    } else {
        sLog.Green("        Copy Time","Normal.");
    }
    if (sConfig.ram.ProdTime != 1.0) {
        sLog.Yellow("  Production Time","Modified at %.0f%%.", (sConfig.ram.ProdTime *100) );
    } else {
        sLog.Green("  Production Time","Normal.");
    }
    if (sConfig.ram.InventTime != 1.0) {
        sLog.Yellow("   Invention Time","Modified at %.0f%%.", (sConfig.ram.InventTime *100) );
    } else {
        sLog.Green("   Invention Time","Normal.");
    }
    if (sConfig.ram.ReTime != 1.0) {
        sLog.Yellow("          RE Time","Modified at %.0f%%.", (sConfig.ram.ReTime *100) );
    } else {
        sLog.Green("          RE Time","Normal.");
    }
    if (sConfig.ram.WasteMod != 1.0) {
        sLog.Yellow("   Waste Modifier","Modified at %.0f%%.", (sConfig.ram.WasteMod *100) );
    } else {
        sLog.Green("   Waste Modifier","Normal.");
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig","Critical Hit Chances");
    if (sConfig.rates.PlayerCritChance != 0.02f) {
        sLog.Yellow("           Player","Modified at %.1f%%.", (sConfig.rates.PlayerCritChance *100) );
    } else {
        sLog.Green("           Player","Normal at 2%%.");
    }
    if (sConfig.rates.NpcCritChance != 0.015f) {
        sLog.Yellow("              NPC","Modified at %.1f%%.", (sConfig.rates.NpcCritChance *100) );
    } else {
        sLog.Green("              NPC","Normal at 1.5%%.");
    }
    if (sConfig.rates.SentryCritChance != 0.02f) {
        sLog.Yellow("           Sentry","Modified at %.1f%%.", (sConfig.rates.SentryCritChance *100) );
    } else {
        sLog.Green("           Sentry","Normal at 2%%.");
    }
    if (sConfig.rates.DroneCritChance != 0.03f) {
        sLog.Yellow("            Drone","Modified at %.1f%%.", (sConfig.rates.DroneCritChance *100) );
    } else {
        sLog.Green("            Drone","Normal at 3%%.");
    }
    if (sConfig.rates.ConcordCritChance != 0.05f) {
        sLog.Yellow("          Concord","Modified at %.1f%%.", (sConfig.rates.ConcordCritChance *100) );
    } else {
        sLog.Green("          Concord","Normal at 5%%.");
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Feature Switches");
    if (sConfig.ram.AutoEvent) {
        sLog.Green("    RAM AutoEvent","Enabled.");
    } else {
        sLog.Warning("    RAM AutoEvent","Disabled.");
    }
    if (sConfig.server.ModuleAutoOff) {
        sLog.Green("  Module Auto-Off","Enabled.");
    } else {
        sLog.Warning("  Module Auto-Off","Disabled.");
    }
    if (sConfig.server.AsteroidsOnDScan) {
        sLog.Green("  DScan Asteroids","Enabled.");
    } else {
        sLog.Warning("  DScan Asteroids","Disabled.");
    }
    if (sConfig.server.CargoMassAdditive) {
        sLog.Green("       Cargo Mass","Enabled.");
    } else {
        sLog.Warning("       Cargo Mass","Disabled.");
    }
    if (sConfig.cosmic.BumpEnabled) {
        sLog.Green("Bumping Mechanics","Enabled.");
    } else {
        sLog.Warning("Bumping Mechanics","Disabled.");
    }
    if (sConfig.server.LoadOldMissions) {
        sLog.Green("Keep Old Missions","Enabled.");
    } else {
        sLog.Warning("Keep Old Missions","Disabled.");
    }
    if (sConfig.testing.EnableDrones) {
        sLog.Green("    Player Drones","Enabled.");
    } else {
        sLog.Warning("    Player Drones","Disabled.");
    }
    if (sConfig.testing.ShipHeat) {
        sLog.Green("        Ship Heat","Enabled.");
    } else {
        sLog.Warning("        Ship Heat","Disabled.");
    }
    if (sConfig.cosmic.PIEnabled) {
        sLog.Green("        PI System","Enabled.");
    } else {
        sLog.Warning("        PI System","Disabled.");
    }
    if (sConfig.cosmic.AnomalyEnabled) {
        sLog.Green("   Anomaly System","Enabled.");
    } else {
        sLog.Warning("   Anomaly System","Disabled.");
    }
    if (sConfig.cosmic.DungeonEnabled) {
        sLog.Green("   Dungeon System","Enabled.");
    } else {
        sLog.Warning("   Dungeon System","Disabled.");
    }
    if (sConfig.cosmic.BeltEnabled) {
        sLog.Green("   Asteroid Belts","Enabled.");
    } else {
        sLog.Warning("   Asteroid Belts","Disabled.");
    }
    if (sConfig.npc.StaticSpawns) {
        sLog.Green("    Static Spawns","Enabled.  Checks every %u minutes", sConfig.npc.StaticTimer /60);
    } else {
        sLog.Warning("    Static Spawns","Disabled.");
    }
    if (sConfig.npc.RoamingSpawns) {
        sLog.Green("   Roaming Spawns","Enabled.  Checks every %u minutes", sConfig.npc.RoamingTimer /60);
    } else {
        sLog.Warning("   Roaming Spawns","Disabled.");
    }
    if (sConfig.npc.RoamingSpawns or sConfig.npc.StaticSpawns)
        sLog.Green("   Spawns Enabled","Respawn timer checks every %u minutes", sConfig.npc.RespawnTimer /60);
    if (sConfig.server.BountyPayoutDelayed) {
        sLog.Green(" Delayed Bounties","Delayed Bounties are Enabled.  Loop runs every %u minutes", sConfig.server.BountyPayoutTimer);
        if (sConfig.server.FleetShareDelayed) {
            sLog.Green(" Delayed Bounties","Delay for Fleet Bounty Sharing is Enabled.");
        } else {
            sLog.Warning(" Delayed Bounties","Delay for Fleet Bounty Sharing is Disabled.  Fleet Sharing of Bounties is immediate.");
        }
    } else {
        sLog.Warning(" Delayed Bounties","Delayed Bounties are Disabled.  Bounty payouts are immediate.");
        if (sConfig.server.FleetShareDelayed) {
            sLog.Warning(" Delayed Bounties","Delayed Bounties are Disabled.  Fleet Sharing of Bounties is immediate.");
        }  else {
            sLog.Warning(" Delayed Bounties","Delay for Fleet Bounty Sharing is Disabled.  Fleet Sharing of Bounties is immediate.");
        }
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Misc Switches");
    if (sConfig.server.ModuleDamageChance) {
        sLog.Green("    Module Damage","Enabled.  Set to %i%% chance.", (int8)(sConfig.server.ModuleDamageChance *100));
    } else {
        sLog.Warning("    Module Damage","Disabled.");
    }
    if (sConfig.rates.WorldDecay) {
        sLog.Green("      Decay Timer","Enabled.  Checks every %u minutes", sConfig.rates.WorldDecay);
    }  else {
        sLog.Warning("      Decay Timer","Disabled.");
    }
    if (sConfig.server.TraderJoe) {
        sLog.Green("   Market Bot Mgr", "TraderJoe is Enabled.");
        /* create the MarketBot singleton */
        sLog.Green("       ServerInit", "Starting Market Bot Manager");
        sMktBotMgr.Initialize();
    } else {
        sLog.Warning("   Market Bot Mgr", "TraderJoe is Disabled.");
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Debug Switches");
    if (sConfig.debug.IsTestServer) {
        sLog.Error("     ServerConfig", "Test Server Enabled");
    }  else {
        sLog.Error("     ServerConfig", "Live Server Enabled");
    }
    if (sConfig.debug.StackTrace) {
        sLog.Warning("       StackTrace", "Enabled");
    }  else {
        sLog.Warning("       StackTrace", "Disabled");
    }
    if (sConfig.debug.UseProfiling) {
        sLog.Green(" Server Profiling","Enabled.");
        sProfiler.Initialize();
    } else {
        sLog.Warning(" Server Profiling","Disabled.");
    }
    if (sConfig.debug.BeanCount) {
        sLog.Green("     BeanCounting","Enabled.");
    } else {
        sLog.Warning("     BeanCounting","Disabled.");
    }
    if (sConfig.debug.SpawnTest) {
        sLog.Warning("       Spawn Test","Enabled.");
    }  else {
        sLog.Warning("       Spawn Test","Disabled.");
    }
    if (sConfig.debug.BubbleTrack) {
        sLog.Warning("  Bubble Tracking","Enabled.");
    }  else {
        sLog.Warning("  Bubble Tracking","Disabled.");
    }
    if (sConfig.debug.UseShipTracking) {
        sLog.Warning("    Ship Tracking","Enabled.");
    }  else {
        sLog.Warning("    Ship Tracking","Disabled.");
    }
    if (sConfig.debug.PositionHack) {
        sLog.Warning("    Position Hack","Enabled.");
    }  else {
        sLog.Warning("    Position Hack","Disabled.");
    }
    std::printf("\n");     // spacer
    