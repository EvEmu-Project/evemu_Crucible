using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class CrtRecommendations : IHasID
    {
        [XmlElement("recommendationID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("shipTypeID")]
        public int ShipTypeID;

        [XmlElement("certificateID")]
        public int CertificateID;

        [XmlElement("recommendationLevel")]
        public int Level;

    }
}
