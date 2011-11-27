using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator
{
    /// <summary>
    /// A list of indexed items
    /// </summary>
    /// <typeparam name="T"></typeparam>
    [XmlRoot("list")]
    public sealed class IndexedList<T>
        where T:IHasID
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public IndexedList()
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

        /// <summary>
        /// Converts list to a Bag
        /// </summary>
        /// <returns>Bag of type T</returns>
        public Bag<T> ToBag()
        {
            return new Bag<T>(this);
        }
    }
}
