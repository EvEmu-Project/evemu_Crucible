/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
	------------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU Lesser General Public License as published by the Free Software
	Foundation; either version 2 of the License, or (at your option) any later
	version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place - Suite 330, Boston, MA 02111-1307, USA, or go to
	http://www.gnu.org/copyleft/lesser.txt.
	------------------------------------------------------------------------------------
	Author:		Zhur
*/

#include "EVEServerPCH.h"

ObjCacheDB::ObjCacheDB()
{
	//register all the generators
	m_generators["config.BulkData.billtypes"] = &ObjCacheDB::Generate_BillTypes;
	m_generators["config.BulkData.allianceshortnames"] = &ObjCacheDB::Generate_AllianceShortnames;
	m_generators["config.BulkData.categories"] = &ObjCacheDB::Generate_invCategories;
	m_generators["config.BulkData.invtypereactions"] = &ObjCacheDB::Generate_invTypeReactions;

	m_generators["config.BulkData.dgmtypeattribs"] = &ObjCacheDB::Generate_dgmTypeAttribs;
	m_generators["config.BulkData.dgmtypeeffects"] = &ObjCacheDB::Generate_dgmTypeEffects;
	m_generators["config.BulkData.dgmeffects"] = &ObjCacheDB::Generate_dgmEffects;
	m_generators["config.BulkData.dgmattribs"] = &ObjCacheDB::Generate_dgmAttribs;
	m_generators["config.BulkData.metagroups"] = &ObjCacheDB::Generate_invMetaGroups;
	m_generators["config.BulkData.ramactivities"] = &ObjCacheDB::Generate_ramActivities;
	m_generators["config.BulkData.ramaltypesdetailpergroup"] = &ObjCacheDB::Generate_ramALTypeGroup;
	m_generators["config.BulkData.ramaltypesdetailpercategory"] = &ObjCacheDB::Generate_ramALTypeCategory;
	m_generators["config.BulkData.ramaltypes"] = &ObjCacheDB::Generate_ramALTypes;
	m_generators["config.BulkData.ramcompletedstatuses"] = &ObjCacheDB::Generate_ramCompletedStatuses;
	m_generators["config.BulkData.ramtypematerials"] = &ObjCacheDB::Generate_ramTypeMaterials;
	m_generators["config.BulkData.ramtyperequirements"] = &ObjCacheDB::Generate_ramTypeRequirements;

	m_generators["config.BulkData.mapcelestialdescriptions"] = &ObjCacheDB::Generate_mapCelestialDescriptions;
	m_generators["config.BulkData.tickernames"] = &ObjCacheDB::Generate_tickerNames;
	m_generators["config.BulkData.groups"] = &ObjCacheDB::Generate_invGroups;
	m_generators["config.BulkData.certificates"] = &ObjCacheDB::Generate_certificates;
	m_generators["config.BulkData.certificaterelationships"] = &ObjCacheDB::Generate_certificateRelationships;
	m_generators["config.BulkData.shiptypes"] = &ObjCacheDB::Generate_invShipTypes;
	m_generators["config.BulkData.locations"] = &ObjCacheDB::Generate_cacheLocations;
	m_generators["config.BulkData.locationwormholeclasses"] = &ObjCacheDB::Generate_locationWormholeClasses;
	m_generators["config.BulkData.bptypes"] = &ObjCacheDB::Generate_invBlueprintTypes;
	m_generators["config.BulkData.graphics"] = &ObjCacheDB::Generate_eveGraphics;
	m_generators["config.BulkData.types"] = &ObjCacheDB::Generate_invTypes;
	m_generators["config.BulkData.invmetatypes"] = &ObjCacheDB::Generate_invMetaTypes;
	m_generators["config.Bloodlines"] = &ObjCacheDB::Generate_chrBloodlines;
	m_generators["config.Units"] = &ObjCacheDB::Generate_eveUnits;
	m_generators["config.BulkData.units"] = &ObjCacheDB::Generate_eveBulkDataUnits;
	m_generators["config.BulkData.owners"] = &ObjCacheDB::Generate_cacheOwners;
	m_generators["config.StaticOwners"] = &ObjCacheDB::Generate_eveStaticOwners;
	m_generators["config.Races"] = &ObjCacheDB::Generate_chrRaces;
	m_generators["config.Attributes"] = &ObjCacheDB::Generate_chrAttributes;
	m_generators["config.Flags"] = &ObjCacheDB::Generate_invFlags;
	m_generators["config.StaticLocations"] = &ObjCacheDB::Generate_eveStaticLocations;
	m_generators["config.InvContrabandTypes"] = &ObjCacheDB::Generate_invContrabandTypes;

	m_generators["charCreationInfo.bloodlines"] = &ObjCacheDB::Generate_c_chrBloodlines;
	m_generators["charCreationInfo.races"] = &ObjCacheDB::Generate_c_chrRaces;
	m_generators["charCreationInfo.ancestries"] = &ObjCacheDB::Generate_c_chrAncestries;
	m_generators["charCreationInfo.schools"] = &ObjCacheDB::Generate_c_chrSchools;
	m_generators["charCreationInfo.attributes"] = &ObjCacheDB::Generate_c_chrAttributes;
	m_generators["charCreationInfo.bl_accessories"] = &ObjCacheDB::Generate_bl_accessories;
	m_generators["charCreationInfo.bl_lights"] = &ObjCacheDB::Generate_bl_lights;
	m_generators["charCreationInfo.bl_skins"] = &ObjCacheDB::Generate_bl_skins;
	m_generators["charCreationInfo.bl_beards"] = &ObjCacheDB::Generate_bl_beards;
	m_generators["charCreationInfo.bl_eyes"] = &ObjCacheDB::Generate_bl_eyes;
	m_generators["charCreationInfo.bl_lipsticks"] = &ObjCacheDB::Generate_bl_lipsticks;
	m_generators["charCreationInfo.bl_makeups"] = &ObjCacheDB::Generate_bl_makeups;
	m_generators["charCreationInfo.bl_hairs"] = &ObjCacheDB::Generate_bl_hairs;
	m_generators["charCreationInfo.bl_backgrounds"] = &ObjCacheDB::Generate_bl_backgrounds;
	m_generators["charCreationInfo.bl_decos"] = &ObjCacheDB::Generate_bl_decos;
	m_generators["charCreationInfo.bl_eyebrows"] = &ObjCacheDB::Generate_bl_eyebrows;
	m_generators["charCreationInfo.bl_costumes"] = &ObjCacheDB::Generate_bl_costumes;

	m_generators["charCreationInfo.eyebrows"] = &ObjCacheDB::Generate_a_eyebrows;
	m_generators["charCreationInfo.eyes"] = &ObjCacheDB::Generate_a_eyes;
	m_generators["charCreationInfo.decos"] = &ObjCacheDB::Generate_a_decos;
	m_generators["charCreationInfo.hairs"] = &ObjCacheDB::Generate_a_hairs;
	m_generators["charCreationInfo.backgrounds"] = &ObjCacheDB::Generate_a_backgrounds;
	m_generators["charCreationInfo.accessories"] = &ObjCacheDB::Generate_a_accessories;
	m_generators["charCreationInfo.lights"] = &ObjCacheDB::Generate_a_lights;
	m_generators["charCreationInfo.costumes"] = &ObjCacheDB::Generate_a_costumes;
	m_generators["charCreationInfo.makeups"] = &ObjCacheDB::Generate_a_makeups;
	m_generators["charCreationInfo.beards"] = &ObjCacheDB::Generate_a_beards;
	m_generators["charCreationInfo.skins"] = &ObjCacheDB::Generate_a_skins;
	m_generators["charCreationInfo.lipsticks"] = &ObjCacheDB::Generate_a_lipsticks;

	m_generators["dogmaIM.attributesByName"] = &ObjCacheDB::Generate_dgmAttributesByName;
}

PyRep *ObjCacheDB::GetCachableObject(const std::string &type)
{
	std::map<std::string, genFunc>::const_iterator res;
	res = m_generators.find(type);
	
	if(res == m_generators.end())
	{
		_log(SERVICE__ERROR, "Unable to find cachable object generator for type '%s'", type.c_str());
		return NULL;
	}

	genFunc f = res->second;
	return (this->*f)();
}

//implement all the generators:
PyRep *ObjCacheDB::Generate_BillTypes()
{
	DBQueryResult res;
	const char *q = "SELECT billTypeID,billTypeName,description FROM billTypes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.billtypes': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_AllianceShortnames()
{
	DBQueryResult res;
	const char *q = "SELECT allianceID,shortName FROM alliance_shortnames";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.allianceshortnames': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_invCategories()
{
	DBQueryResult res;
	const char *q = "SELECT categoryID,categoryName,description,graphicID,published,0 AS dataID FROM invCategories";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.categories': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_invTypeReactions()
{
	DBQueryResult res;
	const char *q = "SELECT reactionTypeID,input,typeID,quantity FROM invTypeReactions";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.invtypereactions': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_dgmTypeAttribs()
{
	DBQueryResult res;
	const char *q = "SELECT	dgmTypeAttributes.typeID,	dgmTypeAttributes.attributeID,	IF(valueInt IS NULL, valueFloat, valueInt) AS value FROM dgmTypeAttributes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.dgmtypeattribs': %s",res.error.c_str());
		return NULL;
	}
	return(DBResultToPackedRowList(res));
}

PyRep *ObjCacheDB::Generate_dgmTypeEffects()
{
	DBQueryResult res;
	const char *q = "SELECT typeID,effectID,isDefault FROM dgmTypeEffects";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.dgmtypeeffects': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_dgmEffects()
{
	DBQueryResult res;
	const char *q = "SELECT effectID,effectName,effectCategory,preExpression,postExpression,description,guid,graphicID,isOffensive,isAssistance,durationAttributeID,trackingSpeedAttributeID,dischargeAttributeID,rangeAttributeID,falloffAttributeID,published,displayName,isWarpSafe,rangeChance,electronicChance,propulsionChance,distribution,sfxName,npcUsageChanceAttributeID,npcActivationChanceAttributeID,0 AS fittingUsageChanceAttributeID,0 AS dataID FROM dgmEffects";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.dgmeffects': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_dgmAttribs()
{
	DBQueryResult res;
	const char *q = "SELECT attributeID,attributeName,attributeCategory,description,maxAttributeID,attributeIdx,graphicID,chargeRechargeTimeID,defaultValue,published,displayName,unitID,stackable,highIsGood,categoryID,0 AS dataID FROM dgmAttributeTypes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.dgmattribs': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_invMetaGroups()
{
	DBQueryResult res;
	const char *q = "SELECT metaGroupID,metaGroupName,description,graphicID,0 AS dataID FROM invMetaGroups";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.metagroups': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_ramActivities()
{
	DBQueryResult res;
	const char *q = "SELECT activityID,activityName,iconNo,description,published FROM ramActivities";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.ramactivities': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_ramALTypeGroup()
{
	DBQueryResult res;
	const char *q = "SELECT a.assemblyLineTypeID,b.activityID,a.groupID,a.timeMultiplier,a.materialMultiplier FROM ramAssemblyLineTypeDetailPerGroup AS a LEFT JOIN ramAssemblyLineTypes AS b ON a.assemblyLineTypeID = b.assemblyLineTypeID";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.ramaltypesdetailpergroup': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_ramALTypeCategory()
{
	DBQueryResult res;
	const char *q = "SELECT a.assemblyLineTypeID,b.activityID,a.categoryID,a.timeMultiplier,a.materialMultiplier FROM ramAssemblyLineTypeDetailPerCategory AS a LEFT JOIN ramAssemblyLineTypes AS b ON a.assemblyLineTypeID = b.assemblyLineTypeID";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.ramaltypesdetailpercategory': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_ramALTypes()
{
	DBQueryResult res;
	const char *q = "SELECT assemblyLineTypeID,assemblyLineTypeName,assemblyLineTypeName AS typeName,description,activityID,baseTimeMultiplier,baseMaterialMultiplier,volume FROM ramAssemblyLineTypes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.ramaltypes': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_ramCompletedStatuses()
{
	DBQueryResult res;
	const char *q = "SELECT completedStatusID,completedStatusName,completedStatusText FROM ramCompletedStatuses";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.ramcompletedstatuses': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_ramTypeMaterials()
{
	DBQueryResult res;
	const char *q = "SELECT typeID, requiredTypeID AS materialTypeID, quantity FROM typeActivityMaterials WHERE activityID = 6 AND damagePerJob = 1.0 UNION SELECT productTypeID AS typeID, requiredTypeID AS materialTypeID, quantity FROM typeActivityMaterials JOIN invBlueprintTypes ON typeID = blueprintTypeID WHERE activityID = 1 AND damagePerJob = 1.0";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.ramtypematerials': %s",res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_ramTypeRequirements()
{
	DBQueryResult res;
	const char *q = "SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, recycle FROM typeActivityMaterials WHERE damagePerJob != 1.0 OR recycle = 1";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.ramtyperequirements': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_mapCelestialDescriptions()
{
	DBQueryResult res;
	const char *q = "SELECT celestialID,description FROM mapCelestialDescriptions";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.mapcelestialdescriptions': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_tickerNames()
{
	DBQueryResult res;
	const char *q = "SELECT corporationID,tickerName,shape1,shape2,shape3,color1,color2,color3 FROM corporation WHERE hasPlayerPersonnelManager=0";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.tickernames': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_invGroups()
{
	DBQueryResult res;
	const char *q = "SELECT groupID,categoryID,groupName,description,graphicID,useBasePrice,allowManufacture,allowRecycler,anchored,anchorable,fittableNonSingleton,1 AS published,0 AS dataID FROM invGroups";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.groups': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_certificates()
{
	DBQueryResult res;
	const char *q = "SELECT certificateID,categoryID,classID,grade,iconID,corpID,description,0 AS dataID FROM crtCertificates";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.groups': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_certificateRelationships()
{
	DBQueryResult res;
	const char *q = "SELECT relationshipID,parentID,parentTypeID,parentLevel,childID,childTypeID FROM crtRelationships";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.groups': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_invShipTypes()
{
	DBQueryResult res;
	const char *q = "SELECT shipTypeID,weaponTypeID,miningTypeID,skillTypeID FROM invShipTypes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.shiptypes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_cacheLocations()
{
	DBQueryResult res;
	const char *q = "SELECT locationID,locationName,x,y,z FROM cacheLocations";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.locations': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_locationWormholeClasses()
{
	DBQueryResult res;
	const char *q = "SELECT locationID,wormholeClassID FROM mapLocationWormholeClasses";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.locationwormholeclasses': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_invBlueprintTypes()
{
	DBQueryResult res;
	const char *q = "SELECT invTypes.typeName AS blueprintTypeName,invTypes.description,invTypes.graphicID,invTypes.basePrice,blueprintTypeID,parentBlueprintTypeID,productTypeID,productionTime,techLevel,researchProductivityTime,researchMaterialTime,researchCopyTime,researchTechTime,productivityModifier,materialModifier,wasteFactor,chanceOfReverseEngineering,maxProductionLimit FROM invBlueprintTypes,invTypes WHERE invBlueprintTypes.blueprintTypeID=invTypes.typeID";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.bptypes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_eveGraphics()
{
	DBQueryResult res;
	const char *q = "SELECT graphicID,url3D,urlWeb,icon,urlSound,explosionID FROM eveGraphics";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.graphics': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_invTypes()
{
	DBQueryResult res;
	const char *q = "SELECT typeID,groupID,typeName,description,graphicID,radius,mass,volume,capacity,portionSize,raceID,basePrice,published,marketGroupID,chanceOfDuplicating,0 AS dataID FROM invTypes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.types': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_invMetaTypes()
{
	DBQueryResult res;
	const char *q = "SELECT typeID,parentTypeID,metaGroupID FROM invMetaTypes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.invmetatypes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_chrBloodlines()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,bloodlineName,raceID,description,maleDescription,femaleDescription,shipTypeID,corporationID,perception,willpower,charisma,memory,intelligence,graphicID,shortDescription,shortMaleDescription,shortFemaleDescription,0 AS dataID FROM chrBloodlines";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.Bloodlines': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_eveUnits()
{
	DBQueryResult res;
	const char *q = "SELECT unitID,unitName,displayName FROM eveUnits";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.Units': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_eveBulkDataUnits()
{
	DBQueryResult res;
	const char *q = "SELECT unitID,unitName,displayName FROM eveUnits";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.units': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_cacheOwners()
{
	DBQueryResult res;
	const char *q = "SELECT ownerID,ownerName,typeID FROM cacheOwners";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.BulkData.owners': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToTupleSet(res);
}

PyRep *ObjCacheDB::Generate_eveStaticOwners()
{
	DBQueryResult res;
	const char *q = "SELECT ownerID,ownerName,typeID FROM eveStaticOwners";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.StaticOwners': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_chrRaces()
{
	DBQueryResult res;
	const char *q = "SELECT raceID,raceName,description,graphicID,shortDescription,0 AS dataID FROM chrRaces";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.Races': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_chrAttributes()
{
	DBQueryResult res;
	const char *q = "SELECT attributeID,attributeName,description,graphicID FROM chrAttributes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.Attributes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_invFlags()
{
	DBQueryResult res;
	const char *q = "SELECT flagID,flagName,flagText,flagType,orderID FROM invFlags";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.Flags': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_eveStaticLocations()
{
	DBQueryResult res;
	const char *q = "SELECT locationID,locationName,x,y,z FROM eveStaticLocations";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.StaticLocations': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_invContrabandTypes()
{
	DBQueryResult res;
	const char *q = "SELECT factionID,typeID,standingLoss,confiscateMinSec,fineByValue,attackMinSec FROM invContrabandTypes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'config.InvContrabandTypes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_c_chrBloodlines()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,bloodlineName,raceID,description,maleDescription,femaleDescription,shipTypeID,corporationID,perception,willpower,charisma,memory,intelligence,graphicID,shortDescription,shortMaleDescription,shortFemaleDescription,0 AS dataID FROM chrBloodlines";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bloodlines': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_c_chrRaces()
{
	DBQueryResult res;
	const char *q = "SELECT raceID,raceName,description,graphicID,shortDescription,0 AS dataID FROM chrRaces";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.races': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_c_chrAncestries()
{
	DBQueryResult res;
	const char *q = "SELECT ancestryID,ancestryName,bloodlineID,description,perception,willpower,charisma,memory,intelligence,graphicID,shortDescription,0 AS dataID FROM chrAncestries";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.ancestries': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_c_chrSchools()
{
	DBQueryResult res;
	const char *q = "SELECT raceID,schoolID,schoolName,description,graphicID,corporationID,agentID,newAgentID FROM chrSchools LEFT JOIN agtAgents USING (corporationID) GROUP BY schoolID";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.schools': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_c_chrAttributes()
{
	DBQueryResult res;
	const char *q = "SELECT attributeID,attributeName,description,graphicID FROM chrAttributes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.attributes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToCRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_accessories()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,accessoryID,npc FROM chrBLAccessories";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_accessories': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_lights()
{
	DBQueryResult res;
	const char *q = "SELECT lightID,lightName FROM chrBLLights";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_lights': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_skins()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,skinID,npc FROM chrBLSkins";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_skins': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_beards()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,beardID,npc FROM chrBLBeards";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_beards': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_eyes()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,eyesID,npc FROM chrBLEyes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_eyes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_lipsticks()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,lipstickID,npc FROM chrBLLipsticks";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_lipsticks': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_makeups()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,makeupID,npc FROM chrBLMakeups";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_makeups': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_hairs()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,hairID,npc FROM chrBLHairs";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_hairs': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_backgrounds()
{
	DBQueryResult res;
	const char *q = "SELECT backgroundID,backgroundName FROM chrBLBackgrounds";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_backgrounds': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_decos()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,decoID,npc FROM chrBLDecos";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_decos': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_eyebrows()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,eyebrowsID,npc FROM chrBLEyebrows";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_eyebrows': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_bl_costumes()
{
	DBQueryResult res;
	const char *q = "SELECT bloodlineID,gender,costumeID,npc FROM chrBLCostumes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.bl_costumes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_eyebrows()
{
	DBQueryResult res;
	const char *q = "SELECT eyebrowsID,eyebrowsName FROM chrEyebrows";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.eyebrows': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_eyes()
{
	DBQueryResult res;
	const char *q = "SELECT eyesID,eyesName FROM chrEyes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.eyes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_decos()
{
	DBQueryResult res;
	const char *q = "SELECT decoID,decoName FROM chrDecos";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.decos': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_hairs()
{
	DBQueryResult res;
	const char *q = "SELECT hairID,hairName FROM chrHairs";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.hairs': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_backgrounds()
{
	DBQueryResult res;
	const char *q = "SELECT backgroundID,backgroundName FROM chrBackgrounds";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.backgrounds': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_accessories()
{
	DBQueryResult res;
	const char *q = "SELECT accessoryID,accessoryName FROM chrAccessories";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.accessories': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_lights()
{
	DBQueryResult res;
	const char *q = "SELECT lightID,lightName FROM chrLights";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.lights': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_costumes()
{
	DBQueryResult res;
	const char *q = "SELECT costumeID,costumeName FROM chrCostumes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.costumes': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_makeups()
{
	DBQueryResult res;
	const char *q = "SELECT makeupID,makeupName FROM chrMakeups";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.makeups': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_beards()
{
	DBQueryResult res;
	const char *q = "SELECT beardID,beardName FROM chrBeards";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.beards': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_skins()
{
	DBQueryResult res;
	const char *q = "SELECT skinID,skinName FROM chrSkins";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.skins': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_a_lipsticks()
{
	DBQueryResult res;
	const char *q = "SELECT lipstickID,lipstickName FROM chrLipsticks";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'charCreationInfo.lipsticks': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToRowset(res);
}

PyRep *ObjCacheDB::Generate_dgmAttributesByName()
{
	DBQueryResult res;
	const char *q = "SELECT attributeID,attributeName,attributeCategory,description,maxAttributeID,attributeIdx,graphicID,chargeRechargeTimeID,defaultValue,published,displayName,unitID,stackable,highIsGood,categoryID,0 AS dataID FROM dgmAttributeTypes";
	if(sDatabase.RunQuery(res, q)==false)
	{
		_log(SERVICE__ERROR, "Error in query for cached object 'dogmaIM.attributesByName': %s", res.error.c_str());
		return NULL;
	}
	return DBResultToIndexRowset(res, "attributeName");
}
