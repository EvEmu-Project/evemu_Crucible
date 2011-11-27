using System;
using System.Linq;

using EVEMon.Common.Data;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    public sealed class ResearchPoint
    {
        #region Fields

        private readonly int m_agentID;
        private readonly int m_skillID;
        private readonly string m_skillName;
        private readonly DateTime m_researchStartDate;
        private readonly double m_pointsPerDay;
        private readonly float m_remainderPoints;

        private string m_agentName;
        private int m_agentLevel;
        private int m_agentQuality;
        private long m_stationID;

        #endregion


        #region Constructor

        /// <summary>
        /// Constructor from the API.
        /// </summary>
        /// <param name="src"></param>
        internal ResearchPoint(SerializableResearchListItem src)
        {
            GetAgentInfoByID(src.AgentID);

            m_agentID = src.AgentID;
            m_skillID = src.SkillID;
            m_skillName = GetSkillName(src.SkillID);
            m_researchStartDate = src.ResearchStartDate;
            m_pointsPerDay = src.PointsPerDay;
            m_remainderPoints = src.RemainderPoints;
        }

        /// <summary>
        /// Constructor from an object deserialized from the settings file.
        /// </summary>
        /// <param name="src"></param>
        internal ResearchPoint(SerializableResearchPoint src)
        {
            GetAgentInfoByID(src.AgentID);

            m_agentID = src.AgentID;
            m_skillID = src.SkillID;
            m_skillName = src.SkillName;
            m_researchStartDate = src.StartDate;
            m_pointsPerDay = src.PointsPerDay;
            m_remainderPoints = src.RemainderPoints;
        }

        #endregion


        #region Properties

        /// <summary>
        /// Gets the agents ID.
        /// </summary>
        public int AgentID
        {
            get { return m_agentID; }
        }

        /// <summary>
        /// Gets the agents name.
        /// </summary>
        public string AgentName
        {
            get { return m_agentName; }
        }

        /// <summary>
        /// Gets the agents level.
        /// </summary>
        public int AgentLevel
        {
            get { return m_agentLevel; }
        }

        /// <summary>
        /// Gets the agents quality.
        /// </summary>
        public int AgentQuality
        {
            get { return m_agentQuality; }
        }

        /// <summary>
        /// Gets the agents field of research.
        /// </summary>
        public string Field
        {
            get { return m_skillName; }
        }

        /// <summary>
        /// Gets the research points per day.
        /// </summary>
        public double PointsPerDay
        {
            get { return m_pointsPerDay; }
        }

        /// <summary>
        /// Cets the current accumulated research points.
        /// </summary>
        public double CurrentRP
        {
            get
            { 
                return (m_remainderPoints + (m_pointsPerDay * DateTime.UtcNow.Subtract(m_researchStartDate).TotalDays));
            }
        }

        /// <summary>
        /// Gets the date the research was started.
        /// </summary>
        public DateTime StartDate
        {
            get { return m_researchStartDate; }
        }

        /// <summary>
        /// Gets the station where the agent is.
        /// </summary>
        public Station Station
        {
            get { return StaticGeography.GetStation(m_stationID); }
        }

        #endregion


        #region Private Finders

        /// <summary>
        /// Assigns the supplemental info of an agent by its ID.
        /// </summary>
        /// <param name="id"></param>
        private void GetAgentInfoByID(int id)
        {
            foreach (var station in StaticGeography.AllStations)
            {
                foreach (var agent in station.Agents.Where(x => x.ID == id))
                {
                    m_agentName = agent.Name;
                    m_agentLevel = agent.Level;
                    m_agentQuality = agent.Quality;
                    m_stationID = station.ID;
                }
            }
        }

        /// <summary>
        /// Gets the skill name by its ID.
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        private static string GetSkillName(int id)
        {
            StaticSkill skill = StaticSkills.GetSkillById(id);

            return (skill != null ? skill.Name : String.Empty);
        }

        #endregion


        #region Export Method

        /// <summary>
        /// Exports the given object to a serialization object.
        /// </summary>
        internal SerializableResearchPoint Export()
        {
            var serial = new SerializableResearchPoint();

            serial.AgentID = m_agentID;
            serial.AgentName = m_agentName;
            serial.SkillID = m_skillID;
            serial.SkillName = m_skillName;
            serial.StartDate = m_researchStartDate;
            serial.PointsPerDay = m_pointsPerDay;
            serial.RemainderPoints = m_remainderPoints;

            return serial;
        }

        #endregion
    }
}
