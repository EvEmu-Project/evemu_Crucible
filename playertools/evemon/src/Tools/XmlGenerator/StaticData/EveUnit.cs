using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class EveUnit : IHasID
    {
        [XmlElement("unitID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("unitName")]
        public string Name;

        [XmlElement("displayName")]
        public string DisplayName;

        [XmlElement("description")]
        public string Description;

    }
}
