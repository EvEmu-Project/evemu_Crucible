using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class InvMetaType : IRelation
    {
        [XmlElement("typeID")]
        public int ItemID;

        [XmlElement("parentTypeID")]
        public int ParentItemID;

        [XmlElement("metaGroupID")]
        public int MetaGroupID;

        #region IRelation Members
        int IRelation.Left
        {
            get { return ItemID; }
        }

        int IRelation.Right
        {
            get { return MetaGroupID; }
        }
        #endregion

    }
}
