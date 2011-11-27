using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a characters' sheet. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPICharacterSheet : SerializableCharacterSheetBase
    {
        [XmlElement("attributeEnhancers")]
        public SerializableImplantSet Implants
        {
            get;
            set;
        }
    }
}
