using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Collections
{
    /// <summary>
    /// Represents a base readonly collection based on an inner dictionary.
    /// </summary>
    public abstract class ReadonlyKeyedCollection<TKey, TItem> : IReadonlyKeyedCollection<TKey, TItem>
        where TItem : class
    {
        protected readonly Dictionary<TKey, TItem> m_items = new Dictionary<TKey, TItem>();

        /// <summary>
        /// Protected default constructor
        /// </summary>
        protected ReadonlyKeyedCollection()
        {
        }

        /// <summary>
        /// Gets the number of items in this collection
        /// </summary>
        public int Count
        {
            get { return m_items.Count; }
        }

        /// <summary>
        /// Gets the item with the given key
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        protected TItem GetByKey(TKey key)
        {
            TItem item = null;
            m_items.TryGetValue(key, out item);
            return item;
        }

        #region Enumerators
        IEnumerator<TItem> IEnumerable<TItem>.GetEnumerator()
        {
            return m_items.Values.GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return m_items.Values.GetEnumerator();
        }
        #endregion


        #region IReadonlyKeyedCollection<TKey,TItem> Members
        TItem IReadonlyKeyedCollection<TKey, TItem>.this[TKey key]
        {
            get { return GetByKey(key); }
        }
        #endregion
    }
}
