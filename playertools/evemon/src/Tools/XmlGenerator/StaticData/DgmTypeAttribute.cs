using System;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class DgmTypeAttribute : IRelation
    {
        [XmlElement("typeID")]
        public int ItemID;

        [XmlElement("attributeID")]
        public int AttributeID;

        [XmlElement("valueInt")]
        public Nullable<int> ValueInt;

        [XmlElement("valueFloat")]
        public Nullable<double> ValueFloat;

        /// <summary>
        /// Returns the value as an integer. 
        /// Some int values are actually stored as floats in the DB, hence this trick.
        /// </summary>
        /// <returns></returns>
        public int GetIntValue()
        {
            if (ValueInt.HasValue) return ValueInt.Value;
            return (int)ValueFloat.Value;
        }

        #region IRelation Members
        int IRelation.Left
        {
            get { return ItemID; }
        }

        int IRelation.Right
        {
            get { return AttributeID; }
        }
        #endregion
    }
}
