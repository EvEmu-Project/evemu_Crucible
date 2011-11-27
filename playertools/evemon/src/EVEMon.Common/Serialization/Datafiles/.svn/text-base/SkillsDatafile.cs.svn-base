using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Datafiles
{
    /// <summary>
    /// Root SkillsDatafile Serialization Class
    /// </summary>
    [XmlRoot("skills")]
    public sealed class SkillsDatafile
    {
        [XmlElement("group")]
        public SerializableSkillGroup[] Groups
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a skill group in our datafile
    /// </summary>
    public sealed class SerializableSkillGroup
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlElement("skill")]
        public SerializableSkill[] Skills
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a skill in our datafile
    /// </summary>
    public sealed class SerializableSkill
    {
        public SerializableSkill()
        {
            Public = true;
        }

        [XmlAttribute("public")]
        public bool Public
        {
            get;
            set;
        }

        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("description")]
        public string Description
        {
            get;
            set;
        }

        [XmlAttribute("primaryAttr")]
        public EveAttribute PrimaryAttribute
        {
            get;
            set;
        }

        [XmlAttribute("secondaryAttr")]
        public EveAttribute SecondaryAttribute
        {
            get;
            set;
        }

        [XmlAttribute("rank")]
        public int Rank
        {
            get;
            set;
        }

        [XmlAttribute("cost")]
        public long Cost
        {
            get;
            set;
        }

        [XmlAttribute("canTrainOnTrial")]
        public bool CanTrainOnTrial
        {
            get;
            set;
        }


        [XmlElement("prereq")]
        public SerializableSkillPrerequisite[] Prereqs
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a skill prerequisite for a skill
    /// </summary>
    public sealed class SerializableSkillPrerequisite
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("level")]
        public int Level
        {
            get;
            set;
        }
    }
}
