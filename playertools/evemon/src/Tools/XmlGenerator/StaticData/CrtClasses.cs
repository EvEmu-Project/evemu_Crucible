using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class CrtClasses : IHasID
    {
        [XmlElement("classID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("description")]
        public string Description;

        [XmlElement("className")]
        public string ClassName;

    }
}
