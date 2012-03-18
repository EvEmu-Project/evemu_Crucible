using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Datafiles
{
    /// <summary>
    /// Root ItemsDatafile Serialization Class
    /// </summary>
    [XmlRoot("itemsDatafile")]
    public sealed class ItemsDatafile
    {
        [XmlElement("marketGroup")]
        public SerializableMarketGroup[] MarketGroups
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents an items category (standard item categories, not market groups) from our datafile
    /// </summary>
    public sealed class SerializableMarketGroup
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("Name")]
        public string Name
        {
            get;
            set;
        }

        [XmlArray("items")]
        [XmlArrayItem("item")]
        public SerializableItem[] Items
        {
            get;
            set;
        }

        [XmlArray("marketGroups")]
        [XmlArrayItem("marketGroup")]
        public SerializableMarketGroup[] SubGroups
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Base serializable class for items, ships and implants.
    /// </summary>
    public sealed class SerializableItem
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("name")]
        public string Name
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

        [XmlAttribute("icon")]
        public string Icon
        {
            get;
            set;
        }

        [XmlAttribute("race")]
        public Race Race
        {
            get;
            set;
        }

        [XmlAttribute("metaLevel")]
        public int MetaLevel
        {
            get;
            set;
        }

        [XmlAttribute("metaGroup")]
        public ItemMetaGroup MetaGroup
        {
            get;
            set;
        }

        [XmlAttribute("slot")]
        public ItemSlot Slot
        {
            get;
            set;
        }

        [XmlAttribute("family")]
        public ItemFamily Family
        {
            get;
            set;
        }

        [XmlElement("s")]
        public SerializablePrerequisiteSkill[] Prereqs
        {
            get;
            set;
        }

        [XmlElement("p")]
        public SerializablePropertyValue[] Properties
        {
            get;
            set;
        }

        [XmlElement("r")]
        public SerializableReprocessingMaterial[] Reprocessing
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a prerequisite skill for ships, items, implants.
    /// </summary>
    public sealed class SerializablePrerequisiteSkill
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("lv")]
        public int Level
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents an Eve property for ships, items and implants.
    /// </summary>
    public sealed class SerializablePropertyValue
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("value")]
        public string Value
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a reprocessing material for ships, items and implants.
    /// </summary>
    public sealed class SerializableReprocessingMaterial
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("quantity")]
        public string Quantity
        {
            get;
            set;
        }
    }
}
