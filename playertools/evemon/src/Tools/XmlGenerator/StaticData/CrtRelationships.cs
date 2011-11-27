using System;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class CrtRelationships : IHasID
    {
        [XmlElement("relationshipID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("parentID")]
        public Nullable<int> ParentID;

        [XmlElement("parentTypeID")]
        public Nullable<int> ParentTypeID;

        [XmlElement("parentLevel")]
        public Nullable<int> ParentLevel;

        [XmlElement("childID")]
        public int ChildID;
    }

}
