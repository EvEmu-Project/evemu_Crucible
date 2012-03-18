using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents an attribute enhancer
    /// </summary>
    public sealed class SerializableImplant
    {
        [XmlElement("augmentatorName")]
        public string Name
        {
            get;
            set;
        }

        [XmlElement("augmentatorValue")]
        public int Amount
        {
            get;
            set;
        }
    }
}
