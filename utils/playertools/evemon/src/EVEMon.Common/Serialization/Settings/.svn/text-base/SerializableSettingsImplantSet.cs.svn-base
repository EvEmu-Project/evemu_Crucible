using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;
using EVEMon.Common.Data;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents the set of attributes enhancers
    /// </summary>
    public sealed class SerializableSettingsImplantSet
    {
        public SerializableSettingsImplantSet()
        {
            Name = "Custom";
            Intelligence = Implant.None.Name;
            Perception = Implant.None.Name;
            Willpower = Implant.None.Name;
            Charisma = Implant.None.Name;
            Memory = Implant.None.Name;
            Slot6 = Implant.None.Name;
            Slot7 = Implant.None.Name;
            Slot8 = Implant.None.Name;
            Slot9 = Implant.None.Name;
            Slot10 = Implant.None.Name;
        }

        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlElement("intelligence")]
        public string Intelligence
        {
            get;
            set;
        }

        [XmlElement("memory")]
        public string Memory
        {
            get;
            set;
        }

        [XmlElement("willpower")]
        public string Willpower
        {
            get;
            set;
        }

        [XmlElement("perception")]
        public string Perception
        {
            get;
            set;
        }

        [XmlElement("charisma")]
        public string Charisma
        {
            get;
            set;
        }

        [XmlElement("slot6")]
        public string Slot6
        {
            get;
            set;
        }

        [XmlElement("slot7")]
        public string Slot7
        {
            get;
            set;
        }

        [XmlElement("slot8")]
        public string Slot8
        {
            get;
            set;
        }

        [XmlElement("slot9")]
        public string Slot9
        {
            get;
            set;
        }

        [XmlElement("slot10")]
        public string Slot10
        {
            get;
            set;
        }

        public override string ToString()
        {
            return Name;
        }
    }

}
