using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents a remapping point
    /// </summary>
    public sealed class SerializableRemappingPoint
    {
        [XmlAttribute("status")]
        public RemappingPoint.PointStatus Status
        {
            get;
            set;
        }

        [XmlAttribute("per")]
        public int Perception
        {
            get;
            set;
        }

        [XmlAttribute("int")]
        public int Intelligence
        {
            get;
            set;
        }

        [XmlAttribute("mem")]
        public int Memory
        {
            get;
            set;
        }

        [XmlAttribute("wil")]
        public int Willpower
        {
            get;
            set;
        }

        [XmlAttribute("cha")]
        public int Charisma
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

        public SerializableRemappingPoint Clone()
        {
            return (SerializableRemappingPoint)this.MemberwiseClone();
        }
    }
}
