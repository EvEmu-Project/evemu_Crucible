using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml.Serialization;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents a plan
    /// </summary>
    public class SerializablePlan
    {
        public SerializablePlan()
        {
            Entries = new List<SerializablePlanEntry>();
            InvalidEntries = new List<SerializableInvalidPlanEntry>();
            SortingPreferences = new PlanSorting();
        }

        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("owner")]
        public Guid Owner
        {
            get;
            set;
        }

        [XmlElement("sorting")]
        public PlanSorting SortingPreferences
        {
            get;
            set;
        }

        [XmlElement("entry")]
        public List<SerializablePlanEntry> Entries
        {
            get;
            set;
        }

        [XmlElement("invalidEntry")]
        public List<SerializableInvalidPlanEntry> InvalidEntries
        {
            get;
            set;
        }
  
        internal SerializablePlan Clone()
        {
            var clone = new SerializablePlan();
            clone.Name = Name;
            clone.Owner = Owner;
            clone.SortingPreferences = SortingPreferences.Clone();
            clone.Entries.AddRange(Entries.Select(x => x.Clone()));
            clone.InvalidEntries.AddRange(InvalidEntries.Select(x => x.Clone()));
            return clone;
        }
    }
}
