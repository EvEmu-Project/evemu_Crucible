using System;
using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a character sheet. Used for settings.xml serialization and CCP querying
    /// </summary>
    public class SerializableCharacterSheetBase : ISerializableCharacterIdentity
    {
        public SerializableCharacterSheetBase()
        {
            Attributes = new SerializableCharacterAttributes();
            Skills = new List<SerializableCharacterSkill>();
            Certificates = new List<SerializableCharacterCertificate>();
        }

        [XmlElement("characterID")]
        public long ID
        {
            get;
            set;
        }

        [XmlElement("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlElement("DoB")]
        public string BirthdayXml
        {
            get { return Birthday.DateTimeToTimeString(); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    Birthday = value.TimeStringToDateTime();
            }
        }

        [XmlElement("race")]
        public string Race
        {
            get;
            set;
        }

        [XmlElement("bloodLine")]
        public string BloodLine
        {
            get;
            set;
        }

        [XmlElement("ancestry")]
        public string Ancestry
        {
            get;
            set;
        }

        [XmlElement("gender")]
        public string Gender
        {
            get;
            set;
        }

        [XmlElement("corporationName")]
        public string CorporationName
        {
            get;
            set;
        }

        [XmlElement("corporationID")]
        public int CorporationID
        {
            get;
            set;
        }

        [XmlElement("allianceName")]
        public string AllianceName
        {
            get;
            set;
        }

        [XmlElement("allianceID")]
        public int AllianceID
        {
            get;
            set;
        }

        [XmlElement("cloneName")]
        public string CloneName
        {
            get;
            set;
        }

        [XmlElement("cloneSkillPoints")]
        public int CloneSkillPoints
        {
            get;
            set;
        }

        [XmlElement("balance")]
        public Decimal Balance
        {
            get;
            set;
        }

        [XmlElement("attributes")]
        public SerializableCharacterAttributes Attributes
        {
            get;
            set;
        }

        [XmlArray("skills")]
        [XmlArrayItem("skill")]
        public List<SerializableCharacterSkill> Skills
        {
            get;
            set;
        }

        [XmlArray("certificates")]
        [XmlArrayItem("certificate")]
        public List<SerializableCharacterCertificate> Certificates
        {
            get;
            set;
        }

        /// <summary>
        /// The date and time the character was created.
        /// </summary>
        [XmlIgnore]
        public DateTime Birthday
        {
            get;
            set;
        }
    }
}
