using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class MapConstellation : IHasID
    {
        [XmlElement("constellationID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("constellationName")]
        public string Name;

        [XmlElement("regionID")]
        public int RegionID;
    }
}
