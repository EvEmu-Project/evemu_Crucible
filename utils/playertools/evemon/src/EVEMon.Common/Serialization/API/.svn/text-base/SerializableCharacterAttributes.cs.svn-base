using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents the character's attributes
    /// </summary>
    public sealed class SerializableCharacterAttributes
    {
        public SerializableCharacterAttributes()
        {
            Intelligence = Memory = Perception = Charisma = Willpower = 1;
        }

        [XmlElement("intelligence")]
        public int Intelligence
        {
            get;
            set;
        }

        [XmlElement("memory")]
        public int Memory
        {
            get;
            set;
        }

        [XmlElement("perception")]
        public int Perception
        {
            get;
            set;
        }

        [XmlElement("willpower")]
        public int Willpower
        {
            get;
            set;
        }

        [XmlElement("charisma")]
        public int Charisma
        {
            get;
            set;
        }
    }
}
