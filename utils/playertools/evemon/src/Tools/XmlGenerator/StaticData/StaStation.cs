using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class StaStation : IHasID
    {
        [XmlElement("stationID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("stationName")]
        public string Name;

        [XmlElement("security")]
        public int SecurityLevel;

        [XmlElement("corporationID")]
        public int CorporationID;

        [XmlElement("solarSystemID")]
        public int SolarSystemID;

        [XmlElement("reprocessingEfficiency")]
        public float ReprocessingEfficiency;

        [XmlElement("reprocessingStationsTake")]
        public float ReprocessingStationsTake;
    }
}
