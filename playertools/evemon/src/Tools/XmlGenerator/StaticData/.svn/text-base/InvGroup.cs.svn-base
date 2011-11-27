using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class InvGroup : IHasID
    {
        [XmlElement("groupID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("categoryID")]
        public int CategoryID;

        [XmlElement("groupName")]
        public string Name;

        [XmlElement("published")]
        public bool Published;
    }
}
