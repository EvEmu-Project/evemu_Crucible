using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Exportation
{
    /// <summary>
    /// A serialization class designed for HTML exportation.
    /// </summary>
    public sealed class OutputSkillGroup
    {
        public OutputSkillGroup()
        {
            Skills = new List<OutputSkill>();
        }

        [XmlAttribute("groupName")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("skillsCount")]
        public int SkillsCount
        {
            get;
            set;
        }

        [XmlAttribute("totalSP")]
        public int TotalSP
        {
            get;
            set;
        }

        [XmlElement("skill")]
        public List<OutputSkill> Skills
        {
            get;
            set;
        }
    }


    /// <summary>
    /// A serialization class designed for HTML exportation.
    /// </summary>
    public sealed class OutputSkill
    {
        [XmlAttribute("typeName")]
        public string Name
        {
            get;
            set;
        }

        [XmlElement("rank")]
        public int Rank
        {
            get;
            set;
        }

        [XmlElement("level")]
        public int Level
        {
            get;
            set;
        }

        [XmlElement("romanLevel")]
        public string RomanLevel
        {
            get;
            set;
        }

        [XmlElement("SP")]
        public int SkillPoints
        {
            get;
            set;
        }

        [XmlElement("maxSP")]
        public int MaxSkillPoints
        {
            get;
            set;
        }
    }
}
