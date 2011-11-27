using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization;
using System.Xml.Serialization;
using System.Drawing;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class SchedulerUISettings
    {
        public SchedulerUISettings()
        {
            TextColor = (SerializableColor)Color.White;
            BlockingColor = (SerializableColor)Color.Red;
            SimpleEventGradientStart = (SerializableColor)Color.Blue;
            SimpleEventGradientEnd = (SerializableColor)Color.LightBlue;
            RecurringEventGradientStart = (SerializableColor)Color.Green;
            RecurringEventGradientEnd = (SerializableColor)Color.LightGreen;
        }

        [XmlElement("textColor")]
        public SerializableColor TextColor
        {
            get;
            set;
        }

        [XmlElement("blockColor")]
        public SerializableColor BlockingColor
        {
            get;
            set;
        }

        [XmlElement("recurringEventGradientStart")]
        public SerializableColor RecurringEventGradientStart
        {
            get;
            set;
        }

        [XmlElement("recurringEventGradientEnd")]
        public SerializableColor RecurringEventGradientEnd
        {
            get;
            set;
        }

        [XmlElement("simpleEventGradientStart")]
        public SerializableColor SimpleEventGradientStart
        {
            get;
            set;
        }

        [XmlElement("simpleEventGradientEnd")]
        public SerializableColor SimpleEventGradientEnd
        {
            get;
            set;
        }

        internal SchedulerUISettings Clone()
        {
            var clone = new SchedulerUISettings();
            clone.TextColor = this.TextColor.Clone();
            clone.BlockingColor = this.BlockingColor.Clone();
            clone.SimpleEventGradientStart = this.SimpleEventGradientStart.Clone();
            clone.SimpleEventGradientEnd = this.SimpleEventGradientEnd.Clone();
            clone.RecurringEventGradientStart = this.RecurringEventGradientStart.Clone();
            clone.RecurringEventGradientEnd = this.RecurringEventGradientEnd.Clone();
            return clone;
        }
    }
}
