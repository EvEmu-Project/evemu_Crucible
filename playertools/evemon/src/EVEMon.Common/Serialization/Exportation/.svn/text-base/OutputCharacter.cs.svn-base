using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Exportation
{
    /// <summary>
    /// A serialization class designed for HTML exportation.
    /// </summary>
    [XmlRoot("outputCharacter")]
    public sealed class OutputCharacter
    {
        public OutputCharacter()
        {
            SkillGroups = new List<OutputSkillGroup>();
            Certificates = new List<OutputCertificate>();
            AttributeEnhancers = new List<OutputAttributeEnhancer>();
        }

        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("characterID")]
        public long CharacterID
        {
            get;
            set;
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

        [XmlElement("balance")]
        public Decimal Balance
        {
            get;
            set;
        }

        [XmlElement("intelligence")]
        public float Intelligence
        {
            get;
            set;
        }

        [XmlElement("charisma")]
        public float Charisma
        {
            get;
            set;
        }

        [XmlElement("perception")]
        public float Perception
        {
            get;
            set;
        }

        [XmlElement("memory")]
        public float Memory
        {
            get;
            set;
        }

        [XmlElement("willpower")]
        public float Willpower
        {
            get;
            set;
        }

        [XmlArray("attributeEnhancers")]
        [XmlArrayItem("implant")]
        public List<OutputAttributeEnhancer> AttributeEnhancers
        {
            get;
            set;
        }

        [XmlArray("skills")]
        [XmlArrayItem("skillGroup")]
        public List<OutputSkillGroup> SkillGroups
        {
            get;
            set;
        }

        [XmlArray("certificates")]
        [XmlArrayItem("certificate")]
        public List<OutputCertificate> Certificates
        {
            get;
            set;
        }

    }


    /// <summary>
    /// A serialization class designed for HTML exportation.
    /// </summary>
    public sealed class OutputAttributeEnhancer
    {
        [XmlAttribute("attribute")]
        public ImplantSlots Attribute
        {
            get;
            set;
        }

        [XmlAttribute("bonus")]
        public int Bonus
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
    }

    /// <summary>
    /// A serialization class designed for HTML exportation.
    /// </summary>
    public sealed class OutputCertificate
    {
        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("grade")]
        public string Grade
        {
            get;
            set;
        }
    }

}
