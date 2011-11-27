using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Datafiles
{
    [XmlRoot("reprocessingDatafile")]
    public sealed class ReprocessingDatafile
    {
        [XmlElement("item")]
        public SerializableItemMaterials[] Items
        {
            get;
            set;
        }
    }

    public sealed class SerializableItemMaterials
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("material")]
        public SerializableMaterialQuantity[] Materials
        {
            get;
            set;
        }
    }

    public sealed class SerializableMaterialQuantity
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("quantity")]
        public int Quantity
        {
            get;
            set;
        }
    }
}
