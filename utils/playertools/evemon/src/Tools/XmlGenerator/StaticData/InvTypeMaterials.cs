using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class InvTypeMaterials
    {
        [XmlElement("typeID")]
        public int TypeID;

        [XmlElement("materialTypeID")]
        public int MaterialTypeID;

        [XmlElement("quantity")]
        public int Quantity;
    }
}
