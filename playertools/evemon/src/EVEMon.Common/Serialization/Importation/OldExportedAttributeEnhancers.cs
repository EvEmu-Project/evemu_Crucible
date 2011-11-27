using System.Xml.Serialization;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common.Serialization.Importation
{
    /// <summary>
    /// Facilitates importation of file characters from versions of
    /// EVEMon prior to 1.3.0.
    /// </summary>
    /// <remarks>
    /// These changes were released early 2010, it is safe to assume
    /// that they can be removed from the project early 2012.
    /// 
    /// NOTE: currently implants are not imported from the old file
    /// format.
    /// </remarks>
    [XmlRoot("attributeEnhancers")]
    public sealed class OldExportedAttributeEnhancers
    {
        [XmlElement("intelligenceBonus")]
        public int Inteligence
        {
            get;
            set;
        }
        [XmlElement("charismaBonus")]
        public int Charisma
        {
            get;
            set;
        }
        [XmlElement("perceptionBonus")]
        public int Perception
        {
            get;
            set;
        }
        [XmlElement("memoryBonus")]
        public int Memory
        {
            get;
            set;
        }
        [XmlElement("willpowerBonus")]
        public int Willpower
        {
            get;
            set;
        }

        internal SerializableSettingsImplantSet ToSerializableImplantSet()
        {
            var ccpImplantSet = new SerializableSettingsImplantSet();
            return ccpImplantSet;
        }
    }
}
