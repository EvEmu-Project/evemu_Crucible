using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class CalendarSettings
    {
        public CalendarSettings()
        {
            GoogleURL = NetworkConstants.GoogleCalendarURL;
            GoogleReminder = GoogleCalendarReminder.Email;
            RemindingInterval = 10;

            var today = DateTime.Now.Date;
            EarlyReminding = today.AddHours(8);
            LateReminding = today.AddHours(20);
        }

        [XmlAttribute("enabled")]
        public bool Enabled
        {
            get;
            set;
        }

        [XmlElement("provider")]
        public CalendarProvider Provider
        {
            get;
            set;
        }

        [XmlElement("useReminding")]
        public bool UseReminding
        {
            get;
            set;
        }

        /// <summary>
        /// Interval between reminidings, in minutes
        /// </summary>
        [XmlElement("remindingInterval")]
        public int RemindingInterval
        {
            get;
            set;
        }

        [XmlElement("remindingRange")]
        public bool UseRemindingRange
        {
            get;
            set;
        }

        [XmlElement("earlyReminding")]
        public DateTime EarlyReminding
        {
            get;
            set;
        }

        [XmlElement("lateReminding")]
        public DateTime LateReminding
        {
            get;
            set;
        }

        [XmlElement("googleEmail")]
        public string GoogleEmail
        {
            get;
            set;
        }

        [XmlElement("googlePassword")]
        public string GooglePassword
        {
            get;
            set;
        }

        [XmlElement("googleUrl")]
        public string GoogleURL
        {
            get;
            set;
        }

        [XmlElement("googleReminder")]
        public GoogleCalendarReminder GoogleReminder
        {
            get;
            set;
        }

        internal CalendarSettings Clone()
        {
            return (CalendarSettings)this.MemberwiseClone();
        }
    }

    public enum CalendarProvider
    {
        Outlook,
        Google
    }

    public enum GoogleCalendarReminder
    {
        Alert = 0,
        All = 1,
        Email = 2,
        None = 3,
        Sms = 4,
        Unspecified = 5
    }
}
