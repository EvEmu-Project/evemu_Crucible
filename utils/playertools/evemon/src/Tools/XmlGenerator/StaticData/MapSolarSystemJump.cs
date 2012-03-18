using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class MapSolarSystemJump
    {
        [XmlElement("fromSolarSystemID")]
        public int A;

        [XmlElement("toSolarSystemID")]
        public int B;
    }
}
