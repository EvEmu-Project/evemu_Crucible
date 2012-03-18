using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Attributes;

namespace EVEMon.Common.Collections
{
    /// <summary>
    /// Represents a base readonly collection relying on an inner list.
    /// </summary>
    public abstract class ReadonlyCollection<T> : IReadonlyCollection<T>
    {
        protected FastList<T> m_items;

        /// <summary>
        /// Protected default constructor with an initial capacity.
        /// </summary>
        protected ReadonlyCollection(int capacity)
        {
            m_items = new FastList<T>(capacity);
        }

        /// <summary>
        /// Protected default constructor
        /// </summary>
        protected ReadonlyCollection()
        {
            m_items = new FastList<T>(0);
        }

        /// <summary>
        /// Gets the number of items in this collection
        /// </summary>
        public int Count
        {
            get { return m_items.Count; }
        }

        /// <summary>
        /// Gets true if the collection contains the given item
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public bool Contains(T item)
        {
            return m_items.Contains(item);
        }

        #region Enumerators
        IEnumerator<T> IEnumerable<T>.GetEnumerator()
        {
            return m_items.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_items.GetEnumerator();
        }
        #endregion
    }
}
