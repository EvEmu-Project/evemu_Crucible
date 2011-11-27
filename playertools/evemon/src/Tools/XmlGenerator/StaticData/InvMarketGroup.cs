using System;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class InvMarketGroup : IHasID
    {
        [XmlElement("marketGroupID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("marketGroupName")]
        public string Name;

        [XmlElement("parentGroupID")]
        public Nullable<int> ParentID;

        [XmlElement("iconID")]
        public Nullable<int> IconID;

        [XmlElement("description")]
        public string Description;

    }
}
