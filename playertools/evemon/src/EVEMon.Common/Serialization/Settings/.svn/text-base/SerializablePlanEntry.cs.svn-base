using System;
using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents a plan entry
    /// </summary>
    public sealed class SerializablePlanEntry
    {
        public SerializablePlanEntry()
        {
            PlanGroups = new List<string>();
            Priority = 3;
        }

        [XmlAttribute("skillID")]
        public long ID
        {
            get;
            set;
        }

        [XmlAttribute("skill")]
        public string SkillName
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

        [XmlAttribute("priority")]
        public int Priority
        {
            get;
            set;
        }

        [XmlAttribute("type")]
        public PlanEntryType Type
        {
            get;
            set;
        }

        [XmlElement("notes")]
        public string Notes
        {
            get;
            set;
        }

        [XmlElement("group")]
        public List<string> PlanGroups
        {
            get;
            set;
        }

        [XmlElement("remapping")]
        public SerializableRemappingPoint Remapping
        {
            get;
            set;
        }

        internal SerializablePlanEntry Clone()
        {
            var clone = (SerializablePlanEntry)MemberwiseClone();
            clone.PlanGroups = new List<string>(PlanGroups);
            clone.Remapping = Remapping.Clone();
            return clone;
        }
    }
}
