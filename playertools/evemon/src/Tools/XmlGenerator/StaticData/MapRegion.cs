using System;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class MapRegion : IHasID
    {
        [XmlElement("regionID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("regionName")]
        public string Name;

        [XmlElement("factionID")]
        public Nullable<int> FactionID;
    }
}
