using System;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class DgmAttributeTypes : IHasID
    {
        [XmlElement("attributeID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("description")]
        public string Description;

        [XmlElement("attributeName")]
        public string Name;

        [XmlElement("displayName")]
        public string DisplayName;

        [XmlElement("defaultValue")]
        public string DefaultValue;

        [XmlElement("iconID")]
        public Nullable<int> IconID;

        [XmlElement("unitID")]
        public Nullable<int> UnitID;

        [XmlElement("categoryID")]
        public Nullable<int> CategoryID;

        [XmlElement("highIsGood")]
        public bool HigherIsBetter;

    }
}
