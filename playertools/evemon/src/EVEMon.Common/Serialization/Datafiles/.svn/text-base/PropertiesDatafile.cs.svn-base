using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Datafiles
{
    [XmlRoot("propertiesDatafile")]
    public sealed class PropertiesDatafile
    {
        [XmlElement("category")]
        public SerializablePropertyCategory[] Categories
        {
            get;
            set;
        }
    }


    public sealed class SerializablePropertyCategory
    {
        [XmlElement("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlElement("description")]
        public string Description
        {
            get;
            set;
        }

        [XmlArray("properties")]
        [XmlArrayItem("property")]
        public SerializableProperty[] Properties
        {
            get;
            set;
        }
    }


    public sealed class SerializableProperty
    {
        [XmlElement("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlElement("description")]
        public string Description
        {
            get;
            set;
        }

        [XmlElement("defaultValue")]
        public string DefaultValue
        {
            get;
            set;
        }

        [XmlElement("icon")]
        public string Icon
        {
            get;
            set;
        }

        [XmlElement("unit")]
        public string Unit
        {
            get;
            set;
        }

        [XmlElement("unitID")]
        public int UnitID
        {
            get;
            set;
        }

        [XmlElement("higherIsBetter")]
        public bool HigherIsBetter
        {
            get;
            set;
        }
    }
}
