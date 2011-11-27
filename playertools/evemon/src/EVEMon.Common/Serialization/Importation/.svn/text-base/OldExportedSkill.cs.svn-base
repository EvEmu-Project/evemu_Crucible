using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Importation
{
    /// <summary>
    /// Facilitates importation of file characters from versions of
    /// EVEMon prior to 1.3.0.
    /// </summary>
    /// <remarks>
    /// These changes were released early 2010, it is safe to assume
    /// that they can be removed from the project early 2012.
    /// </remarks>
    [XmlRoot("skill")]
    public sealed class OldExportedSkill
    {
        [XmlAttribute("typeName")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("typeID")]
        public int Id
        {
            get;
            set;
        }

        [XmlElement("skillpoints")]
        public int SkillPoints
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

        [XmlElement("lastconfirmedlevel")]
        public int LastConfirmedLevel
        {
            get;
            set;
        }
    }
}
