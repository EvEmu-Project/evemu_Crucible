
#ifndef KCACHABLE
#define KCACHABLE(name, key, symbol, type, query)
#endif
#ifndef CACHABLE
#define CACHABLE(name, symbol, type, query) \
	KCACHABLE(name, name, symbol, type, query)
#endif
#ifndef ICACHABLE
#define ICACHABLE(name, symbol, type, key, query) \
	KCACHABLE(name, name, symbol, type, query)
#endif


CACHABLE("config.BulkData.billtypes", BillTypes, TupleSet,
	"SELECT billTypeID,billTypeName,description FROM billTypes"
);
CACHABLE("config.BulkData.allianceshortnames", AllianceShortnames, TupleSet,
	"SELECT allianceID,shortName FROM alliance_shortnames"
);
CACHABLE("config.BulkData.categories", invCategories, TupleSet,
	"SELECT categoryID,categoryName,description,graphicID,0 AS dataID FROM invCategories"
);
CACHABLE("config.BulkData.invtypereactions", invTypeReactions, TupleSet,
	"SELECT reactionTypeID,input,typeID,quantity FROM invTypeReactions"
);
/* this query is a strange one, we might have to develop a non-macro based
 * solution for building this one because it stretches all of the rules and is
 * requiring all kinds of special cases...
 *
 * I added the default value join in here to try to squeeze the size down a bit
 * (we are barely over the max packet size right now)... I have no idea how they
 * are determining what to include in the dgmTypeAttribs table (which has a
 * string value type, so it makes the client unhappy)... they must have some
 * sort of filter, but hell if I can figure it out...
 */
CACHABLE("config.BulkData.dgmtypeattribs", dgmTypeAttribs, Rowset,
	"SELECT"
	"	dgmTypeAttributes.typeID,"
	"	dgmTypeAttributes.attributeID,"
	"	IF(valueInt IS NULL, valueFloat, valueInt) AS value"
	" FROM dgmTypeAttributes"
	"	NATURAL JOIN dgmAttributeTypes"
	" WHERE"
//	"	defaultValue!=valueInt"
	"	published=1"	//not sure if this is right, but default value exclusion doesnt work with trinity
);
CACHABLE("config.BulkData.dgmtypeeffects", dgmTypeEffects, Rowset,
	"SELECT typeID,effectID,isDefault FROM dgmTypeEffects"
);
CACHABLE("config.BulkData.dgmeffects", dgmEffects, TupleSet,
	"SELECT effectID,effectName,effectCategory,preExpression,postExpression,description,guid,graphicID,isOffensive,isAssistance,durationAttributeID,trackingSpeedAttributeID,dischargeAttributeID,rangeAttributeID,falloffAttributeID,published,displayName,isWarpSafe,rangeChance,electronicChance,propulsionChance,distribution,sfxName,npcUsageChanceAttributeID,npcActivationChanceAttributeID,0 AS fittingUsageChanceAttributeID,0 AS dataID FROM dgmEffects"
);
CACHABLE("config.BulkData.dgmattribs", dgmAttribs, TupleSet,
	"SELECT attributeID,attributeName,attributeCategory,description,maxAttributeID,attributeIdx,graphicID,chargeRechargeTimeID,defaultValue,published,displayName,unitID,stackable,highIsGood,categoryID,0 AS dataID FROM dgmAttributeTypes"
);
CACHABLE("config.BulkData.metagroups", invMetaGroups, TupleSet,
	"SELECT metaGroupID,metaGroupName,description,graphicID,0 AS dataID FROM invMetaGroups"
);
CACHABLE("config.BulkData.ramactivities", ramActivities, TupleSet,
	"SELECT activityID,activityName,iconNo,description,published FROM ramActivities"
);
CACHABLE("config.BulkData.ramaltypesdetailpergroup", ramALTypeGroup, TupleSet,
	"SELECT a.assemblyLineTypeID,b.activityID,a.groupID,a.timeMultiplier,a.materialMultiplier FROM ramAssemblyLineTypeDetailPerGroup AS a LEFT JOIN ramAssemblyLineTypes AS b ON a.assemblyLineTypeID = b.assemblyLineTypeID"
);
CACHABLE("config.BulkData.ramaltypesdetailpercategory", ramALTypeCategory, TupleSet,
	"SELECT a.assemblyLineTypeID,b.activityID,a.categoryID,a.timeMultiplier,a.materialMultiplier FROM ramAssemblyLineTypeDetailPerCategory AS a LEFT JOIN ramAssemblyLineTypes AS b ON a.assemblyLineTypeID = b.assemblyLineTypeID"
);
CACHABLE("config.BulkData.ramaltypes", ramALTypes, TupleSet,
	"SELECT assemblyLineTypeID,assemblyLineTypeName,assemblyLineTypeName AS typeName,description,activityID,baseTimeMultiplier,baseMaterialMultiplier,volume FROM ramAssemblyLineTypes"
);
CACHABLE("config.BulkData.ramcompletedstatuses", ramCompletedStatuses, TupleSet,
	"SELECT completedStatusID,completedStatusName,completedStatusText FROM ramCompletedStatuses"
);
//returns what each item consists of
CACHABLE("config.BulkData.ramtypematerials", ramTypeMaterials, Rowset,
	"SELECT"
		" typeID,"
		" requiredTypeID AS materialTypeID,"
		" quantity"
	" FROM typeActivityMaterials"
		" WHERE activityID = 6"	//duplicating
		" AND damagePerJob = 1.0"
	" UNION"
	" SELECT"
		" productTypeID AS typeID,"
		" requiredTypeID AS materialTypeID,"
		" quantity"
	" FROM typeActivityMaterials"
	" JOIN invBlueprintTypes ON typeID = blueprintTypeID"
		" WHERE activityID = 1"	//manufacturing
		" AND damagePerJob = 1.0"
);
//no:
CACHABLE("config.BulkData.ramtyperequirements", ramTypeRequirements, Rowset,
	"SELECT"
		" typeID,"
		" activityID,"
		" requiredTypeID,"
		" quantity,"
		" damagePerJob,"
		" 0 AS recycle"	//hacking
	" FROM typeActivityMaterials"
		" WHERE damagePerJob != 1.0"
		//" OR recycle = 1"	//this is valid as well, but since we hack recycle it's not usable
	//as a blue.DBRow set
	/*
double damagePerJob;
uint32 activity;
uint32 quantity;
uint16 requiredTypeID;
uint16 typeID;
uint8 recycle;
    */
);
CACHABLE("config.BulkData.mapcelestialdescriptions", mapCelestialDescriptions, TupleSet,
	"SELECT celestialID,description FROM mapCelestialDescriptions"
);
CACHABLE("config.BulkData.tickernames", tickerNames, TupleSet,
	//npc corps only I think.
	"SELECT corporationID,tickerName,shape1,shape2,shape3,color1,color2,color3 FROM corporation WHERE hasPlayerPersonnelManager=0"
);
CACHABLE("config.BulkData.groups", invGroups, TupleSet,
	"SELECT groupID,categoryID,groupName,description,graphicID,useBasePrice,allowManufacture,allowRecycler,anchored,anchorable,fittableNonSingleton,1 AS published,0 AS dataID FROM invGroups"
);
CACHABLE("config.BulkData.shiptypes", invShipTypes, TupleSet,
	"SELECT shipTypeID,weaponTypeID,miningTypeID,skillTypeID FROM invShipTypes"
);
CACHABLE("config.BulkData.locations", cacheLocations, TupleSet,
	//this is such as worthless cache item...
	"SELECT locationID,locationName,x,y,z FROM cacheLocations"
);
CACHABLE("config.BulkData.bptypes", invBlueprintTypes, TupleSet,
	//old method used dedicated table invBlueprints
	//"SELECT blueprintTypeName,description,graphicID,basePrice,blueprintTypeID,parentBlueprintTypeID,productTypeID,productionTime,techLevel,researchProductivityTime,researchMaterialTime,researchCopyTime,researchTechTime,productivityModifier,materialModifier,wasteFactor,chanceOfReverseEngineering,maxProductionLimit FROM invBlueprints"
	//all data can be obtained from existing tables:
	"SELECT invTypes.typeName AS blueprintTypeName,invTypes.description,invTypes.graphicID,invTypes.basePrice,blueprintTypeID,parentBlueprintTypeID,productTypeID,productionTime,techLevel,researchProductivityTime,researchMaterialTime,researchCopyTime,researchTechTime,productivityModifier,materialModifier,wasteFactor,chanceOfReverseEngineering,maxProductionLimit FROM invBlueprintTypes,invTypes WHERE invBlueprintTypes.blueprintTypeID=invTypes.typeID"
);
CACHABLE("config.BulkData.graphics", eveGraphics, TupleSet,
	"SELECT graphicID,url3D,urlWeb,icon,urlSound,explosionID FROM eveGraphics"
);
CACHABLE("config.BulkData.types", invTypes, TupleSet,
	"SELECT typeID,groupID,typeName,description,graphicID,radius,mass,volume,capacity,portionSize,raceID,basePrice,published,marketGroupID,chanceOfDuplicating,0 AS dataID FROM invTypes"
);
CACHABLE("config.BulkData.invmetatypes", invMetaTypes, TupleSet,
	"SELECT typeID,parentTypeID,metaGroupID FROM invMetaTypes"
);
CACHABLE("config.Bloodlines", chrBloodlines, Rowset,
	"SELECT bloodlineID,bloodlineName,raceID,description,maleDescription,femaleDescription,shipTypeID,corporationID,perception,willpower,charisma,memory,intelligence,graphicID,shortDescription,shortMaleDescription,shortFemaleDescription,0 AS dataID FROM chrBloodlines"
);
CACHABLE("config.Units", eveUnits, Rowset,
	"SELECT unitID,unitName,displayName FROM eveUnits"
);
CACHABLE("config.BulkData.units", eveBulkDataUnits, TupleSet,
	"SELECT unitID,unitName,displayName FROM eveUnits"
);
CACHABLE("config.BulkData.owners", cacheOwners, TupleSet,
	"SELECT ownerID,ownerName,typeID FROM cacheOwners"
);
CACHABLE("config.StaticOwners", eveStaticOwners, Rowset,
	//im pretty sure this data should not be in its own table, but I cant figure out the filter on eveNames to get it.
	"SELECT ownerID,ownerName,typeID FROM eveStaticOwners"
);
CACHABLE("config.Races", chrRaces, Rowset,
	"SELECT raceID,raceName,description,graphicID,shortDescription,0 AS dataID FROM chrRaces"
);
CACHABLE("config.Attributes", chrAttributes, Rowset,
	"SELECT attributeID,attributeName,description,graphicID FROM chrAttributes"
);
CACHABLE("config.Flags", invFlags, Rowset,
	"SELECT flagID,flagName,flagText,flagType,orderID FROM invFlags"
);
CACHABLE("config.StaticLocations", eveStaticLocations, Rowset,
	//im pretty sure this data should not be in its own table, but I cant figure out the filter on mapDenormalize to get it.
	"SELECT locationID,locationName,x,y,z FROM eveStaticLocations"
);
CACHABLE("config.InvContrabandTypes", invContrabandTypes, Rowset,
	"SELECT factionID,typeID,standingLoss,confiscateMinSec,fineByValue,attackMinSec FROM invContrabandTypes"
);

//GetCharCreationInfo
KCACHABLE("charCreationInfo.bloodlines", "bloodlines", c_chrBloodlines, PackedRowset,
	"SELECT bloodlineID,bloodlineName,raceID,description,maleDescription,femaleDescription,shipTypeID,corporationID,perception,willpower,charisma,memory,intelligence,graphicID,shortDescription,shortMaleDescription,shortFemaleDescription,0 AS dataID FROM chrBloodlines"
);
KCACHABLE("charCreationInfo.races", "races", c_chrRaces, PackedRowset,
	"SELECT raceID,raceName,description,graphicID,shortDescription,0 AS dataID FROM chrRaces"
);
KCACHABLE("charCreationInfo.ancestries", "ancestries", c_chrAncestries, PackedRowset,
	"SELECT ancestryID,ancestryName,bloodlineID,description,perception,willpower,charisma,memory,intelligence,graphicID,shortDescription,0 AS dataID FROM chrAncestries"
);
KCACHABLE("charCreationInfo.schools", "schools", c_chrSchools, PackedRowset,
	"SELECT raceID,schoolID,schoolName,description,graphicID,corporationID,agentID,newAgentID FROM chrSchools"
);
KCACHABLE("charCreationInfo.attributes", "attributes", c_chrAttributes, PackedRowset,
	"SELECT attributeID,attributeName,description,graphicID FROM chrAttributes"
);
KCACHABLE("charCreationInfo.bl_accessories", "accessories", bl_accessories, Rowset,
	"SELECT bloodlineID,gender,accessoryID,npc FROM chrBLAccessories"
);
KCACHABLE("charCreationInfo.bl_lights", "lights", bl_lights, Rowset,
	"SELECT lightID,lightName FROM chrBLLights"
);
KCACHABLE("charCreationInfo.bl_skins", "skins", bl_skins, Rowset,
	"SELECT bloodlineID,gender,skinID,npc FROM chrBLSkins"
);
KCACHABLE("charCreationInfo.bl_beards", "beards", bl_beards, Rowset,
	"SELECT bloodlineID,gender,beardID,npc FROM chrBLBeards"
);
KCACHABLE("charCreationInfo.bl_eyes", "eyes", bl_eyes, Rowset,
	"SELECT bloodlineID,gender,eyesID,npc FROM chrBLEyes"
);
KCACHABLE("charCreationInfo.bl_lipsticks", "lipsticks", bl_lipsticks, Rowset,
	"SELECT bloodlineID,gender,lipstickID,npc FROM chrBLLipsticks"
);
KCACHABLE("charCreationInfo.bl_makeups", "makeups", bl_makeups, Rowset,
	"SELECT bloodlineID,gender,makeupID,npc FROM chrBLMakeups"
);
KCACHABLE("charCreationInfo.bl_hairs", "hairs", bl_hairs, Rowset,
	"SELECT bloodlineID,gender,hairID,npc FROM chrBLHairs"
);
KCACHABLE("charCreationInfo.bl_backgrounds", "backgrounds", bl_backgrounds, Rowset,
	"SELECT backgroundID,backgroundName FROM chrBLBackgrounds"
);
KCACHABLE("charCreationInfo.bl_decos", "decos", bl_decos, Rowset,
	"SELECT bloodlineID,gender,decoID,npc FROM chrBLDecos"
);
KCACHABLE("charCreationInfo.bl_eyebrows", "eyebrows", bl_eyebrows, Rowset,
	"SELECT bloodlineID,gender,eyebrowsID,npc FROM chrBLEyebrows"
);
KCACHABLE("charCreationInfo.bl_costumes", "costumes", bl_costumes, Rowset,
	"SELECT bloodlineID,gender,costumeID,npc FROM chrBLCostumes"
);

//GetCharNewExtraCreationInfo
KCACHABLE("charNewExtraCreationInfo.raceskills", "raceskills", nec_raceskills, Rowset,
	"SELECT raceID, skillTypeID, levels FROM chrRaceSkills"
);
KCACHABLE("charNewExtraCreationInfo.careerskills", "careerskills", nec_careerskills, Rowset,
	"SELECT careerID, skillTypeID, levels FROM chrCareerSkills"
);
KCACHABLE("charNewExtraCreationInfo.specialityskills", "specialityskills", nec_specialityskills, Rowset,
	"SELECT specialityID, skillTypeID, levels FROM chrCareerSpecialitySkills"
);
KCACHABLE("charNewExtraCreationInfo.careers", "careers", nec_careers, Rowset,
	"SELECT raceID, careerID, careerName, description, shortDescription, graphicID, schoolID, 0 AS dataID FROM chrCareers"
);
KCACHABLE("charNewExtraCreationInfo.specialities", "specialities", nec_specialities, Rowset,
	"SELECT specialityID, careerID, specialityName, description, shortDescription, graphicID, departmentID, 0 AS dataID FROM chrCareerSpecialities"
);

//GetAppearanceInfo
KCACHABLE("charCreationInfo.eyebrows", "eyebrows", a_eyebrows, Rowset,
	"SELECT eyebrowsID,eyebrowsName FROM chrEyebrows"
);
KCACHABLE("charCreationInfo.eyes", "eyes", a_eyes, Rowset,
	"SELECT eyesID,eyesName FROM chrEyes"
);
KCACHABLE("charCreationInfo.decos", "decos", a_decos, Rowset,
	"SELECT decoID,decoName FROM chrDecos"
);
KCACHABLE("charCreationInfo.hairs", "hairs", a_hairs, Rowset,
	"SELECT hairID,hairName FROM chrHairs"
);
KCACHABLE("charCreationInfo.backgrounds", "backgrounds", a_backgrounds, Rowset,
	"SELECT backgroundID,backgroundName FROM chrBackgrounds"
);
KCACHABLE("charCreationInfo.accessories", "accessories", a_accessories, Rowset,
	"SELECT accessoryID,accessoryName FROM chrAccessories"
);
KCACHABLE("charCreationInfo.lights", "lights", a_lights, Rowset,
	"SELECT lightID,lightName FROM chrLights"
);
KCACHABLE("charCreationInfo.costumes", "costumes", a_costumes, Rowset,
	"SELECT costumeID,costumeName FROM chrCostumes"
);
KCACHABLE("charCreationInfo.makeups", "makeups", a_makeups, Rowset,
	"SELECT makeupID,makeupName FROM chrMakeups"
);
KCACHABLE("charCreationInfo.beards", "beards", a_beards, Rowset,
	"SELECT beardID,beardName FROM chrBeards"
);
KCACHABLE("charCreationInfo.skins", "skins", a_skins, Rowset,
	"SELECT skinID,skinName FROM chrSkins"
);
KCACHABLE("charCreationInfo.lipsticks", "lipsticks", a_lipsticks, Rowset,
	"SELECT lipstickID,lipstickName FROM chrLipsticks"
);

//other
ICACHABLE("dogmaIM.attributesByName", dgmAttributesByName, IndexRowset, "attributeName",
	"SELECT attributeID,attributeName,attributeCategory,description,maxAttributeID,attributeIdx,graphicID,chargeRechargeTimeID,defaultValue,published,displayName,unitID,stackable,highIsGood,categoryID,0 AS dataID FROM dgmAttributeTypes"
);


/* Kludged cached method calls..
 *
 * Right now we can only cache method calls which do not take any arguments.
 *
 */
#if 0
CACHABLE("Method Call.server."
         "map.GetSolarSystemPseudoSecurities", 
          map_GetSolarSystemPseudoSecurities, Rowset,
		"SELECT solarSystemID, security FROM mapSolarSystems"
);
CACHABLE("Method Call.server."
         "agentMgr.GetAgents", 
          agentMgr_GetAgents, Rowset,
		"SELECT agentID,agentTypeID,divisionID,level,stationID,bloodlineID,quality,corporationID,gender"
		"	FROM agtAgents"
);
CACHABLE("Method Call.server."
         "account.GetRefTypes", 
          account_GetRefTypes, Rowset,
		"SELECT refTypeID,refTypeText,description FROM market_refTypes"
);
//Method Call.server.map.GetStationExtraInfo is too complex for here...
CACHABLE("Method Call.server."
         "account.GetKeyMap", 
          account_GetKeyMap, Rowset,
		"SELECT accountKey,accountType,accountName,description FROM market_keyMap"
);
CACHABLE("Method Call.server."
         "billMgr.GetBillTypes", 
          billMgr_GetBillTypes, Rowset,
		"SELECT billTypeID,billTypeName,description FROM billTypes"
);
CACHABLE("Method Call.server."
         "standing2.GetNPCNPCStandings", 
          standing2_GetNPCNPCStandings, Rowset,
		"SELECT fromID,toID,standing FROM npcStandings"
);
//Method Call.server.beyonce.GetFormations is too complex for here...
#endif


#undef CACHABLE
#undef KCACHABLE
#undef ICACHABLE

