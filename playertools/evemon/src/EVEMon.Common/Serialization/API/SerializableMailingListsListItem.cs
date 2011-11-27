using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    public sealed class SerializableMailingListsListItem
    {
        [XmlAttribute("listID")]
        public long ID
        {
            get;
            set;
        }

        [XmlAttribute("displayName")]
        public string Name
        {
            get;
            set;
        }
    }
}
