using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class ItemBrowserSettings
    {
        public ItemBrowserSettings()
        {
            UsabilityFilter = ObjectUsabilityFilter.All;
            MetagroupFilter = ItemMetaGroup.All;
            SlotFilter = ItemSlot.All;
        }

        [XmlElement("showAllGroups")]
        public bool ShowAllGroups
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

        [XmlElement("usabilityFilter")]
        public ObjectUsabilityFilter UsabilityFilter
        {
            get;
            set;
        }

        [XmlElement("slotFilter")]
        public ItemSlot SlotFilter
        {
            get;
            set;
        }

        [XmlElement("metaGroupFilter")]
        public ItemMetaGroup MetagroupFilter
        {
            get;
            set;
        }

        internal ItemBrowserSettings Clone()
        {
            return (ItemBrowserSettings)MemberwiseClone();
        }
    }
}
