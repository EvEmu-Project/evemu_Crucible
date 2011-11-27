using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;
using EVEMon.Common.Scheduling;

namespace EVEMon.Common.Serialization.Settings
{
    public sealed class SerializableSimpleScheduleEntry : SerializableScheduleEntry
    {
        [XmlAttribute("startDateTime")]
        public DateTime StartDateTime
        {
            get;
            set;
        }

        [XmlAttribute("endDateTime")]
        public DateTime EndDateTime
        {
            get;
            set;
        }

        [XmlElement("title")]
        public string Title
        {
            get;
            set;
        }

        [XmlElement("options")]
        public ScheduleEntryOptions Options
        {
            get;
            set;
        }
    }
}
