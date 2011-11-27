using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a characters list. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPICharacters
    {
        [XmlArray("characters")]
        [XmlArrayItem("character")]
        public List<SerializableCharacterListItem> Characters
        {
            get;
            set;
        }
    }
}
