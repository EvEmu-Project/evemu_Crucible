using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class DgmTypeEffect
        : IRelation
    {
        [XmlElement("typeID")]
        public int ItemID;

        [XmlElement("effectID")]
        public int EffectID;

        #region IRelation Members
        int IRelation.Left
        {
            get { return ItemID; }
        }

        int IRelation.Right
        {
            get { return EffectID; }
        }
        #endregion
    }
}
