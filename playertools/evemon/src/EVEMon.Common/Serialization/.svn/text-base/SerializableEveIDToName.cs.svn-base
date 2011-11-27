using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    [XmlRoot("EveIDToName")]
    public sealed class SerializableEveIDToName
    {
        public SerializableEveIDToName()
        {
            Entities = new List<SerializableEveIDToNameListItem>();
        }

        [XmlArray("entities")]
        [XmlArrayItem("entity")]
        public List<SerializableEveIDToNameListItem> Entities
        {
            get;
            set;
        }
    }
}
