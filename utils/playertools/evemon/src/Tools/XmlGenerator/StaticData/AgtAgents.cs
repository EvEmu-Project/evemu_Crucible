using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class AgtAgents : IHasID
    {
        [XmlElement("agentID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("locationID")]
        public int LocationID;

        [XmlElement("level")]
        public int Level;

        [XmlElement("quality")]
        public int Quality;
    }
}
