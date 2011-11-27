using System.Xml.Serialization;

namespace EVEMon.Common.Serialization
{
    public sealed class SerializableNotificationRefTypeIDsListItem
    {
        [XmlAttribute("refTypeID")]
        public int TypeID
        {
            get;
            set;
        }

        [XmlAttribute("refTypeName")]
        public string TypeName
        {
            get;
            set;
        }
    }
}
