using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents a collection of implants sets
    /// </summary>
    public sealed class SerializableImplantSetCollection
    {
        public SerializableImplantSetCollection()
        {
            API = new SerializableSettingsImplantSet();
            OldAPI = new SerializableSettingsImplantSet();
            CustomSets = new List<SerializableSettingsImplantSet>();
        }

        [XmlElement("api")]
        public SerializableSettingsImplantSet API
        {
            get;
            set;
        }

        [XmlElement("oldApi")]
        public SerializableSettingsImplantSet OldAPI
        {
            get;
            set;
        }

        [XmlElement("customSet")]
        public List<SerializableSettingsImplantSet> CustomSets
        {
            get;
            set;
        }

        [XmlElement("selectedIndex")]
        public int SelectedIndex
        {
            get;
            set;
        }
    }

}
