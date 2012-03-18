using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class ShipBrowserSettings
    {
        public ShipBrowserSettings()
        {
            UsabilityFilter = ObjectUsabilityFilter.All;
            RacesFilter = Race.All;
        }

        [XmlElement("usabilityFilter")]
        public ObjectUsabilityFilter UsabilityFilter
        {
            get;
            set;
        }

        [XmlElement("racesFilter")]
        public Race RacesFilter
        {
            get;
            set;
        }

        [XmlElement("textSearch")]
        public string TextSearch
        {
            get;
            set;
        }

        internal ShipBrowserSettings Clone()
        {
            return (ShipBrowserSettings)MemberwiseClone();
        }
    }
}
