using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Scheduling;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    public sealed class SerializableScheduler
    {
        public SerializableScheduler()
        {
            Entries = new List<SerializableScheduleEntry>();
        }

        [XmlArray("entries")]
        [XmlArrayItem("simple", typeof(SerializableSimpleScheduleEntry))]
        [XmlArrayItem("recurring", typeof(SerializableRecurringScheduleEntry))]
        public List<SerializableScheduleEntry> Entries
        {
            get;
            set;
        }
    }

    public class SerializableScheduleEntry
    {
    }
}
