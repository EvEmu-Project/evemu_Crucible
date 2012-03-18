using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator
{
    /// <summary>
    /// List of related items of type T
    /// </summary>
    /// <typeparam name="T"></typeparam>
    [XmlRoot("list")]
    public sealed class Relations<T>
        where T : class, IRelation
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public Relations()
        {
            Items = new List<T>();
        }

        /// <summary>
        /// List of items in the list
        /// </summary>
        [XmlElement("item")]
        public List<T> Items
        {
            get;
            set;
        }

        public RelationSet<T> ToSet()
        {
            return new RelationSet<T>(Items);
        }
    }
}
