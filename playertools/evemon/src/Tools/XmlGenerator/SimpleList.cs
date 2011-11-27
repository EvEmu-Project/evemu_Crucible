using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator
{
    /// <summary>
    /// Simple wrapper for list of objects of type T
    /// </summary>
    /// <typeparam name="T"></typeparam>
    [XmlRoot("list")]
    public sealed class SimpleList<T>
    {
        /// <summary>
        /// Construcor
        /// </summary>
        public SimpleList()
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
    }
}
