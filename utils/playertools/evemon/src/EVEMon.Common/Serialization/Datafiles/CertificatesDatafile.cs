using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Datafiles
{
    /// <summary>
    /// Represents out certificates datafile
    /// </summary>
    [XmlRoot("certificates")]
    public sealed class CertificatesDatafile
    {
        [XmlElement("certificateCategory")]
        public SerializableCertificateCategory[] Categories
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a certificate category from our datafiles
    /// </summary>
    public sealed class SerializableCertificateCategory
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

        [XmlAttribute("descr")]
        public string Description
        {
            get;
            set;
        }

        [XmlElement("certificateClass")]
        public SerializableCertificateClass[] Classes
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a certificate class from our datafiles
    /// </summary>
    public sealed class SerializableCertificateClass
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

        [XmlAttribute("descr")]
        public string Description
        {
            get;
            set;
        }

        [XmlElement("certificate")]
        public SerializableCertificate[] Certificates
        {
            get;
            set;
        }
    }


    /// <summary>
    /// Represents a certificate from our datafiles
    /// </summary>
    public sealed class SerializableCertificate
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }
        [XmlAttribute("grade")]
        public CertificateGrade Grade
        {
            get;
            set;
        }

        [XmlAttribute("descr")]
        public string Description
        {
            get;
            set;
        }

        [XmlElement("requires")]
        public SerializableCertificatePrerequisite[] Prerequisites
        {
            get;
            set;
        }

        [XmlElement("recommendation")]
        public SerializableCertificateRecommendation[] Recommendations
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a skill prerequisite for a certificate
    /// </summary>
    public sealed class SerializableCertificatePrerequisite
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }
        
        [XmlAttribute("type")]
        public SerializableCertificatePrerequisiteKind Kind
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
        public string Level
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a recommendation for a ship
    /// </summary>
    public sealed class SerializableCertificateRecommendation
    {
        [XmlAttribute("id")]
        public int ID
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

        [XmlAttribute("ship")]
        public string Ship
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Describes a type of prerequisite certificate. May be a certificate or a skill.
    /// </summary>
    public enum SerializableCertificatePrerequisiteKind
    {
        [XmlEnum("skill")]
        Skill,
        [XmlEnum("cert")]
        Certificate
    }
}
