using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    public static class StaticProperties
    {
        private static readonly Dictionary<long, EveProperty> m_propertiesByID = new Dictionary<long, EveProperty>();
        private static readonly Dictionary<string, EveProperty> m_propertiesByName = new Dictionary<string, EveProperty>();
        private static readonly Dictionary<string, EvePropertyCategory> m_categoriesByName = new Dictionary<string, EvePropertyCategory>();

        /// <summary>
        /// Gets the list of properties categories.
        /// </summary>
        public static IEnumerable<EvePropertyCategory> AllCategories
        {
            get
            {
                foreach (var category in m_categoriesByName.Values)
                {
                    yield return category;
                }
            }
        }

        /// <summary>
        /// Gets the list of properties
        /// </summary>
        public static IEnumerable<EveProperty> AllProperties
        {
            get
            {
                foreach (var category in m_categoriesByName.Values)
                {
                    foreach (var property in category)
                    {
                        yield return property;
                    }
                }
            }
        }

        /// <summary>
        /// Gets a property by its name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static EveProperty GetPropertyByName(string name)
        {
            EveProperty property = null;
            m_propertiesByName.TryGetValue(name, out property);
            return property;
        }

        /// <summary>
        /// Gets a property by its identifier
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public static EveProperty GetPropertyById(int id)
        {
            EveProperty property = null;
            m_propertiesByID.TryGetValue(id, out property);
            return property;
        }

        /// <summary>
        /// Gets a group by its name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static EvePropertyCategory GetCategoryByName(string name)
        {
            EvePropertyCategory category = null;
            m_categoriesByName.TryGetValue(name, out category);
            return category;
        }

        /// <summary>
        /// Initialize static properties
        /// </summary>
        internal static void Load()
        {
            PropertiesDatafile datafile = Util.DeserializeDatafile<PropertiesDatafile>(DatafileConstants.PropertiesDatafile);

            // Fetch deserialized data
            foreach (var srcCategory in datafile.Categories)
            {
                var category = new EvePropertyCategory(srcCategory);
                m_categoriesByName[category.Name] = category;

                // Store skills
                foreach (var property in category)
                {
                    m_propertiesByID[property.ID] = property;
                    m_propertiesByName[property.Name] = property;
                }
            }

            // Visibility in ships browser
            m_propertiesByName["Base Price"].AlwaysVisibleForShips = true;
            m_propertiesByID[48].AlwaysVisibleForShips = true; // CPU
            m_propertiesByID[11].AlwaysVisibleForShips = true; // Powergrid
            m_propertiesByID[1132].AlwaysVisibleForShips = true; // Calibration
            m_propertiesByID[14].AlwaysVisibleForShips = true; // High Slots
            m_propertiesByID[13].AlwaysVisibleForShips = true; // Med Slots
            m_propertiesByID[12].AlwaysVisibleForShips = true; // Low Slots

            m_propertiesByID[283].AlwaysVisibleForShips = true; // Drone Capacity
            m_propertiesByID[1271].AlwaysVisibleForShips = true; // Drone Bandwidth

            m_propertiesByID[38].AlwaysVisibleForShips = true; // Cargo Capacity
            m_propertiesByID[4].AlwaysVisibleForShips = true; // Mass
            m_propertiesByID[161].AlwaysVisibleForShips = true; // Volume

            m_propertiesByID[482].AlwaysVisibleForShips = true; // Capacitor Capacity
            m_propertiesByID[55].AlwaysVisibleForShips = true; // Recharge time

            m_propertiesByID[76].AlwaysVisibleForShips = true; // Maximum Targeting Range
            m_propertiesByID[564].AlwaysVisibleForShips = true; // Scan Resolution
            m_propertiesByID[552].AlwaysVisibleForShips = true; // Signature Radius

            m_propertiesByID[37].AlwaysVisibleForShips = true; // Max Velocity
            m_propertiesByID[1281].AlwaysVisibleForShips = true; // Ship Warp Speed

            m_propertiesByID[9].AlwaysVisibleForShips = true; // Structure HP
            m_propertiesByID[263].AlwaysVisibleForShips = true; // Shield HP
            m_propertiesByID[265].AlwaysVisibleForShips = true; // Armor HP
            m_propertiesByID[479].AlwaysVisibleForShips = true; // Shield recharge time

            m_propertiesByID[271].AlwaysVisibleForShips = true; // Shield EM resistances
            m_propertiesByID[272].AlwaysVisibleForShips = true; // Shield Exp resistances
            m_propertiesByID[273].AlwaysVisibleForShips = true; // Shield Kin resistances
            m_propertiesByID[274].AlwaysVisibleForShips = true; // Shield Therm resistances

            m_propertiesByID[267].AlwaysVisibleForShips = true; // Armor EM resistances
            m_propertiesByID[268].AlwaysVisibleForShips = true; // Armor Exp resistances
            m_propertiesByID[269].AlwaysVisibleForShips = true; // Armor Kin resistances
            m_propertiesByID[270].AlwaysVisibleForShips = true; // Armor Therm resistances

            // Hide if default
            m_propertiesByID[101].HideIfDefault = true; // Launcher hardpoints
            m_propertiesByID[102].HideIfDefault = true; // Turret hardpoints

            m_propertiesByID[208].HideIfDefault = true; // RADAR Sensor Strength
            m_propertiesByID[209].HideIfDefault = true; // LADAR Sensor Strength
            m_propertiesByID[210].HideIfDefault = true; // Magnetometric Sensor Strength
            m_propertiesByID[211].HideIfDefault = true; // Gravimetric Sensor Strength
            
            m_propertiesByID[974].HideIfDefault = true; // Hull EM resistances
            m_propertiesByID[975].HideIfDefault = true; // Hull Exp resistances
            m_propertiesByID[976].HideIfDefault = true; // Hull Kin resistances
            m_propertiesByID[977].HideIfDefault = true; // Hull Therm resistances

            m_propertiesByID[114].HideIfDefault = true; // EM damage
            m_propertiesByID[116].HideIfDefault = true; // Explosive damage
            m_propertiesByID[117].HideIfDefault = true; // Kinetic damage
            m_propertiesByID[118].HideIfDefault = true; // Thermal damage

            m_propertiesByID[175].HideIfDefault = true; // Charisma Modifier
            m_propertiesByID[176].HideIfDefault = true; // Intelligence Modifier
            m_propertiesByID[177].HideIfDefault = true; // Memory Modifier
            m_propertiesByID[178].HideIfDefault = true; // Perception Modifier
            m_propertiesByID[179].HideIfDefault = true; // Willpower Modifier

            m_propertiesByID[633].HideIfDefault = true; // Meta Level
        }
    }
}