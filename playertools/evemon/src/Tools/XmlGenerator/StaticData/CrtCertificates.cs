using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class CrtCertificates : IHasID
    {
        [XmlElement("certificateID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("categoryID")]
        public int CategoryID;

        [XmlElement("classID")]
        public int ClassID;

        [XmlElement("grade")]
        public int Grade;

        [XmlElement("description")]
        public string Description;

    }
}
