using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    public class SerializableResearchPoint
    {
        [XmlAttribute("agentID")]
        public int AgentID
        {
            get;
            set;
        }

        [XmlAttribute("agentName")]
        public string AgentName
        {
            get;
            set;
        }

        [XmlAttribute("skillID")]
        public int SkillID
        {
            get;
            set;
        }

        [XmlAttribute("skillName")]
        public string SkillName
        {
            get;
            set;
        }

        [XmlAttribute("startDate")]
        public DateTime StartDate
        {
            get;
            set;
        }

        [XmlAttribute("pointsPerDay")]
        public double PointsPerDay
        {
            get;
            set;
        }

        [XmlAttribute("remainderPoints")]
        public float RemainderPoints
        {
            get;
            set;
        }
    }
}
