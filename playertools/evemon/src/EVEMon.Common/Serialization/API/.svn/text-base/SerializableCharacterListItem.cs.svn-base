using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a reference to a character in the charactersList API
    /// </summary>
    public sealed class SerializableCharacterListItem : ISerializableCharacterIdentity
    {
        /// <summary>
        /// Default constructor for XML serialization
        /// </summary>
        public SerializableCharacterListItem()
        {
        }

        [XmlAttribute("characterID")]
        public long ID
        {
            get;
            set;
        }

        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("corporationName")]
        public string CorporationName
        {
            get;
            set;
        }

        [XmlAttribute("corporationID")]
        public long CorporationID
        {
            get;
            set;
        }
    }
}
