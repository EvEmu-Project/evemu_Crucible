using System;
using System.Collections.Generic;
using EVEMon.XmlGenerator.StaticData;

namespace EVEMon.XmlGenerator
{
    public static class Database
    {
        private static EveStaticDataEntities s_entities;

        /// <summary>
        /// Makes the context available to all database methods without
        /// reinstantiating each time.
        /// </summary>
        private static EveStaticDataEntities Context
        {
            get
            {
                if (s_entities == null)
                    s_entities = new EveStaticDataEntities();

                return s_entities;
            }
        }

        /// <summary>
        /// EVE Agents
        /// </summary>
        /// <returns><c>Bag</c> of EVE Agents</returns>
        internal static Bag<AgtAgents> Agents()
        {
            var list = new IndexedList<AgtAgents>();

            foreach (agtAgents agent in Context.agtAgents)
            {
                var item = new AgtAgents
                               {
                                   ID = agent.agentID,
                                   LocationID = agent.locationID.Value,
                                   Level = agent.level.Value
                               };

                if (agent.quality.HasValue)
                    item.Quality = agent.quality.Value;

                list.Items.Add(item);
            }

            return new Bag<AgtAgents>(list);
        }

        /// <summary>
        /// EVE Names
        /// </summary>
        /// <returns><c>Bag</c> of EVE Names</returns>
        internal static Bag<EveNames> Names()
        {
            var list = new IndexedList<EveNames>();

            foreach (eveNames name in Context.eveNames)
            {
                var item = new EveNames
                               {
                                   ID = name.itemID,
                                   Name = name.itemName
                               };

                list.Items.Add(item);
            }

            return new Bag<EveNames>(list);
        }

        /// <summary>
        /// EVE Units
        /// </summary>
        /// <returns><c>Bag</c> of EVE Units</returns>
        internal static Bag<EveUnit> Units()
        {
            var list = new IndexedList<EveUnit>();

            foreach (eveUnits unit in Context.eveUnits)
            {
                var item = new EveUnit
                               {
                                   Description = unit.description.Clean(),
                                   DisplayName = unit.displayName.Clean(),
                                   ID = unit.unitID,
                                   Name = unit.unitName
                               };

                list.Items.Add(item);
            }

            return new Bag<EveUnit>(list);
        }

        /// <summary>
        /// EVE Icons
        /// </summary>
        /// <returns><c>Bag</c> of icons</returns>
        internal static Bag<EveIcons> Icons()
        {
            var list = new IndexedList<EveIcons>();

            foreach (eveIcons icon in Context.eveIcons)
            {
                var item = new EveIcons
                               {
                                   ID = icon.iconID,
                                   Icon = icon.iconFile
                               };

                list.Items.Add(item);
            }

            return new Bag<EveIcons>(list);
        }

        /// <summary>
        /// EVE Attributes
        /// </summary>
        /// <returns><c>Bag</c> of Attributes</returns>
        internal static Bag<DgmAttributeTypes> Attributes()
        {
            var list = new IndexedList<DgmAttributeTypes>();

            foreach (dgmAttributeTypes attribute in Context.dgmAttributeTypes)
            {
                var item = new DgmAttributeTypes
                               {
                                   ID = attribute.attributeID,
                                   CategoryID = attribute.categoryID,
                                   Description = attribute.description.Clean(),
                                   DisplayName = attribute.displayName.Clean(),
                                   IconID = attribute.iconID,
                                   Name = attribute.attributeName.Clean(),
                                   UnitID = attribute.unitID
                               };

                if (attribute.defaultValue.HasValue)
                    item.DefaultValue = attribute.defaultValue.Value.ToString();

                if (attribute.highIsGood.HasValue)
                    item.HigherIsBetter = attribute.highIsGood.Value;

                list.Items.Add(item);
            }

            return new Bag<DgmAttributeTypes>(list);
        }

        /// <summary>
        /// Attribute categories
        /// </summary>
        /// <returns><c>Bag</c> of Attribute Categories</returns>
        internal static Bag<DgmAttributeCategory> AttributeCategories()
        {
            var list = new IndexedList<DgmAttributeCategory>();

            foreach (dgmAttributeCategories category in Context.dgmAttributeCategories)
            {
                var item = new DgmAttributeCategory
                               {
                                   ID = category.categoryID,
                                   Description = category.categoryDescription.Clean(),
                                   Name = category.categoryName.Clean()
                               };

                list.Items.Add(item);
            }

            return new Bag<DgmAttributeCategory>(list);
        }

        /// <summary>
        /// Regions in the EVE Universe
        /// </summary>
        /// <returns><c>Bag</c> of all regions in EVE</returns>
        internal static Bag<MapRegion> Regions()
        {
            var list = new IndexedList<MapRegion>();

            foreach (mapRegions region in Context.mapRegions)
            {
                var item = new MapRegion
                               {
                                   ID = region.regionID,
                                   Name = region.regionName,
                                   FactionID = region.factionID
                               };

                list.Items.Add(item);
            }

            return new Bag<MapRegion>(list);
        }

        /// <summary>
        /// Constallations in the EVE Universe
        /// </summary>
        /// <returns><c>Bag</c> of Constallations in EVE</returns>
        internal static Bag<MapConstellation> Constellations()
        {
            var list = new IndexedList<MapConstellation>();

            foreach (mapConstellations constellation in Context.mapConstellations)
            {
                var item = new MapConstellation
                               {
                                   ID = constellation.constellationID,
                                   Name = constellation.constellationName,
                               };

                if (constellation.regionID.HasValue)
                    item.RegionID = constellation.regionID.Value;

                list.Items.Add(item);
            }

            return new Bag<MapConstellation>(list);
        }

        /// <summary>
        /// Solar Systems in EVE
        /// </summary>
        /// <returns><c>Bag</c> of Solar Systems in the EVE</returns>
        internal static Bag<MapSolarSystem> Solarsystems()
        {
            var list = new IndexedList<MapSolarSystem>();

            foreach (mapSolarSystems solarsystem in Context.mapSolarSystems)
            {
                var item = new MapSolarSystem
                               {
                                   ID = solarsystem.solarSystemID,
                                   Name = solarsystem.solarSystemName
                               };

                if (solarsystem.constellationID.HasValue)
                    item.ConstellationID = solarsystem.constellationID.Value;

                if (solarsystem.security.HasValue)
                    item.SecurityLevel = (float) solarsystem.security.Value;

                if (solarsystem.x.HasValue)
                    item.X = solarsystem.x.Value;

                if (solarsystem.y.HasValue)
                    item.Y = solarsystem.y.Value;

                if (solarsystem.z.HasValue)
                    item.Z = solarsystem.z.Value;

                list.Items.Add(item);
            }

            return new Bag<MapSolarSystem>(list);
        }

        /// <summary>
        /// Stations in the EVE Universe
        /// </summary>
        /// <returns><c>Bag</c> of Stations in the EVE Universe</returns>
        internal static Bag<StaStation> Stations()
        {
            var list = new IndexedList<StaStation>();

            foreach (staStations station in Context.staStations)
            {
                var item = new StaStation
                               {
                                   ID = station.stationID,
                                   Name = station.stationName,
                               };

                if (station.reprocessingEfficiency.HasValue)
                    item.ReprocessingEfficiency = (float) station.reprocessingEfficiency.Value;

                if (station.reprocessingStationsTake.HasValue)
                    item.ReprocessingStationsTake = (float) station.reprocessingStationsTake.Value;

                if (station.security.HasValue)
                    item.SecurityLevel = station.security.Value;

                if (station.solarSystemID.HasValue)
                    item.SolarSystemID = station.solarSystemID.Value;

                if (station.corporationID.HasValue)
                    item.CorporationID = station.corporationID.Value;

                list.Items.Add(item);
            }

            return new Bag<StaStation>(list);
        }

        /// <summary>
        /// Jumps between two solar systems in the EVE Universe
        /// </summary>
        /// <returns><c>List</c> of jumps between SolarSystems in EVE</returns>
        internal static List<MapSolarSystemJump> Jumps()
        {
            var list = new List<MapSolarSystemJump>();

            foreach (var jump in Context.mapSolarSystemJumps)
            {
                var item = new MapSolarSystemJump
                               {
                                   A = jump.fromSolarSystemID,
                                   B = jump.toSolarSystemID
                               };

                list.Add(item);
            }

            return list;
        }

        /// <summary>
        /// Inventory Blueprint Types
        /// </summary>
        /// <returns><c>Bag</c> of Inventory Blueprint Types</returns>
        internal static Bag<InvBlueprintTypes> BlueprintTypes()
        {
            var list = new IndexedList<InvBlueprintTypes>();

            foreach (var blueprint in Context.invBlueprintTypes)
            {
                var item = new InvBlueprintTypes
                               {
                                   ID = blueprint.blueprintTypeID,
                                   ParentID = blueprint.parentBlueprintTypeID,
                                   ProductTypeID = blueprint.productTypeID.Value,
                                   ProductionTime = blueprint.productionTime.Value,
                                   TechLevel = blueprint.techLevel.Value,
                                   ResearchProductivityTime = blueprint.researchProductivityTime.Value,
                                   ResearchMaterialTime = blueprint.researchMaterialTime.Value,
                                   ResearchCopyTime = blueprint.researchCopyTime.Value,
                                   ResearchTechTime = blueprint.researchTechTime.Value,
                                   ProductivityModifier = blueprint.productivityModifier.Value,
                                   WasteFactor = blueprint.wasteFactor.Value,
                                   MaxProductionLimit = blueprint.maxProductionLimit.Value
                               };

                list.Items.Add(item);
            }

            return new Bag<InvBlueprintTypes>(list);
        }

        /// <summary>
        /// Inventory Item Market Groups
        /// </summary>
        /// <returns><c>Bag</c> of Market Groups available on the market</returns>
        internal static Bag<InvMarketGroup> MarketGroups()
        {
            var list = new IndexedList<InvMarketGroup>();

            foreach (invMarketGroups marketGroup in Context.invMarketGroups)
            {
                var item = new InvMarketGroup
                               {
                                   ID = marketGroup.marketGroupID,
                                   Description = marketGroup.description.Clean(),
                                   IconID = marketGroup.iconID,
                                   Name = marketGroup.marketGroupName,
                                   ParentID = marketGroup.parentGroupID
                               };

                list.Items.Add(item);
            }

            return new Bag<InvMarketGroup>(list);
        }

        /// <summary>
        /// Inventory Item Groups
        /// </summary>
        /// <returns><c>Bag</c> of Inventory Groups</returns>
        internal static Bag<InvGroup> Groups()
        {
            var list = new IndexedList<InvGroup>();

            foreach (invGroups group in Context.invGroups)
            {
                var item = new InvGroup
                               {
                                   ID = group.groupID,
                                   Name = group.groupName,
                                   Published = group.published.Value
                               };

                if (group.categoryID.HasValue)
                    item.CategoryID = group.categoryID.Value;

                list.Items.Add(item);
            }

            return new Bag<InvGroup>(list);
        }

        /// <summary>
        /// Inventory Types
        /// </summary>
        /// <returns><c>Bag</c> of items from the Inventory</returns>
        internal static Bag<InvType> Types()
        {
            var list = new IndexedList<InvType>();

            foreach (invTypes type in Context.invTypes)
            {
                var item = new InvType
                               {
                                   ID = type.typeID,
                                   Description = type.description.Clean(),
                                   IconID = type.iconID,
                                   MarketGroupID = type.marketGroupID,
                                   Name = type.typeName,
                                   RaceID = type.raceID
                               };

                if (type.basePrice.HasValue)
                    item.BasePrice = type.basePrice.Value;

                if (type.capacity.HasValue)
                    item.Capacity = type.capacity.Value;

                if (type.groupID.HasValue)
                    item.GroupID = type.groupID.Value;

                if (type.mass.HasValue)
                    item.Mass = type.mass.Value;

                if (type.published.HasValue)
                    item.Published = type.published.Value;

                if (type.volume.HasValue)
                    item.Volume = type.volume.Value;

                list.Items.Add(item);
            }

            return new Bag<InvType>(list);
        }

        /// <summary>
        /// Requirements used for an Activity
        /// </summary>
        /// <returns>List of Requirements needed for a particular activity.</returns>
        internal static List<RamTypeRequirements> TypeRequirements()
        {
            var list = new List<RamTypeRequirements>();

            foreach (ramTypeRequirements requirement in Context.ramTypeRequirements)
            {
                var item = new RamTypeRequirements
                               {
                                   TypeID = requirement.typeID,
                                   ActivityID = requirement.activityID,
                                   RequiredTypeID = requirement.requiredTypeID
                               };

                if (requirement.quantity.HasValue)
                    item.Quantity = requirement.quantity.Value;

                if (requirement.damagePerJob.HasValue)
                    item.DamagePerJob = requirement.damagePerJob.Value;

                if (requirement.recycle.HasValue)
                    item.Recyclable = requirement.recycle.Value;

                list.Add(item);
            }

            return list;
        }

        /// <summary>
        /// Materials 
        /// </summary>
        /// <returns>List of Materials.</returns>
        internal static List<InvTypeMaterials> TypeMaterials()
        {
            var list = new List<InvTypeMaterials>();

            foreach (var material in Context.invTypeMaterials)
            {
                var item = new InvTypeMaterials
                               {
                                   TypeID = material.typeID,
                                   MaterialTypeID = material.materialTypeID,
                                   Quantity = material.quantity
                               };

                list.Add(item);
            }

            return list;
        }

        /// <summary>
        /// Certificate Categories
        /// </summary>
        /// <returns><c>Bag</c> of Certificate Categories</returns>
        internal static Bag<CrtCategories> CertificateCategories()
        {
            var list = new IndexedList<CrtCategories>();

            foreach (crtCategories category in Context.crtCategories)
            {
                var item = new CrtCategories
                               {
                                   ID = category.categoryID,
                                   CategoryName = category.categoryName,
                                   Description = category.description.Clean()
                               };

                list.Items.Add(item);
            }

            return new Bag<CrtCategories>(list);
        }

        /// <summary>
        /// Certificate Classes
        /// </summary>
        /// <returns><c>Bag</c> of Classes of Certificate</returns>
        internal static Bag<CrtClasses> CertificateClasses()
        {
            var list = new IndexedList<CrtClasses>();

            foreach (crtClasses cClass in Context.crtClasses)
            {
                var item = new CrtClasses
                               {
                                   ID = cClass.classID,
                                   ClassName = cClass.className,
                                   Description = cClass.description.Clean()
                               };

                list.Items.Add(item);
            }

            return new Bag<CrtClasses>(list);
        }

        /// <summary>
        /// Certificates
        /// </summary>
        /// <returns><c>Bag</c> of Certificates</returns>
        internal static Bag<CrtCertificates> Certificates()
        {
            var list = new IndexedList<CrtCertificates>();

            foreach (crtCertificates certificate in Context.crtCertificates)
            {
                var item = new CrtCertificates
                               {
                                   ID = certificate.certificateID,
                                   Description = certificate.description.Clean()
                               };

                if (certificate.categoryID.HasValue)
                    item.CategoryID = certificate.categoryID.Value;

                if (certificate.classID.HasValue)
                    item.ClassID = certificate.classID.Value;

                if (certificate.grade.HasValue)
                    item.Grade = certificate.grade.Value;

                list.Items.Add(item);
            }

            return new Bag<CrtCertificates>(list);
        }

        /// <summary>
        /// Certificate Recommendations
        /// </summary>
        /// <returns><c>Bag</c> of Certificate Recommendations</returns>
        internal static Bag<CrtRecommendations> CertificateRecommendations()
        {
            var list = new IndexedList<CrtRecommendations>();

            foreach (crtRecommendations recommendation in Context.crtRecommendations)
            {
                var item = new CrtRecommendations
                               {
                                   ID = recommendation.recommendationID,
                                   Level = recommendation.recommendationLevel,
                               };

                if (recommendation.certificateID.HasValue)
                    item.CertificateID = recommendation.certificateID.Value;

                if (recommendation.shipTypeID.HasValue)
                    item.ShipTypeID = recommendation.shipTypeID.Value;

                list.Items.Add(item);
            }

            return new Bag<CrtRecommendations>(list);
        }

        /// <summary>
        /// Certificate Relationships
        /// </summary>
        /// <returns><c>Bag</c> of parent-child relationships between certificates.</returns>
        internal static Bag<CrtRelationships> CertificateRelationships()
        {
            var list = new IndexedList<CrtRelationships>();

            foreach (crtRelationships relationship in Context.crtRelationships)
            {
                var item = new CrtRelationships
                               {
                                   ID = relationship.relationshipID,
                                   ParentID = relationship.parentID,
                                   ParentLevel = relationship.parentLevel,
                               };

                if (relationship.parentTypeID != 0)
                    item.ParentTypeID = relationship.parentTypeID;

                if (relationship.childID.HasValue)
                    item.ChildID = relationship.childID.Value;

                list.Items.Add(item);
            }

            return new Bag<CrtRelationships>(list);
        }

        /// <summary>
        /// Type Attribes
        /// </summary>
        /// <returns><c>RelationSet</c> of attributes for types</returns>
        internal static RelationSet<DgmTypeAttribute> TypeAttributes()
        {
            var list = new List<DgmTypeAttribute>();

            foreach (var typeAttribute in Context.dgmTypeAttributes)
            {
                var item = new DgmTypeAttribute
                               {
                                   AttributeID = typeAttribute.attributeID,
                                   ItemID = typeAttribute.typeID,
                                   ValueFloat = typeAttribute.valueFloat,
                                   ValueInt = typeAttribute.valueInt
                               };

                list.Add(item);
            }

            return new RelationSet<DgmTypeAttribute>(list);
        }

        /// <summary>
        /// Meta Types
        /// </summary>
        /// <returns><c>RelationSet</c> parent-child relationships between types</returns>
        internal static RelationSet<InvMetaType> MetaTypes()
        {
            var list = new List<InvMetaType>();

            foreach (var metaType in Context.invMetaTypes)
            {
                var item = new InvMetaType
                               {
                                   ItemID = metaType.typeID,
                                   MetaGroupID = Convert.ToInt32(metaType.metaGroupID),
                                   ParentItemID = Convert.ToInt32(metaType.parentTypeID)
                               };

                list.Add(item);
            }

            return new RelationSet<InvMetaType>(list);
        }

        /// <summary>
        /// Effects of various types
        /// </summary>
        /// <returns><c>RelationSet</c> of Types and Effects</returns>
        internal static RelationSet<DgmTypeEffect> TypeEffects()
        {
            var list = new List<DgmTypeEffect>();

            foreach (var typeEffect in Context.dgmTypeEffects)
            {
                var item = new DgmTypeEffect
                               {
                                   EffectID = typeEffect.effectID,
                                   ItemID = typeEffect.typeID
                               };

                list.Add(item);
            }

            return new RelationSet<DgmTypeEffect>(list);
        }
    }
}