using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    public sealed class SerializableResearchListItem
    {
        [XmlAttribute("agentID")]
        public int AgentID
        {
            get;
            set;
        }

        [XmlAttribute("skillTypeID")]
        public int SkillID
        {
            get;
            set;
        }

        [XmlAttribute("researchStartDate")]
        public string ResearchStartDateXml
        {
            get { return ResearchStartDate.DateTimeToTimeString(); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    ResearchStartDate = value.TimeStringToDateTime();
            }
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

        /// <summary>
        /// The time this research was started.
        /// </summary>
        [XmlIgnore]
        public DateTime ResearchStartDate
        {
            get;
            set;
        }
    }
}
