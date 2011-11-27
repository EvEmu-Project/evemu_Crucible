using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Threading;

using EVEMon.Common;
using EVEMon.Common.Data;
using EVEMon.Common.Serialization.Datafiles;
using EVEMon.XmlGenerator.StaticData;

namespace EVEMon.XmlGenerator
{
    internal class Program
    {
        private static string s_text = String.Empty;
        private static double s_counter;
        private static double s_tablesCount;
        private static double s_totalTablesCount = 25;

        private static int s_percentOld;
        private static int s_propGenTotal = 1547;
        private static int s_itemGenTotal = 8713;
        private static int s_skillGenTotal = 454;
        private static int s_certGenTotal = 4272;
        private static int s_blueprintGenTotal = 3945;
        private static int s_geoGen = 5219;
        private static int s_geoGenTotal = 19501;
        private static int s_reprocessGenTotal = 10032;

        private static DateTime s_startTime;
        private static DateTime s_endTime;

        private static int s_propBasePriceID;
        private static List<InvMarketGroup> s_injectedMarketGroups;

        private static Bag<AgtAgents> s_agents;
        private static Bag<EveUnit> s_units;
        private static Bag<EveNames> s_names;
        private static Bag<EveIcons> s_icons;
        private static Bag<DgmAttributeTypes> s_attributeTypes;
        private static Bag<DgmAttributeCategory> s_attributeCategories;
        private static RelationSet<DgmTypeEffect> s_typeEffects;
        private static RelationSet<DgmTypeAttribute> s_typeAttributes;
        private static Bag<MapRegion> s_regions;
        private static Bag<MapConstellation> s_constellations;
        private static Bag<MapSolarSystem> s_solarSystems;
        private static Bag<StaStation> s_stations;
        private static List<MapSolarSystemJump> s_jumps;
        private static Bag<InvBlueprintTypes> s_blueprintTypes;
        private static Bag<InvMarketGroup> s_marketGroups;
        private static Bag<InvGroup> s_groups;
        private static Bag<InvType> s_types;
        private static RelationSet<InvMetaType> s_metaTypes;
        private static List<InvTypeMaterials> s_typeMaterials;
        private static List<RamTypeRequirements> s_typeRequirements;
        private static Bag<CrtCategories> s_crtCategories;
        private static Bag<CrtClasses> s_crtClasses;
        private static Bag<CrtCertificates> s_certificates;
        private static Bag<CrtRecommendations> s_crtRecommendations;
        private static Bag<CrtRelationships> s_crtRelationships;

        #region Initilization

        private static void Main(string[] args)
        {
            // Setting a standard format for the generated files
            Thread.CurrentThread.CurrentCulture = new CultureInfo("en-US");

            // Data dumps are available from CCP
            Console.Write(@"Loading Data from SQL Server... ");

            #region Read Tables From Database

            // Read tables from database
            s_agents = Database.Agents();
            UpdateProgress();
            s_units = Database.Units();
            UpdateProgress();
            s_names = Database.Names();
            UpdateProgress();
            s_icons = Database.Icons();
            UpdateProgress();
            s_attributeTypes = Database.Attributes();
            UpdateProgress();
            s_attributeCategories = Database.AttributeCategories();
            UpdateProgress();
            s_regions = Database.Regions();
            UpdateProgress();
            s_constellations = Database.Constellations();
            UpdateProgress();
            s_solarSystems = Database.Solarsystems();
            UpdateProgress();
            s_stations = Database.Stations();
            UpdateProgress();
            s_jumps = Database.Jumps();
            UpdateProgress();
            s_typeAttributes = Database.TypeAttributes();
            UpdateProgress();
            s_blueprintTypes = Database.BlueprintTypes();
            UpdateProgress();
            s_marketGroups = Database.MarketGroups();
            UpdateProgress();
            s_groups = Database.Groups();
            UpdateProgress();
            s_metaTypes = Database.MetaTypes();
            UpdateProgress();
            s_typeEffects = Database.TypeEffects();
            UpdateProgress();
            s_types = Database.Types();
            UpdateProgress();
            s_typeMaterials = Database.TypeMaterials();
            UpdateProgress();
            s_typeRequirements = Database.TypeRequirements();
            UpdateProgress();
            s_crtCategories = Database.CertificateCategories();
            UpdateProgress();
            s_crtClasses = Database.CertificateClasses();
            UpdateProgress();
            s_certificates = Database.Certificates();
            UpdateProgress();
            s_crtRecommendations = Database.CertificateRecommendations();
            UpdateProgress();
            s_crtRelationships = Database.CertificateRelationships();
            UpdateProgress();

            Console.WriteLine();
            Console.WriteLine();

            #endregion

            // Generate datafiles
            Console.WriteLine(@"Datafile Generating In Progress");
            Console.WriteLine();

            GenerateProperties();
            GenerateItems(); // Requires GenerateProperties()
            GenerateSkills();
            GenerateCertificates();
            GenerateBlueprints();
            GenerateGeography();
            GenerateReprocessing(); // Requires GenerateItems()

            GenerateMD5Sums();

            Console.WriteLine(@"Done");
            Console.ReadLine();
        }

        #endregion

        #region Properties Datafile

        /// <summary>
        /// Generate the properties datafile.
        /// </summary>
        private static void GenerateProperties()
        {
            Console.WriteLine();
            Console.Write(@"Generated properties datafile... ");

            s_counter = 0;
            s_percentOld = 0;
            s_text = String.Empty;
            s_startTime = DateTime.Now;

            int newID = 0;

            // Change some display names and default values
            s_attributeTypes[9].DisplayName = "Structure HP";
            s_attributeTypes[263].DisplayName = "Shield HP";
            s_attributeTypes[265].DisplayName = "Armor HP";

            s_attributeTypes[38].DisplayName = "Cargo Capacity";
            s_attributeTypes[48].DisplayName = "CPU";
            s_attributeTypes[11].DisplayName = "Powergrid";

            // Shield
            s_attributeTypes[271].DisplayName = "EM Resistance";
            s_attributeTypes[272].DisplayName = "Explosive Resistance";
            s_attributeTypes[273].DisplayName = "Kinetic Resistance";
            s_attributeTypes[274].DisplayName = "Thermal Resistance";

            // Armor
            s_attributeTypes[267].DisplayName = "EM Resistance";
            s_attributeTypes[268].DisplayName = "Explosive Resistance";
            s_attributeTypes[269].DisplayName = "Kinetic Resistance";
            s_attributeTypes[270].DisplayName = "Thermal Resistance";

            // Hull
            s_attributeTypes[974].DisplayName = "EM Resistance";
            s_attributeTypes[975].DisplayName = "Explosive Resistance";
            s_attributeTypes[976].DisplayName = "Kinetic Resistance";
            s_attributeTypes[977].DisplayName = "Thermal Resistance";

            // Items attribute
            s_attributeTypes[6].DisplayName = "Activation cost";
            s_attributeTypes[30].DisplayName = "Powergrid usage";
            s_attributeTypes[68].DisplayName = "Shield Bonus";
            s_attributeTypes[87].DisplayName = "Shield Transfer Range";
            s_attributeTypes[116].DisplayName = "Explosive damage";
            s_attributeTypes[424].DisplayName = "CPU Output Bonus";
            s_attributeTypes[1082].DisplayName = "CPU Penalty";

            // Changing the categoryID for some attributes 
            s_attributeTypes[1132].CategoryID = 1; // Calibration
            s_attributeTypes[1547].CategoryID = 1; // Rig Size
            s_attributeTypes[908].CategoryID = 4; // Ship Maintenance Bay Capacity
            s_attributeTypes[1692].CategoryID = 9; // MetaGroup of type

            // Changing HigherIsBetter to false (CCP has this wrong?)
            s_attributeTypes[30].HigherIsBetter = false; // CPU usage
            s_attributeTypes[50].HigherIsBetter = false; // PG usage
            s_attributeTypes[161].HigherIsBetter = false; // Volume
            s_attributeTypes[70].HigherIsBetter = false; // Inertia Modifier
            s_attributeTypes[4].HigherIsBetter = false; // Mass
            s_attributeTypes[6].HigherIsBetter = false; // Activation Cost
            s_attributeTypes[55].HigherIsBetter = false; // Capacitor recharge time
            s_attributeTypes[144].HigherIsBetter = false;
                // Capacitor recharge rate bonus (reversed due to values deriving from substraction)
            s_attributeTypes[479].HigherIsBetter = false; // Shield recharge time
            s_attributeTypes[552].HigherIsBetter = false; // Signature radius
            s_attributeTypes[560].HigherIsBetter = false; // Sensor Recalibration Time
            s_attributeTypes[1082].HigherIsBetter = false; // CPU Penalty
            s_attributeTypes[1153].HigherIsBetter = false; // Calibration cost
            s_attributeTypes[1272].HigherIsBetter = false; // Bandwidth Needed
            s_attributeTypes[1416].HigherIsBetter = false; // Target Switch Timer
            s_attributeTypes[73].HigherIsBetter = false; // Activation time / duration
            s_attributeTypes[556].HigherIsBetter = false; // Anchoring Delay
            s_attributeTypes[676].HigherIsBetter = false; // Unanchoring Delay
            s_attributeTypes[677].HigherIsBetter = false; // Onlining Delay
            s_attributeTypes[780].HigherIsBetter = false; // Cycle Time bonus
            s_attributeTypes[669].HigherIsBetter = false; // Reactivation Delay


            // Export attribute categories
            var categories = new List<SerializablePropertyCategory>();

            // We insert custom categories
            var general = new SerializablePropertyCategory {Name = "General", Description = "General informations"};
            var propulsion = new SerializablePropertyCategory
                                 {Name = "Propulsion", Description = "Navigation attributes for ships"};
            var gProperties = new List<SerializableProperty>();
            var pProperties = new List<SerializableProperty>();
            categories.Insert(0, general);
            categories.Insert(0, propulsion);

            foreach (DgmAttributeCategory srcCategory in s_attributeCategories)
            {
                var category = new SerializablePropertyCategory();
                categories.Add(category);

                category.Description = srcCategory.Description;
                category.Name = srcCategory.Name;

                // Export attributes
                var properties = new List<SerializableProperty>();

                foreach (var srcProp in s_attributeTypes.Where(x => x.CategoryID == srcCategory.ID))
                {
                    UpdatePercentDone(s_propGenTotal);

                    var prop = new SerializableProperty();
                    properties.Add(prop);

                    prop.DefaultValue = srcProp.DefaultValue;
                    prop.Description = srcProp.Description;
                    prop.HigherIsBetter = srcProp.HigherIsBetter;
                    prop.Name = (String.IsNullOrEmpty(srcProp.DisplayName) ? srcProp.Name : srcProp.DisplayName);
                    prop.ID = srcProp.ID;

                    // Unit
                    if (srcProp.UnitID == null)
                    {
                        prop.Unit = String.Empty;
                    }
                    else
                    {
                        prop.Unit = s_units[srcProp.UnitID.Value].DisplayName;
                        prop.UnitID = srcProp.UnitID.Value;
                    }

                    // Ship warp speed unit
                    if (srcProp.ID == DBConstants.ShipWarpSpeedPropertyID)
                        prop.Unit = "AU/S";

                    // Icon
                    prop.Icon = (srcProp.IconID.HasValue ? s_icons[srcProp.IconID.Value].Icon : String.Empty);

                    // Reordering some properties
                    int index = properties.IndexOf(prop);
                    switch (srcProp.ID)
                    {
                        case 9:
                            properties.Insert(0, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 37:
                            pProperties.Insert(0, prop);
                            properties.RemoveAt(index);
                            break;
                        case 38:
                            properties.Insert(1, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 48:
                            properties.Insert(0, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 70:
                            properties.Insert(3, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 161:
                            properties.Insert(3, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 422:
                            gProperties.Insert(0, prop);
                            properties.RemoveAt(index);
                            break;
                        case 479:
                            properties.Insert(6, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 482:
                            properties.Insert(0, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 564:
                            properties.Insert(4, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 633:
                            gProperties.Insert(1, prop);
                            properties.RemoveAt(index);
                            break;
                        case 974:
                            properties.Insert(5, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 975:
                            properties.Insert(6, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 976:
                            properties.Insert(7, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 977:
                            properties.Insert(8, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 1132:
                            properties.Insert(2, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 1137:
                            properties.Insert(10, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        case 1281:
                            pProperties.Insert(1, prop);
                            properties.RemoveAt(index);
                            break;
                        case 1547:
                            properties.Insert(11, prop);
                            properties.RemoveAt(index + 1);
                            break;
                        default:
                            break;
                    }

                    // New ID
                    newID = Math.Max(newID, srcProp.ID);
                }
                category.Properties = properties.ToArray();
            }

            // Add EVEMon custom properties (Base Price)
            var gprop = new SerializableProperty();
            s_propBasePriceID = newID + 1;
            gProperties.Insert(0, gprop);
            gprop.ID = s_propBasePriceID;
            gprop.Name = "Base Price";
            gprop.Unit = "ISK";
            gprop.Description = "The price from NPC vendors (does not mean there is any).";

            // Add properties to custom categories
            general.Properties = gProperties.ToArray();
            propulsion.Properties = pProperties.ToArray();

            // Sort groups
            string[] orderedGroupNames = {
                                             "General", "Fitting", "Drones", "Structure", "Armor", "Shield", "Capacitor",
                                             "Targeting", "Propulsion", "Miscellaneous", "NULL"
                                         };

            s_endTime = DateTime.Now;
            Console.WriteLine(String.Format(" in {0}", s_endTime.Subtract(s_startTime)).TrimEnd('0'));

            // Serialize
            var datafile = new PropertiesDatafile();
            datafile.Categories =
                categories.OrderBy(x => Array.IndexOf(orderedGroupNames, String.Intern(x.Name))).ToArray();
            Util.SerializeXML(datafile, DatafileConstants.PropertiesDatafile);
        }

        #endregion

        #region Items Datafile

        /// <summary>
        /// Generate the items datafile.
        /// </summary>
        private static void GenerateItems()
        {
            Console.WriteLine();
            Console.Write(@"Generated items datafile... ");

            s_counter = 0;
            s_percentOld = 0;
            s_text = String.Empty;
            s_startTime = DateTime.Now;


            // Create custom market groups that don't exist in EVE
            s_injectedMarketGroups = new List<InvMarketGroup>
                                         {
                                             new InvMarketGroup
                                                 {
                                                     Name = "Various Non-Market",
                                                     Description = "Non-Market Items",
                                                     ID = DBConstants.RootNonMarketGroupID,
                                                     ParentID = null,
                                                     IconID = null
                                                 },
                                             new InvMarketGroup
                                                 {
                                                     Name = "Unique Designs",
                                                     Description = "Ships of a unique design",
                                                     ID = DBConstants.RootUniqueDesignsGroupID,
                                                     ParentID = DBConstants.ShipsGroupID,
                                                     IconID = 1443
                                                 },
                                             new InvMarketGroup
                                                 {
                                                     Name = "Unique Shuttles",
                                                     Description = "Fast ships of a unique design",
                                                     ID = DBConstants.UniqueDesignShuttlesGroupID,
                                                     ParentID = DBConstants.RootUniqueDesignsGroupID,
                                                     IconID = null
                                                 },
                                             new InvMarketGroup
                                                 {
                                                     Name = "Unique Battleships",
                                                     Description = "Battleships ships of a unique design",
                                                     ID = DBConstants.UniqueDesignBattleshipsGroupID,
                                                     ParentID = DBConstants.RootUniqueDesignsGroupID,
                                                     IconID = null
                                                 },
                                         };

            // Manually set some items attributes
            s_types[11].Published = true;
            s_types[12].Published = true;
            s_types[13].Published = true;
            s_types[2014].Published = true;
            s_types[2015].Published = true;
            s_types[2016].Published = true;
            s_types[2017].Published = true;
            s_types[2063].Published = true;
            s_types[30889].Published = true;
            s_types[27029].Published = true;
            s_types[32077].Published = true;
            
            // Set some attributes to items because their MarketGroupID is NULL
            foreach (InvType srcItem in s_types.Where(x => x.Published && x.MarketGroupID == null))
            {
                // Set some ships market group and race
                switch (srcItem.ID)
                {
                    case 13202: // Megathron Federate Issue
                    case 26840: // Raven State Issue
                    case 26842: // Tempest Tribal Issue
                        srcItem.MarketGroupID = DBConstants.UniqueDesignBattleshipsGroupID;
                        srcItem.RaceID = (int)Race.Faction;
                        break;
                    case 21097: // Goru's Shuttle
                    case 21628: // Guristas Shuttle
                    case 30842: // Interbus Shuttle
                        srcItem.MarketGroupID = DBConstants.UniqueDesignShuttlesGroupID;
                        srcItem.RaceID = (int)Race.Faction;
                        break;

                        // Set some items market group to support blueprints
                    case 21815:
                    case 23882:
                    case 24289:
                    case 27029:
                    case 27038:
                    case 29203:
                    case 29226:
                    case 10260:
                    case 19758:
                    case 10257:
                    case 10258:
                    case 32077:
                    case 30424:
                    case 30839:
                    case 30422:
                    case 30342:
                    case 30420:
                    case 30328:
                    case 21054:
                    case 17761:
                    case 23883:
                    case 29202:
                    case 21510:
                    case 22335:
                    case 22333:
                    case 22337:
                    case 22339:
                    case 22923:
                    case 22329:
                    case 22327:
                    case 22325:
                    case 22331:
                        srcItem.MarketGroupID = DBConstants.RootNonMarketGroupID;
                        break;
                }

                // Set some items market group to support blueprints
                if (srcItem.ID > 28685 && srcItem.ID < 28694)
                    srcItem.MarketGroupID = DBConstants.RootNonMarketGroupID;

                // Adding planets to support attribute browsing for command centers
                if (srcItem.GroupID == DBConstants.PlanetGroupID)
                    srcItem.MarketGroupID = DBConstants.RootNonMarketGroupID;
            }

            var groups = new Dictionary<int, SerializableMarketGroup>();

            // Create the market groups
            foreach (InvMarketGroup srcGroup in s_marketGroups.Concat(s_injectedMarketGroups))
            {
                var group = new SerializableMarketGroup {ID = srcGroup.ID, Name = srcGroup.Name};
                groups[srcGroup.ID] = group;

                // Add the items in this group
                var items = new List<SerializableItem>();
                foreach (InvType srcItem in s_types
                    .Where(x => x.Published && (x.MarketGroupID.GetValueOrDefault() == srcGroup.ID)))
                {
                    CreateItem(srcItem, items);
                }

                // If this is an implant group, we add the implants with no market groups in this one.
                if (srcGroup.ParentID == DBConstants.SkillHardwiringImplantGroupID || srcGroup.ParentID == DBConstants.AttributeEnhancersImplantsGroupID)
                {
                    string slotString = srcGroup.Name.Substring("Implant Slot ".Length);
                    int slot = Int32.Parse(slotString);

                    // Enumerate all implants without market groups
                    foreach (InvType srcItem in s_types
                        .Where(x => x.MarketGroupID == null 
                        && s_groups[x.GroupID].CategoryID == DBConstants.ImplantCategoryID 
                        && x.GroupID != DBConstants.CyberLearningImplantsGroupID)
                        )
                    {
                        // Check the slot matches
                        DgmTypeAttribute slotAttrib = s_typeAttributes.Get(srcItem.ID, DBConstants.ImplantSlotPropertyID);
                        if (slotAttrib != null && slotAttrib.GetIntValue() == slot)
                            CreateItem(srcItem, items);
                    }
                }

                // Store the items
                group.Items = items.OrderBy(x => x.Name).ToArray();
            }

            // Create the parent-children groups relations
            foreach (SerializableMarketGroup group in groups.Values)
            {
                IEnumerable<SerializableMarketGroup> children =
                    s_marketGroups.Concat(s_injectedMarketGroups).Where(x => x.ParentID.GetValueOrDefault() == group.ID)
                        .Select(x => groups[x.ID]);
                group.SubGroups = children.OrderBy(x => x.Name).ToArray();
            }

            // Pick the family
            SetItemFamilyByMarketGroup(groups[DBConstants.BlueprintsGroupID], ItemFamily.Bpo);
            SetItemFamilyByMarketGroup(groups[DBConstants.ShipsGroupID], ItemFamily.Ship);
            SetItemFamilyByMarketGroup(groups[DBConstants.ImplantsGroupID], ItemFamily.Implant);
            SetItemFamilyByMarketGroup(groups[DBConstants.DronesGroupID], ItemFamily.Drone);
            SetItemFamilyByMarketGroup(groups[DBConstants.StarbaseStructuresGroupID], ItemFamily.StarbaseStructure);

            // Sort groups
            IOrderedEnumerable<SerializableMarketGroup> rootGroups =
                s_marketGroups.Concat(s_injectedMarketGroups).Where(x => !x.ParentID.HasValue).Select(x => groups[x.ID])
                    .OrderBy(x => x.Name);

            s_endTime = DateTime.Now;
            Console.WriteLine(String.Format(" in {0}", s_endTime.Subtract(s_startTime)).TrimEnd('0'));

            // Serialize
            var datafile = new ItemsDatafile();
            datafile.MarketGroups = rootGroups.ToArray();
            Util.SerializeXML(datafile, DatafileConstants.ItemsDatafile);
        }

        /// <summary>
        /// Add properties to an item.
        /// </summary>
        /// <param name="srcItem"></param>
        /// <param name="groupItems"></param>
        /// <returns></returns>
        private static void CreateItem(InvType srcItem, List<SerializableItem> groupItems)
        {
            UpdatePercentDone(s_itemGenTotal);

            srcItem.Generated = true;

            // Creates the item with base informations
            var item = new SerializableItem
                           {
                               ID = srcItem.ID,
                               Name = srcItem.Name,
                               Description = srcItem.Description
                           };

            // Icon
            item.Icon = (srcItem.IconID.HasValue ? s_icons[srcItem.IconID.Value].Icon : String.Empty);
            
            // Initialize item metagroup
            item.MetaGroup = ItemMetaGroup.Empty;

            // Add the properties and prereqs
            int baseWarpSpeed = 3;
            double warpSpeedMultiplier = 1;
            var props = new List<SerializablePropertyValue>();
            var prereqSkills = new int[DBConstants.RequiredSkillPropertyIDs.Length];
            var prereqLevels = new int[DBConstants.RequiredSkillPropertyIDs.Length];
            foreach (DgmTypeAttribute srcProp in s_typeAttributes.Where(x => x.ItemID == srcItem.ID))
            {
                var propIntValue = (srcProp.ValueInt.HasValue ? srcProp.ValueInt.Value : (int) srcProp.ValueFloat.Value);

                // Is it a prereq skill ?
                int prereqIndex = Array.IndexOf(DBConstants.RequiredSkillPropertyIDs, srcProp.AttributeID);
                if (prereqIndex >= 0)
                {
                    prereqSkills[prereqIndex] = propIntValue;
                    continue;
                }

                // Is it a prereq level ?
                prereqIndex = Array.IndexOf(DBConstants.RequiredSkillLevelPropertyIDs, srcProp.AttributeID);
                if (prereqIndex >= 0)
                {
                    prereqLevels[prereqIndex] = propIntValue;
                    continue;
                }

                // Launcher group ?
                int launcherIndex = Array.IndexOf(DBConstants.LauncherGroupIDs, srcProp.AttributeID);
                if (launcherIndex >= 0)
                {
                    props.Add(new SerializablePropertyValue
                                    {
                                        ID = srcProp.AttributeID,
                                        Value = s_groups[propIntValue].Name
                                    });
                    continue;
                }

                // Charge group ?
                int chargeIndex = Array.IndexOf(DBConstants.ChargeGroupIDs, srcProp.AttributeID);
                if (chargeIndex >= 0)
                {
                    props.Add(new SerializablePropertyValue
                                    {
                                        ID = srcProp.AttributeID,
                                        Value = s_groups[propIntValue].Name
                                    });
                    continue;
                }

                // CanFitShip group ?
                int canFitShipIndex = Array.IndexOf(DBConstants.CanFitShipGroupIDs, srcProp.AttributeID);
                if (canFitShipIndex >= 0)
                {
                    props.Add(new SerializablePropertyValue
                                    {
                                        ID = srcProp.AttributeID,
                                        Value = s_groups[propIntValue].Name
                                    });
                    continue;
                }

                // ModuleShip group ?
                int moduleShipIndex = Array.IndexOf(DBConstants.ModuleShipGroupIDs, srcProp.AttributeID);
                if (moduleShipIndex >= 0)
                {
                    props.Add(new SerializablePropertyValue
                                    {
                                        ID = srcProp.AttributeID,
                                        Value = s_groups[propIntValue].Name
                                    });
                    continue;
                }

                // SpecialisationAsteroid group ?
                int specialisationAsteroidIndex = Array.IndexOf(DBConstants.SpecialisationAsteroidGroupIDs,
                                                                srcProp.AttributeID);
                if (specialisationAsteroidIndex >= 0)
                {
                    props.Add(new SerializablePropertyValue
                                    {
                                        ID = srcProp.AttributeID,
                                        Value = s_groups[propIntValue].Name
                                    });
                    continue;
                }

                // Reaction group ?
                int reactionIndex = Array.IndexOf(DBConstants.ReactionGroupIDs, srcProp.AttributeID);
                if (reactionIndex >= 0)
                {
                    props.Add(new SerializablePropertyValue
                                    {
                                        ID = srcProp.AttributeID,
                                        Value = s_groups[propIntValue].Name
                                    });
                    continue;
                }

                // PosCargobayAccept group ?
                int posCargobayAcceptIndex = Array.IndexOf(DBConstants.PosCargobayAcceptGroupIDs, srcProp.AttributeID);
                if (posCargobayAcceptIndex >= 0)
                {
                    props.Add(new SerializablePropertyValue
                                    {
                                        ID = srcProp.AttributeID,
                                        Value = s_groups[propIntValue].Name
                                    });
                    continue;
                }

                // Get the warp speed multiplier
                if (srcProp.AttributeID == DBConstants.WarpSpeedMultiplierPropertyID)
                    warpSpeedMultiplier = srcProp.ValueFloat.Value;

                // We calculate the Ships Warp Speed
                if (srcProp.AttributeID == DBConstants.ShipWarpSpeedPropertyID)
                    props.Add(new SerializablePropertyValue
                                    {ID = srcProp.AttributeID, Value = (baseWarpSpeed*warpSpeedMultiplier).ToString()});

                // Other props
                props.Add(new SerializablePropertyValue
                                {ID = srcProp.AttributeID, Value = srcProp.FormatPropertyValue()});

                // Is metalevel property ?
                if (srcProp.AttributeID == DBConstants.MetaLevelPropertyID)
                    item.MetaLevel = propIntValue;
                
                // Is techlevel property ?
                if (srcProp.AttributeID == DBConstants.TechLevelPropertyID)
                {
                    switch (propIntValue)
                    {
                        default:
                        case 1:
                            item.MetaGroup = ItemMetaGroup.T1;
                            break;
                        case 2:
                            item.MetaGroup = ItemMetaGroup.T2;
                            break;
                        case 3:
                            item.MetaGroup = ItemMetaGroup.T3;
                            break;
                    }
                }

                // Is metagroup property ?
                if (srcProp.AttributeID == DBConstants.MetaGroupPropertyID)
                {
                    switch (propIntValue)
                    {
                        case 3:
                            item.MetaGroup = ItemMetaGroup.Storyline;
                            break;
                        case 4:
                            item.MetaGroup = ItemMetaGroup.Faction;
                            break;
                        case 5:
                            item.MetaGroup = ItemMetaGroup.Officer;
                            break;
                        case 6:
                            item.MetaGroup = ItemMetaGroup.Deadspace;
                            break;
                        default:
                            item.MetaGroup = ItemMetaGroup.None;
                            break;
                    }
                }
            }

            // Ensures there is a mass and add it to prop
            if (srcItem.Mass != 0)
                props.Add(new SerializablePropertyValue
                              {ID = DBConstants.MassPropertyID, Value = srcItem.Mass.ToString()});

            // Ensures there is a cargo capacity and add it to prop
            if (srcItem.Capacity != 0)
                props.Add(new SerializablePropertyValue
                              {ID = DBConstants.CargoCapacityPropertyID, Value = srcItem.Capacity.ToString()});

            // Ensures there is a volume and add it to prop
            if (srcItem.Volume != 0)
                props.Add(new SerializablePropertyValue
                              {ID = DBConstants.VolumePropertyID, Value = srcItem.Volume.ToString()});

            // Add base price as a prop
            props.Add(new SerializablePropertyValue {ID = s_propBasePriceID, Value = srcItem.BasePrice.FormatDecimal()});

            // Add properties info to item
            item.Properties = props.ToArray();

            // Prerequisites completion
            var prereqs = new List<SerializablePrerequisiteSkill>();
            for (int i = 0; i < prereqSkills.Length; i++)
            {
                if (prereqSkills[i] != 0)
                    prereqs.Add(new SerializablePrerequisiteSkill {ID = prereqSkills[i], Level = prereqLevels[i]});
            }

            // Add prerequisite skills info to item
            item.Prereqs = prereqs.ToArray();

            // Metagroup
            foreach (InvMetaType relation in s_metaTypes
                .Where(x => x.ItemID == srcItem.ID && item.MetaGroup == ItemMetaGroup.Empty))
            {
                switch (relation.MetaGroupID)
                {
                    case 1:
                        item.MetaGroup = ItemMetaGroup.T1;
                        break;
                    case 2:
                        item.MetaGroup = ItemMetaGroup.T2;
                        break;
                    case 3:
                        item.MetaGroup = ItemMetaGroup.Storyline;
                        break;
                    case 4:
                        item.MetaGroup = ItemMetaGroup.Faction;
                        break;
                    case 5:
                        item.MetaGroup = ItemMetaGroup.Officer;
                        break;
                    case 6:
                        item.MetaGroup = ItemMetaGroup.Deadspace;
                        break;
                    case 14:
                        item.MetaGroup = ItemMetaGroup.T3;
                        break;
                    default:
                        item.MetaGroup = ItemMetaGroup.None;
                        break;
                }
            }

            if (item.MetaGroup == ItemMetaGroup.Empty)
                item.MetaGroup = ItemMetaGroup.T1;

            // Race ID
            item.Race = (Race) Enum.ToObject(typeof (Race), (srcItem.RaceID == null ? 0 : srcItem.RaceID));
            
            // Set race to Faction if item race is Jovian
            if (item.Race == Race.Jove)
                item.Race = Race.Faction;

            // Set race to ORE if it is in the ORE market groups
            // within mining barges, exhumers, industrial or capital industrial ships
            if (srcItem.MarketGroupID == DBConstants.MiningBargesGroupID
                || srcItem.MarketGroupID == DBConstants.ExhumersGroupID
                || srcItem.MarketGroupID == DBConstants.IndustrialsGroupID
                || srcItem.MarketGroupID == DBConstants.CapitalIndustrialsGroupID)
                item.Race = Race.Ore;

            // Set race to Faction if ship has Pirate Faction property
            foreach (SerializablePropertyValue prop in props)
            {
                if (prop.ID == DBConstants.ShipBonusPirateFactionPropertyID)
                    item.Race = Race.Faction;
            }

            // Look for slots
            if (s_typeEffects.Contains(srcItem.ID, DBConstants.LowSlotEffectID))
            {
                item.Slot = ItemSlot.Low;
            }
            else if (s_typeEffects.Contains(srcItem.ID, DBConstants.MedSlotEffectID))
            {
                item.Slot = ItemSlot.Medium;
            }
            else if (s_typeEffects.Contains(srcItem.ID, DBConstants.HiSlotEffectID))
            {
                item.Slot = ItemSlot.High;
            }
            else
            {
                item.Slot = ItemSlot.None;
            }

            // Add this item
            groupItems.Add(item);

            // If the current item isn't in a market group then we are done
            if (srcItem.MarketGroupID == null)
                return;

            // Look for variations which are not in any market group
            foreach (InvMetaType variation in s_metaTypes.Where(x => x.ParentItemID == srcItem.ID))
            {
                InvType srcVariationItem = s_types[variation.ItemID];
                if (srcVariationItem.Published && srcVariationItem.MarketGroupID == null)
                {
                    srcVariationItem.RaceID = (int)Race.Faction;
                    CreateItem(srcVariationItem, groupItems);
                }
            }
        }

        /// <summary>
        /// Sets the item family according to its market group
        /// </summary>
        /// <param name="group"></param>
        /// <param name="itemFamily"></param>
        private static void SetItemFamilyByMarketGroup(SerializableMarketGroup group, ItemFamily itemFamily)
        {
            foreach (SerializableItem item in group.Items)
            {
                item.Family = itemFamily;
            }

            foreach (SerializableMarketGroup childGroup in group.SubGroups)
            {
                SetItemFamilyByMarketGroup(childGroup, itemFamily);
            }
        }

        #endregion

        #region Skills Datafile

        /// <summary>
        /// Generate the skills datafile.
        /// </summary>
        private static void GenerateSkills()
        {
            Console.WriteLine();
            Console.Write(@"Generated skills datafile... ");

            s_counter = 0;
            s_percentOld = 0;
            s_text = String.Empty;
            s_startTime = DateTime.Now;

            // Export skill groups
            var listOfSkillGroups = new List<SerializableSkillGroup>();

            foreach (InvGroup group in s_groups.Where(x => x.CategoryID == 16 && x.ID != 505).OrderBy(x => x.Name))
            {
                var skillGroup = new SerializableSkillGroup
                                     {
                                         ID = group.ID,
                                         Name = group.Name,
                                     };

                // Export skills
                var listOfSkillsInGroup = new List<SerializableSkill>();

                foreach (InvType skill in s_types.Where(x => x.GroupID == group.ID))
                {
                    UpdatePercentDone(s_skillGenTotal);

                    var singleSkill = new SerializableSkill
                                          {
                                              ID = skill.ID,
                                              Name = skill.Name,
                                              Description = skill.Description,
                                              Public = skill.Published,
                                              Cost = (long) skill.BasePrice,
                                          };

                    // Export skill atributes
                    var skillAttributes = new Dictionary<int, int>();

                    foreach (DgmTypeAttribute attribute in s_typeAttributes.Where(x => x.ItemID == skill.ID))
                    {
                        skillAttributes.Add(attribute.AttributeID, attribute.GetIntValue());
                    }

                    if (skillAttributes.ContainsKey(275) && skillAttributes[275] > 0)
                    {
                        singleSkill.Rank = skillAttributes[275];
                    }
                    else
                    {
                        singleSkill.Rank = 1;
                    }

                    singleSkill.PrimaryAttribute = skillAttributes.ContainsKey(180)
                                                       ? IntToEveAttribute(skillAttributes[180])
                                                       : EveAttribute.None;
                    singleSkill.SecondaryAttribute = skillAttributes.ContainsKey(181)
                                                         ? IntToEveAttribute(skillAttributes[181])
                                                         : EveAttribute.None;
                    singleSkill.CanTrainOnTrial = skillAttributes.ContainsKey(1047) && skillAttributes[1047] == 0
                                                      ? true
                                                      : false;

                    // Export prerequesities
                    var listOfPrerequisites = new List<SerializableSkillPrerequisite>();

                    for (int i = 0; i < DBConstants.RequiredSkillPropertyIDs.Length; i++)
                    {
                        if (skillAttributes.ContainsKey(DBConstants.RequiredSkillPropertyIDs[i]) &&
                            skillAttributes.ContainsKey(DBConstants.RequiredSkillLevelPropertyIDs[i]))
                        {
                            var j = i;
                            InvType prereqSkill =
                                s_types.First(x => x.ID == skillAttributes[DBConstants.RequiredSkillPropertyIDs[j]]);

                            var preReq = new SerializableSkillPrerequisite
                                             {
                                                 ID = prereqSkill.ID,
                                                 Level = skillAttributes[DBConstants.RequiredSkillLevelPropertyIDs[i]],
                                             };

                            if (prereqSkill != null)
                                preReq.Name = prereqSkill.Name;

                            // Add prerequisites
                            listOfPrerequisites.Add(preReq);
                        }
                    }

                    // Add prerequesites to skill
                    singleSkill.Prereqs = listOfPrerequisites.ToArray();

                    // Add skill
                    listOfSkillsInGroup.Add(singleSkill);
                }

                // Add skills in skill group
                skillGroup.Skills = listOfSkillsInGroup.OrderBy(x => x.Name).ToArray();

                // Add skill group
                listOfSkillGroups.Add(skillGroup);
            }

            s_endTime = DateTime.Now;
            Console.WriteLine(String.Format(" in {0}", s_endTime.Subtract(s_startTime)).TrimEnd('0'));

            // Serialize
            var datafile = new SkillsDatafile();
            datafile.Groups = listOfSkillGroups.ToArray();
            Util.SerializeXML(datafile, DatafileConstants.SkillsDatafile);
        }

        private static EveAttribute IntToEveAttribute(int attributeValue)
        {
            switch (attributeValue)
            {
                case 164:
                    return EveAttribute.Charisma;
                case 165:
                    return EveAttribute.Intelligence;
                case 166:
                    return EveAttribute.Memory;
                case 167:
                    return EveAttribute.Perception;
                case 168:
                    return EveAttribute.Willpower;
                default:
                    return EveAttribute.None;
            }
        }

        #endregion

        #region Certificates Datafile

        /// <summary>
        /// Generate the certificates datafile.
        /// </summary>        
        private static void GenerateCertificates()
        {
            Console.WriteLine();
            Console.Write(@"Generated certificates datafile... ");

            s_counter = 0;
            s_percentOld = 0;
            s_text = String.Empty;
            s_startTime = DateTime.Now;

            // Export certificates categories
            var listOfCertCategories = new List<SerializableCertificateCategory>();

            foreach (CrtCategories category in s_crtCategories.OrderBy(x => x.CategoryName))
            {
                var crtCategory = new SerializableCertificateCategory
                                      {
                                          ID = category.ID,
                                          Name = category.CategoryName,
                                          Description = category.Description
                                      };

                // Export certificates classes
                var listOfCertClasses = new List<SerializableCertificateClass>();

                int categoryID = 0;
                foreach (CrtClasses certClass in s_crtClasses)
                {
                    // Exclude unused classes
                    int id = certClass.ID;
                    if (id == 104 || id == 106 || id == 111)
                        continue;

                    var crtClasses = new SerializableCertificateClass
                                         {
                                             ID = certClass.ID,
                                             Name = certClass.ClassName,
                                             Description = certClass.Description
                                         };

                    // Export certificates
                    var listOfCertificates = new List<SerializableCertificate>();

                    foreach (var certificate in s_certificates.Where(x => x.ClassID == certClass.ID))
                    {
                        UpdatePercentDone(s_certGenTotal);

                        var crtCertificates = new SerializableCertificate
                                                  {
                                                      ID = certificate.ID,
                                                      Grade = GetGrade(certificate.Grade),
                                                      Description = certificate.Description
                                                  };

                        // Export prerequesities
                        var listOfPrereq = new List<SerializableCertificatePrerequisite>();

                        foreach (CrtRelationships relationship in s_crtRelationships
                            .Where(x => x.ChildID == certificate.ID))
                        {
                            var crtPrerequisites = new SerializableCertificatePrerequisite
                                                       {
                                                           ID = relationship.ID,
                                                       };


                            if (relationship.ParentTypeID != null) // prereq is a skill
                            {
                                var skill = s_types.First(x => x.ID == relationship.ParentTypeID);
                                crtPrerequisites.Kind = SerializableCertificatePrerequisiteKind.Skill;
                                crtPrerequisites.Name = skill.Name;
                                crtPrerequisites.Level = relationship.ParentLevel.ToString();
                            }
                            else // prereq is a certificate
                            {
                                CrtCertificates cert = s_certificates.First(x => x.ID == relationship.ParentID);
                                CrtClasses crtClass = s_crtClasses.First(x => x.ID == cert.ClassID);
                                crtPrerequisites.Kind = SerializableCertificatePrerequisiteKind.Certificate;
                                crtPrerequisites.Name = crtClass.ClassName;
                                crtPrerequisites.Level = GetGrade(cert.Grade).ToString();
                            }

                            // Add prerequisite
                            listOfPrereq.Add(crtPrerequisites);
                        }

                        // Export recommendations
                        var listOfRecom = new List<SerializableCertificateRecommendation>();

                        foreach (CrtRecommendations recommendation in s_crtRecommendations
                            .Where(x => x.CertificateID == certificate.ID))
                        {
                            // Finds the ships name
                            InvType shipName = s_types.First(x => x.ID == recommendation.ShipTypeID);

                            var crtRecommendations = new SerializableCertificateRecommendation
                                                         {
                                                             ID = recommendation.ID,
                                                             Ship = shipName.Name,
                                                             Level = recommendation.Level
                                                         };

                            // Add recommendation
                            listOfRecom.Add(crtRecommendations);
                        }


                        //Add prerequisites to certificate
                        crtCertificates.Prerequisites = listOfPrereq.ToArray();

                        // Add recommendations to certificate
                        crtCertificates.Recommendations = listOfRecom.ToArray();

                        // Add certificate
                        listOfCertificates.Add(crtCertificates);

                        // Storing the certificate categoryID for use in classes
                        categoryID = certificate.CategoryID;
                    }

                    // Grouping certificates according to their classes
                    if (categoryID == category.ID)
                    {
                        // Add certificates to classes
                        crtClasses.Certificates = listOfCertificates.OrderBy(x => x.Grade).ToArray();

                        // Add certificate class
                        listOfCertClasses.Add(crtClasses);
                    }
                }

                // Add classes to categories
                crtCategory.Classes = listOfCertClasses.ToArray();

                // Add category
                listOfCertCategories.Add(crtCategory);
            }

            s_endTime = DateTime.Now;
            Console.WriteLine(String.Format(" in {0}", s_endTime.Subtract(s_startTime)).TrimEnd('0'));

            // Serialize
            var datafile = new CertificatesDatafile();
            datafile.Categories = listOfCertCategories.ToArray();
            Util.SerializeXML(datafile, DatafileConstants.CertificatesDatafile);
        }

        /// <summary>
        /// Gets the certificates Grade.
        /// </summary>        
        private static CertificateGrade GetGrade(int gradeValue)
        {
            switch (gradeValue)
            {
                case 1:
                    return CertificateGrade.Basic;
                case 2:
                    return CertificateGrade.Standard;
                case 3:
                    return CertificateGrade.Improved;
                case 5:
                    return CertificateGrade.Elite;
                default:
                    throw new NotImplementedException();
            }
        }

        #endregion

        #region Blueprints Datafile

        /// <summary>
        /// Generate the skills datafile.
        /// </summary>
        private static void GenerateBlueprints()
        {
            Console.WriteLine();
            Console.Write(@"Generated blueprints datafile... ");

            s_counter = 0;
            s_percentOld = 0;
            s_text = String.Empty;
            s_startTime = DateTime.Now;

            // Configure blueprints with Null market group
            ConfigureNullMarketBlueprint();

            var groups = new Dictionary<int, SerializableBlueprintGroup>();

            // Export blueprint groups           
            foreach (InvMarketGroup marketGroup in s_marketGroups.Concat(s_injectedMarketGroups))
            {
                var group = new SerializableBlueprintGroup
                                {
                                    ID = marketGroup.ID,
                                    Name = marketGroup.Name,
                                };

                groups[marketGroup.ID] = group;

                // Add the items in this group
                var blueprints = new List<SerializableBlueprint>();
                foreach (InvType item in s_types.Where(x => x.MarketGroupID == marketGroup.ID
                                       && s_groups[x.GroupID].CategoryID == DBConstants.BlueprintCategoryID
                                       && s_groups[x.GroupID].Published))
                {
                    CreateBlueprint(item, blueprints);
                }

                // Store the items
                group.Blueprints = blueprints.OrderBy(x => x.Name).ToArray();
            }

            // Create the parent-children groups relations
            foreach (SerializableBlueprintGroup group in groups.Values)
            {
                IEnumerable<SerializableBlueprintGroup> children = s_marketGroups.Concat(
                    s_injectedMarketGroups).Where(x => x.ParentID == group.ID).Select(x => groups[x.ID]);

                group.SubGroups = children.OrderBy(x => x.Name).ToArray();
            }

            // Sort groups
            IOrderedEnumerable<SerializableBlueprintGroup> blueprintGroups = s_marketGroups.Concat(
                s_injectedMarketGroups).Where(x => x.ParentID == DBConstants.BlueprintsGroupID)
                .Select(x => groups[x.ID]).OrderBy(x => x.Name);

            s_endTime = DateTime.Now;
            Console.WriteLine(String.Format(" in {0}", s_endTime.Subtract(s_startTime)).TrimEnd('0'));

            // Serialize
            var datafile = new BlueprintsDatafile();
            datafile.Groups = blueprintGroups.ToArray();
            Util.SerializeXML(datafile, DatafileConstants.BlueprintsDatafile);
        }

        /// <summary>
        /// 
        /// </summary>
        private static void ConfigureNullMarketBlueprint()
        {
            s_injectedMarketGroups = new List<InvMarketGroup>();

            // Create custom market groups that don't exist in EVE
            s_injectedMarketGroups.Add(new InvMarketGroup
                                           {
                                               Name = "Various Non-Market",
                                               Description = "Various blueprints not in EVE market",
                                               ID = DBConstants.BlueprintRootNonMarketGroupID,
                                               ParentID = DBConstants.BlueprintsGroupID,
                                               IconID = 2703
                                           });

            s_injectedMarketGroups.Add(new InvMarketGroup
                                           {
                                               Name = "Tech I",
                                               Description = "Tech I blueprints not in EVE market",
                                               ID = DBConstants.BlueprintNonMarketTechIGroupID,
                                               ParentID = DBConstants.BlueprintRootNonMarketGroupID,
                                               IconID = 2703
                                           });

            s_injectedMarketGroups.Add(new InvMarketGroup
                                           {
                                               Name = "Tech II",
                                               Description = "Tech II blueprints not in EVE market",
                                               ID = DBConstants.BlueprintNonMarketTechIIGroupID,
                                               ParentID = DBConstants.BlueprintRootNonMarketGroupID,
                                               IconID = 2703
                                           });

            s_injectedMarketGroups.Add(new InvMarketGroup
                                           {
                                               Name = "Storyline",
                                               Description = "Storyline blueprints not in EVE market",
                                               ID = DBConstants.BlueprintNonMarketStorylineGroupID,
                                               ParentID = DBConstants.BlueprintRootNonMarketGroupID,
                                               IconID = 2703
                                           });

            s_injectedMarketGroups.Add(new InvMarketGroup
                                           {
                                               Name = "Faction",
                                               Description = "Faction blueprints not in EVE market",
                                               ID = DBConstants.BlueprintNonMarketFactionGroupID,
                                               ParentID = DBConstants.BlueprintRootNonMarketGroupID,
                                               IconID = 2703
                                           });

            s_injectedMarketGroups.Add(new InvMarketGroup
                                           {
                                               Name = "Officer",
                                               Description = "Officer blueprints not in EVE market",
                                               ID = DBConstants.BlueprintNonMarketOfficerGroupID,
                                               ParentID = DBConstants.BlueprintRootNonMarketGroupID,
                                               IconID = 2703
                                           });

            s_injectedMarketGroups.Add(new InvMarketGroup
                                           {
                                               Name = "Tech III",
                                               Description = "Tech III blueprints not in EVE market",
                                               ID = DBConstants.BlueprintNonMarketTechIIIGroupID,
                                               ParentID = DBConstants.BlueprintRootNonMarketGroupID,
                                               IconID = 2703
                                           });

            // Set some blueprints to market groups manually
            foreach (InvType item in s_types.Where(x => x.MarketGroupID == null))
            {
                switch (item.ID)
                {
                    case 22924: // 'Wild' Miner I Blueprint
                        item.MarketGroupID = DBConstants.BlueprintNonMarketStorylineGroupID;
                        break;
                    case 2837: // Adrestia Blueprint
                    case 3533: // Echelon Blueprint
                    case 17704: // Imperial Navy Slicer Blueprint
                    case 17620: // Caldari Navy Hookbill Blueprint
                    case 17842: // Federation Navy Comet Blueprint
                    case 17813: // Republic Fleet Firetail Blueprint
                    case 17737: // Nightmare Blueprint
                    case 17739: // Machariel Blueprint
                    case 17933: // Dramiel Blueprint
                    case 17927: // Cruor Blueprint
                    case 17925: // Succubus Blueprint
                    case 17929: // Daredevil Blueprint
                    case 17721: // Cynabal Blueprint
                    case 17923: // Ashimmu Blueprint
                    case 17719: // Phantasm Blueprint
                    case 21098: // Goru's Shuttle Blueprint
                    case 21629: // Guristas Shuttle Blueprint
                    case 21842: // Gallente Mining Laser Blueprint
                    case 30843: // Interbus Shuttle Blueprint
                    case 32208: // Freki Blueprint
                    case 32210: // Mimir Blueprint
                        item.MarketGroupID = DBConstants.BlueprintNonMarketFactionGroupID;
                        break;
                    case 29987: // Legion Blueprint
                    case 30227: // Legion Defensive - Adaptive Augmenter Blueprint
                    case 30037: // Legion Electronics - Energy Parasitic Complex Blueprint
                    case 30170: // Legion Engineering - Power Core Multiplier Blueprint
                    case 30392: // Legion Offensive - Drone Synthesis Projector Blueprint
                    case 30077: // Legion Propulsion - Chassis Optimization Blueprint
                    case 29991: // Loki Blueprint
                    case 30242: // Loki Defensive - Adaptive Shielding Blueprint
                    case 30067: // Loki Electronics - Immobility Drivers Blueprint
                    case 30160: // Loki Engineering - Power Core Multiplier Blueprint
                    case 30407: // Loki Offensive - Turret Concurrence Registry Blueprint
                    case 30107: // Loki Propulsion - Chassis Optimization Blueprint
                    case 29989: // Proteus Blueprint
                    case 30237: // Proteus Defensive - Adaptive Augmenter Blueprint
                    case 30057: // Proteus Electronics - Friction Extension Processor Blueprint
                    case 30150: // Proteus Engineering - Power Core Multiplier Blueprint
                    case 30402: // Proteus Offensive - Dissonic Encoding Platform Blueprint
                    case 30097: // Proteus Propulsion - Wake Limiter Blueprint
                    case 29985: // Tengu Blueprint
                    case 30232: // Tengu Defensive - Adaptive Shielding Blueprint
                    case 30047: // Tengu Electronics - Obfuscation Manifold Blueprint
                    case 30140: // Tengu Engineering - Power Core Multiplier Blueprint
                    case 30397: // Tengu Offensive - Accelerated Ejection Bay Blueprint
                    case 30087: // Tengu Propulsion - Intercalated Nanofibers Blueprint
                        item.MarketGroupID = DBConstants.BlueprintNonMarketTechIIIGroupID;
                        break;
                    case 32078: // Small EW Drone Range Augmentor II Blueprint
                        item.MarketGroupID = DBConstants.BlueprintNonMarketTechIIGroupID;
                        break;
                }
            }

            // Set the market group of the blueprints with NULL MarketGroupID to custom market groups
            foreach (InvType item in s_types.Where(x => x.MarketGroupID == null && !x.Name.Contains("TEST")
                                                        && s_blueprintTypes.Any(y => y.ID == x.ID) &&
                                                        s_types.Any(z => z.ID == s_blueprintTypes[x.ID].ProductTypeID)
                                                        && s_types[s_blueprintTypes[x.ID].ProductTypeID].Published))
            {
                UpdatePercentDone(s_blueprintGenTotal);

                foreach (InvMetaType relation in s_metaTypes
                    .Where(x => x.ItemID == s_blueprintTypes[item.ID].ProductTypeID))
                {
                    switch (relation.MetaGroupID)
                    {
                        case 2:
                            item.MarketGroupID = DBConstants.BlueprintNonMarketTechIIGroupID;
                            break;
                        case 3:
                            item.MarketGroupID = DBConstants.BlueprintNonMarketStorylineGroupID;
                            break;
                        case 4:
                            item.MarketGroupID = DBConstants.BlueprintNonMarketFactionGroupID;
                            break;
                        case 5:
                            item.MarketGroupID = DBConstants.BlueprintNonMarketOfficerGroupID;
                            break;
                        case 14:
                            item.MarketGroupID = DBConstants.BlueprintNonMarketTechIIIGroupID;
                            break;
                    }
                }

                if (item.MarketGroupID == null)
                    item.MarketGroupID = DBConstants.BlueprintNonMarketTechIGroupID;
            }
        }

        /// <summary>
        /// Add properties to a blueprint.
        /// </summary>
        /// <param name="srcBlueprint"></param>
        /// <param name="blueprintsGroup"></param>
        /// <returns></returns>
        private static void CreateBlueprint(InvType srcBlueprint, List<SerializableBlueprint> blueprintsGroup)
        {
            UpdatePercentDone(s_blueprintGenTotal);

            srcBlueprint.Generated = true;

            // Creates the blueprint with base informations
            var blueprint = new SerializableBlueprint
                           {
                               ID = srcBlueprint.ID,
                               Name = srcBlueprint.Name,
                           };

            // Icon
            blueprint.Icon = (srcBlueprint.IconID.HasValue ? s_icons[srcBlueprint.IconID.Value].Icon : String.Empty);

            // Export attributes
            foreach (InvBlueprintTypes attribute in s_blueprintTypes.Where(x => x.ID == srcBlueprint.ID))
            {
                blueprint.ProduceItemID = attribute.ProductTypeID;
                blueprint.ProductionTime = attribute.ProductionTime;
                blueprint.TechLevel = attribute.TechLevel;
                blueprint.ResearchProductivityTime = attribute.ResearchProductivityTime;
                blueprint.ResearchMaterialTime = attribute.ResearchMaterialTime;
                blueprint.ResearchCopyTime = attribute.ResearchCopyTime;
                blueprint.ResearchTechTime = attribute.ResearchTechTime;
                blueprint.ProductivityModifier = attribute.ProductivityModifier;
                blueprint.WasteFactor = attribute.WasteFactor;
                blueprint.MaxProductionLimit = attribute.MaxProductionLimit;
            }

            // Metagroup
            foreach (InvMetaType relation in s_metaTypes
                .Where(x => x.ItemID == s_blueprintTypes[srcBlueprint.ID].ProductTypeID))
            {
                switch (relation.MetaGroupID)
                {
                    default:
                    case 1:
                        blueprint.MetaGroup = ItemMetaGroup.T1;
                        break;
                    case 2:
                        blueprint.MetaGroup = ItemMetaGroup.T2;
                        break;
                    case 3:
                        blueprint.MetaGroup = ItemMetaGroup.Storyline;
                        break;
                    case 4:
                        blueprint.MetaGroup = ItemMetaGroup.Faction;
                        break;
                    case 5:
                        blueprint.MetaGroup = ItemMetaGroup.Officer;
                        break;
                    case 6:
                        blueprint.MetaGroup = ItemMetaGroup.Deadspace;
                        break;
                    case 14:
                        blueprint.MetaGroup = ItemMetaGroup.T3;
                        break;
                }
            }

            // Metagroup for the custom market groups
            switch (srcBlueprint.MarketGroupID)
            {
                case DBConstants.BlueprintNonMarketStorylineGroupID:
                    blueprint.MetaGroup = ItemMetaGroup.Storyline;
                    break;
                case DBConstants.BlueprintNonMarketFactionGroupID:
                    blueprint.MetaGroup = ItemMetaGroup.Faction;
                    break;
                case DBConstants.BlueprintNonMarketTechIIIGroupID:
                    blueprint.MetaGroup = ItemMetaGroup.T3;
                    break;
                case DBConstants.BlueprintNonMarketTechIIGroupID:
                    blueprint.MetaGroup = ItemMetaGroup.T2;
                    break;
            }

            if (blueprint.MetaGroup == ItemMetaGroup.Empty)
                blueprint.MetaGroup = ItemMetaGroup.T1;

            // Export item requirements
            ExportRequirements(srcBlueprint, blueprint);

            // Look for the tech 2 variations that this blueprint invents
            var inventionBlueprint = new List<int>();
            foreach (int relationItemID in s_metaTypes
                .Where(x => x.ParentItemID == blueprint.ProduceItemID && x.MetaGroupID == 2)
                .Select(x => x.ItemID))
            {
                // Look for a blueprint that produces the related item
                foreach (int variationItemID in s_blueprintTypes
                    .Where(x => x.ProductTypeID == relationItemID).Select(x => x.ID))
                {
                    // Add the variation blueprint
                    inventionBlueprint.Add(s_types[variationItemID].ID);
                }
            }

            // Add invention blueprints to item
            blueprint.InventionTypeID = inventionBlueprint.ToArray();

            // Add this item
            blueprintsGroup.Add(blueprint);
        }

        /// <summary>
        /// Export item requirements. 
        /// </summary>
        /// <param name="srcBlueprint"></param>
        /// <param name="blueprint"></param>
        private static void ExportRequirements(InvType srcBlueprint, SerializableBlueprint blueprint)
        {
            var prerequisiteSkills = new List<SerializablePrereqSkill>();
            var requiredMaterials = new List<SerializableRequiredMaterial>();

            // Add the required raw materials
            AddRequiredRawMaterials(blueprint.ProduceItemID, requiredMaterials);

            // Add the required extra materials
            AddRequiredExtraMaterials(srcBlueprint.ID, prerequisiteSkills, requiredMaterials);

            // Add prerequisite skills to item
            blueprint.PrereqSkill = prerequisiteSkills.OrderBy(x => x.Activity).ToArray();

            // Add required materials to item
            blueprint.ReqMaterial = requiredMaterials.OrderBy(x => x.Activity).ToArray();
        }

        /// <summary>
        /// Adds the raw materials needed to produce an item.
        /// </summary>
        /// <param name="blueprint"></param>
        /// <param name="requiredMaterials"></param>
        private static void AddRequiredRawMaterials(int produceItemID,
                                                    List<SerializableRequiredMaterial> requiredMaterials)
        {
            // Find the raw materials needed for the produced item and add them to the list
            foreach (InvTypeMaterials reprocItem in s_typeMaterials.Where(x => x.TypeID == produceItemID))
            {
                requiredMaterials.Add(new SerializableRequiredMaterial
                                          {
                                              ID = reprocItem.MaterialTypeID,
                                              Quantity = reprocItem.Quantity,
                                              DamagePerJob = 1,
                                              Activity = (int) BlueprintActivity.Manufacturing,
                                              WasteAffected = 1
                                          });
            }
        }

        /// <summary>
        /// Adds the extra materials needed to produce an item.
        /// </summary>
        /// <param name="srcBlueprint"></param>
        /// <param name="prerequisiteSkills"></param>
        /// <param name="requiredMaterials"></param>
        private static void AddRequiredExtraMaterials(int blueprintID,
                                                      List<SerializablePrereqSkill> prerequisiteSkills,
                                                      List<SerializableRequiredMaterial> requiredMaterials)
        {
            // Find the additional extra materials and add them to the list
            foreach (RamTypeRequirements requirement in s_typeRequirements.Where(x => x.TypeID == blueprintID))
            {
                // Is it a skill ? Add it to the prerequisities skills list
                if (s_types.Any(x => x.ID == requirement.RequiredTypeID
                                    && s_groups.Any(y => y.ID == x.GroupID
                                    && y.CategoryID == DBConstants.SkillCategoryID)))
                {
                    prerequisiteSkills.Add(new SerializablePrereqSkill
                                               {
                                                   ID = requirement.RequiredTypeID,
                                                   Level = requirement.Quantity,
                                                   Activity = requirement.ActivityID
                                               });
                }
                else // It is an item (extra material)
                {
                    requiredMaterials.Add(new SerializableRequiredMaterial
                                              {
                                                  ID = requirement.RequiredTypeID,
                                                  Quantity = requirement.Quantity,
                                                  DamagePerJob = requirement.DamagePerJob,
                                                  Activity = requirement.ActivityID,
                                                  WasteAffected = 0
                                              });

                    // If the item is recyclable, we need to find the materials produced by reprocessing it
                    // and substracted them from the related materials of the requiredMaterials list
                    if (requirement.Recyclable)
                    {
                        foreach (InvTypeMaterials reprocItem in s_typeMaterials
                                .Where(x => x.TypeID == requirement.RequiredTypeID))
                        {
                            if (requiredMaterials.Any(x => x.ID == reprocItem.MaterialTypeID))
                            {
                                SerializableRequiredMaterial material = requiredMaterials
                                    .First(x => x.ID == reprocItem.MaterialTypeID);

                                material.Quantity -= requirement.Quantity*reprocItem.Quantity;

                                if (material.Quantity < 1)
                                    requiredMaterials.Remove(material);
                            }
                        }
                    }

                    // If activity is invention, add the prerequisite skill
                    // of the required material as it's not included in this table
                    if (requirement.ActivityID == (int)BlueprintActivity.Invention)
                    {
                        // Add the prerequisite skills for a material used in invention activity.
                        MaterialPrereqSkill(requirement, prerequisiteSkills);
                    }
                }
            }
        }

        /// <summary>
        /// Add the prerequisite skills for a material used in invention activity.
        /// </summary>
        private static void MaterialPrereqSkill(RamTypeRequirements requirement,
                                                List<SerializablePrereqSkill> prerequisiteSkills)
        {
            var prereqSkills = new int[DBConstants.RequiredSkillPropertyIDs.Length];
            var prereqLevels = new int[DBConstants.RequiredSkillPropertyIDs.Length];

            foreach (DgmTypeAttribute attribute in s_typeAttributes
                    .Where(x => x.ItemID == requirement.RequiredTypeID))
            {
                int attributeIntValue = (attribute.ValueInt.HasValue
                                                ? attribute.ValueInt.Value
                                                : (int)attribute.ValueFloat.Value);

                // Is it a prereq skill ?
                int prereqIndex = Array.IndexOf(DBConstants.RequiredSkillPropertyIDs, attribute.AttributeID);
                if (prereqIndex >= 0)
                {
                    prereqSkills[prereqIndex] = attributeIntValue;
                    continue;
                }

                // Is it a prereq level ?
                prereqIndex = Array.IndexOf(DBConstants.RequiredSkillLevelPropertyIDs, attribute.AttributeID);
                if (prereqIndex >= 0)
                {
                    prereqLevels[prereqIndex] = attributeIntValue;
                    continue;
                }
            }

            // Add the prerequisite skills
            for (int i = 0; i < prereqSkills.Length; i++)
            {
                if (prereqSkills[i] != 0)
                    prerequisiteSkills.Add(new SerializablePrereqSkill
                                               {
                                                   ID = prereqSkills[i],
                                                   Level = prereqLevels[i],
                                                   Activity = requirement.ActivityID
                                               });
            }
        }

        #endregion

        #region Geography Datafile

        /// <summary>
        /// Generates the geo datafile.
        /// </summary>
        private static void GenerateGeography()
        {
            Console.WriteLine();
            Console.Write(@"Generated geography datafile... ");

            s_counter = 0;
            s_percentOld = 0;
            s_text = String.Empty;
            s_startTime = DateTime.Now;

            var allSystems = new List<SerializableSolarSystem>();
            var regions = new List<SerializableRegion>();

            // Regions
            foreach (MapRegion srcRegion in s_regions)
            {
                var region = new SerializableRegion
                                 {
                                     ID = srcRegion.ID,
                                     Name = srcRegion.Name
                                 };
                regions.Add(region);

                // Constellations
                var constellations = new List<SerializableConstellation>();
                foreach (MapConstellation srcConstellation in s_constellations.Where(x => x.RegionID == srcRegion.ID))
                {
                    var constellation = new SerializableConstellation
                                            {
                                                ID = srcConstellation.ID,
                                                Name = srcConstellation.Name
                                            };
                    constellations.Add(constellation);

                    // Systems
                    const double baseDistance = 1.0E14;
                    var systems = new List<SerializableSolarSystem>();
                    foreach (
                        MapSolarSystem srcSystem in s_solarSystems.Where(x => x.ConstellationID == srcConstellation.ID))
                    {
                        var system = new SerializableSolarSystem
                                         {
                                             ID = srcSystem.ID,
                                             Name = srcSystem.Name,
                                             X = (int) (srcSystem.X/baseDistance),
                                             Y = (int) (srcSystem.Y/baseDistance),
                                             Z = (int) (srcSystem.Z/baseDistance),
                                             SecurityLevel = srcSystem.SecurityLevel
                                         };
                        systems.Add(system);

                        // Stations
                        var stations = new List<SerializableStation>();
                        foreach (StaStation srcStation in s_stations.Where(x => x.SolarSystemID == srcSystem.ID))
                        {
                            UpdatePercentDone(s_geoGen);

                            // Agents
                            var stationAgents = new List<SerializableAgent>();
                            foreach (AgtAgents srcAgent in s_agents.Where(x => x.LocationID == srcStation.ID))
                            {
                                var agent = new SerializableAgent
                                                {
                                                    ID = srcAgent.ID,
                                                    Level = srcAgent.Level,
                                                    Quality = srcAgent.Quality,
                                                    Name = s_names.FirstOrDefault(x => x.ID == srcAgent.ID).Name
                                                };
                                stationAgents.Add(agent);
                            }

                            var station = new SerializableStation
                                              {
                                                  ID = srcStation.ID,
                                                  Name = srcStation.Name,
                                                  CorporationID = srcStation.CorporationID,
                                                  CorporationName = s_names.FirstOrDefault(x => x.ID == srcStation.CorporationID).Name,
                                                  ReprocessingEfficiency = srcStation.ReprocessingEfficiency,
                                                  ReprocessingStationsTake = srcStation.ReprocessingStationsTake,
                                                  Agents = stationAgents.ToArray()
                                              };
                            stations.Add(station);
                        }
                        system.Stations = stations.OrderBy(x => x.Name).ToArray();
                    }
                    constellation.Systems = systems.OrderBy(x => x.Name).ToArray();
                }
                region.Constellations = constellations.OrderBy(x => x.Name).ToArray();
            }

            // Jumps
            var jumps = new List<SerializableJump>();
            foreach (MapSolarSystemJump srcJump in s_jumps)
            {
                UpdatePercentDone(s_geoGenTotal);

                // In CCP tables, every jump is included twice, we only need one.
                if (srcJump.A < srcJump.B)
                    jumps.Add(new SerializableJump {FirstSystemID = srcJump.A, SecondSystemID = srcJump.B});
            }

            s_endTime = DateTime.Now;
            Console.WriteLine(String.Format(" in {0}", s_endTime.Subtract(s_startTime)).TrimEnd('0'));

            // Serialize
            var datafile = new GeoDatafile();
            datafile.Regions = regions.OrderBy(x => x.Name).ToArray();
            datafile.Jumps = jumps.ToArray();
            Util.SerializeXML(datafile, DatafileConstants.GeographyDatafile);
        }

        #endregion

        #region Reprocessing Datafile

        /// <summary>
        /// Generates the reprocessing datafile.
        /// </summary>
        private static void GenerateReprocessing()
        {
            Console.WriteLine();
            Console.Write(@"Generated reprocessing datafile... ");

            s_counter = 0;
            s_percentOld = 0;
            s_text = String.Empty;
            s_startTime = DateTime.Now;

            var types = new List<SerializableItemMaterials>();

            foreach (int typeID in s_types.Where(x => x.Generated).Select(x => x.ID))
            {
                UpdatePercentDone(s_reprocessGenTotal);

                var materials = new List<SerializableMaterialQuantity>();
                foreach (InvTypeMaterials srcMaterial in s_typeMaterials.Where(x => x.TypeID == typeID))
                {
                    materials.Add(new SerializableMaterialQuantity
                                      {ID = srcMaterial.MaterialTypeID, Quantity = srcMaterial.Quantity});
                }

                if (materials.Count != 0)
                {
                    types.Add(new SerializableItemMaterials
                                  {
                                      ID = typeID,
                                      Materials = materials.OrderBy(x => x.ID).ToArray()
                                  });
                }
            }

            s_endTime = DateTime.Now;
            Console.WriteLine(String.Format(" in {0}", s_endTime.Subtract(s_startTime)).TrimEnd('0'));

            // Serialize
            var datafile = new ReprocessingDatafile();
            datafile.Items = types.ToArray();
            Util.SerializeXML(datafile, DatafileConstants.ReprocessingDatafile);
        }

        #endregion

        #region MD5Sums

        /// <summary>
        /// Generates the MD5Sums file.
        /// </summary>
        private static void GenerateMD5Sums()
        {
            Util.CreateMD5SumsFile("MD5Sums.txt");
        }

        #endregion

        #region Helper Methods

        /// <summary>
        /// Updates the percantage done of the datafile generating procedure.
        /// </summary>
        /// <param name="total"></param>
        private static void UpdatePercentDone(double total)
        {
            s_counter++;
            var percent = (int) ((s_counter/total)*100);

            if (s_counter == 1 || s_percentOld < percent)
            {
                Console.SetCursorPosition(Console.CursorLeft - s_text.Length, Console.CursorTop);
                s_text = String.Format("{0}%", percent);
                Console.Write(s_text);
                s_percentOld = percent;
            }
        }

        /// <summary>
        /// Updates the progress of data loaded from SQL server.
        /// </summary>
        private static void UpdateProgress()
        {
            Console.SetCursorPosition(Console.CursorLeft - s_text.Length, Console.CursorTop);
            s_tablesCount++;
            s_text = String.Format("{0}%", (int) ((s_tablesCount/s_totalTablesCount)*100));
            Console.Write(s_text);
        }

        #endregion
    }
}