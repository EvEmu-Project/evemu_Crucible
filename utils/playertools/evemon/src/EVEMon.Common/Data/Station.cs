using System;
using System.Collections.Generic;
using System.Text;

using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a station inside the EVE universe.
    /// </summary>
    public class Station : IComparable<Station>
    {
        private readonly long m_id;
        private readonly string m_name;
        private readonly int m_corporationID;
        private readonly string m_corporationName;
        private readonly SolarSystem m_owner;
        private readonly float m_reprocessingTake;
        private readonly float m_reprocessingEfficiency;
        private readonly FastList<SerializableAgent> m_agents;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="src"></param>
        public Station(SerializableOutpost src)
        {
            m_id = src.StationID;
            m_name = src.StationName;
            m_corporationID = src.CorporationID;
            m_corporationName = src.CorporationName;
            m_owner = StaticGeography.GetSystem(src.SolarSystemID); 
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="src"></param>
        public Station(SolarSystem owner, SerializableStation src)
        {
            m_id = src.ID;
            m_name = src.Name;
            m_corporationID = src.CorporationID;
            m_corporationName = src.CorporationName;
            m_owner = owner;
            m_reprocessingTake = src.ReprocessingStationsTake;
            m_reprocessingEfficiency = src.ReprocessingEfficiency;

            m_agents = new FastList<SerializableAgent>(src.Agents != null ? src.Agents.Length : 0);
            if (src.Agents == null)
                return;

            foreach (var agent in src.Agents)
            {
                m_agents.Add(agent);
            }
        }

        /// <summary>
        /// Gets this object's id.
        /// </summary>
        public long ID
        {
            get { return m_id; }
        }

        /// <summary>
        /// Gets this object's name.
        /// </summary>
        public string Name
        {
            get { return m_name; }
        }

        /// <summary>
        /// Gets this object's corporation id.
        /// </summary>
        public int CorporationID
        {
            get { return m_corporationID; }
        }

        /// <summary>
        /// Gets this object's corporation name.
        /// </summary>
        public string CorporationName
        {
            get { return m_corporationName; }
        }

        /// <summary>
        /// Gets the solar system where this station is located.
        /// </summary>
        public SolarSystem SolarSystem
        {
            get { return m_owner; }
        }

        /// <summary>
        /// Gets something like Heimatar > Constellation > Pator > Pator III - Republic Military School.
        /// </summary>
        public string FullLocation
        {
            get { return m_owner.FullLocation + " > " + m_name; }
        }

        /// <summary>
        /// Gets the base reprocessing efficiency of the station.
        /// </summary>
        public float ReprocessingEfficiency
        {
            get { return m_reprocessingEfficiency; }
        }

        /// <summary>
        /// Gets the fraction of reprocessing products taken by the station.
        /// </summary>
        public float ReprocessingStationsTake
        {
            get { return m_reprocessingTake; }
        }

        /// <summary>
        /// Gets the agents of the station.
        /// </summary>
        public IEnumerable<SerializableAgent> Agents
        {
            get { return m_agents; }
        }

        /// <summary>
        /// Compares this station with another one.
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        public int CompareTo(Station other)
        {
            if (this.SolarSystem != other.SolarSystem) return this.SolarSystem.CompareTo(other.SolarSystem);
            return m_name.CompareTo(other.m_name);
        }

        /// <summary>
        /// Gets the name of this object.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return m_name;
        }
    }
}
