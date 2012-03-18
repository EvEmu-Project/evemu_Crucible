using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class G15Settings
    {
        public G15Settings()
        {
            CharactersCycleInterval = 20;
            TimeFormatsCycleInterval = 10;
        }

        [XmlElement("enabled")]
        public bool Enabled
        {
            get;
            set;
        }

        [XmlElement("useCharsCycle")]
        public bool UseCharactersCycle
        {
            get;
            set;
        }

        [XmlElement("charsCycleInterval")]
        public int CharactersCycleInterval
        {
            get;
            set;
        }

        [XmlElement("useTimeFormatsCycle")]
        public bool UseTimeFormatsCycle
        {
            get;
            set;
        }

        [XmlElement("timeFormatsCycleInterval")]
        public int TimeFormatsCycleInterval
        {
            get;
            set;
        }

        [XmlElement("showSystemTime")]
        public bool ShowSystemTime
        {
            get;
            set;
        }

        [XmlElement("showEVETime")]
        public bool ShowEVETime
        {
            get;
            set;
        }

        internal G15Settings Clone()
        {
            return (G15Settings)MemberwiseClone();
        }
    }
}
