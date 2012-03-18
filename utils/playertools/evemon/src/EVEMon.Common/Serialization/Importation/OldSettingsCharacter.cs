using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Importation
{
    /// <summary>
    /// Facilitates importation of characters from the settings XML 
    /// of versions of EVEMon prior to 1.3.0.
    /// </summary>
    /// <remarks>
    /// These changes were released early 2010, it is safe to assume
    /// that they can be removed from the project early 2012.
    /// </remarks>
    public sealed class OldSettingsCharacter
    {
        [XmlAttribute("id")]
        public int ID
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
    }
}
