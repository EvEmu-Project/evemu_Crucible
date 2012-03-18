using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace EVEMon.Common.Collections
{
    /// <summary>
    /// Implements an as fast and light list as possible. No checks are performed, no synchronization possibilities, etc, etc, etc.
    /// </summary>
    /// <remarks>
    /// Beware ! This list is a struct. Resizing it (or adding items and such) may mean to change the array and, therefore, the structure.
    /// Also, this structure *must* be initialized with the custom capacity-based constructor to avoid <see cref="NullReferenceException"/>.
    /// Use when you're sure about what you're doing.
    /// </remarks>
    /// <typeparam name="T"></typeparam>
    public struct FastList<T> : IList<T>
    {
        #region Enumerator
        /// <summary>
        /// An enumerator for the FastList
        /// </summary>
        private struct Enumerator : IEnumerator<T>
        {
            private T[] m_items;
            private int m_count;
            private int m_index;

            /// <summary>
            /// Constructor
            /// </summary>
            /// <param name="items"></param>
            /// <param name="count"></param>
            public Enumerator(T[] items, int count)
            {
                this.m_items = items;
                this.m_count = count;
                this.m_index = -1;
            }

            public T Current
            {
                get { return this.m_items[this.m_index]; }
            }

            public void Dispose()
            {
                this.m_items = null;
            }

            object System.Collections.IEnumerator.Current
            {
                get { return (object)this.m_items[m_index]; }
            }

            public bool MoveNext()
            {
                this.m_index++;
                return (this.m_index < m_count);
            }

            public void Reset()
            {
                this.m_index = -1;
            }
        }
        #endregion

        private int m_count;
        private T[] m_items;

        /// <summary>
        /// Constructor with a starting capacity. Always use this constructor, not the default one !
        /// </summary>
        /// <param name="capacity">Initial capacity of the list</param>
        public FastList(int capacity)
        {
            m_items = new T[capacity];
            m_count = 0;
        }

        /// <summary>
        /// Gets the number of items within this list
        /// </summary>
        public int Count
        {
            get { return m_count; }
        }

        /// <summary>
        /// Gets or sets the current capacity of the list.
        /// </summary>
        public int Capacity
        {
            get { return m_items.Length; }
            set
            {
                if (value < m_count)
                {
                    throw new ArgumentException("The given count is lesser than the items in the list.");
                }
                Array.Resize(ref m_items, value);
            }
        }

        /// <summary>
        /// Gets / sets the item at the given index
        /// </summary>
        /// <param name="index">The index where the item is located</param>
        /// <returns>The item found at the given index</returns>
        public T this[int index]
        {
            get { return m_items[index]; }
            set { m_items[index] = value; }
        }

        /// <summary>
        /// Gets the index of an item within the list
        /// </summary>
        /// <param name="item">The item to search for</param>
        /// <returns>The index where the item was found, -1 otherwise</returns>
        public int IndexOf(T item)
        {
            var comparer = EqualityComparer<T>.Default;

            for (int i = 0; i < m_count; i++)
            {
                if (comparer.Equals(m_items[i], item)) return i;
            }
            return -1;
        }

        /// <summary>
        /// Adds an item at the end of the list
        /// </summary>
        /// <param name="item">The item to add</param>
        public void Add(T item)
        {
            // We ensure the capacity is high enough
            if (m_count == m_items.Length)
            {
                int newSize = Math.Max(1, m_count + 1);
                this.Capacity = newSize;
            }

            // We add the item
            m_items[m_count] = item;

            // Finally, we increase the count
            m_count++;
        }

        /// <summary>
        /// Add items at the end of the list
        /// </summary>
        /// <param name="enumerable">The enumeration containing the items to add</param>
        public void AddRange(IEnumerable<T> enumerable)
        {
            // Scroll through the items to add
            foreach (var item in enumerable)
            {
                // Ensure size is enough
                if (m_count == m_items.Length)
                {
                    int newSize = Math.Max(1, m_count + 1);
                    Array.Resize(ref m_items, newSize);
                }

                // We add the item
                m_items[m_count] = item;

                // Finally, we increase the count
                m_count++;
            }
        }

        /// <summary>
        /// Add items at the end of the list
        /// </summary>
        /// <param name="list">The list containing the items to add</param>
        public void AddRange(FastList<T> list)
        {
            this.AddRange(list.m_items, list.m_count);
        }

        /// <summary>
        /// Add items at the end of the list
        /// </summary>
        /// <param name="newItems">The array containing the items to add</param>
        /// <param name="newCount">The number of items to add</param>
        public void AddRange(T[] newItems, int newCount)
        {
            // We ensure the capacity is high enough
            if (m_count + newCount > m_items.Length)
            {
                int newSize = Math.Max(m_count + newCount, m_count + 1);
                Array.Resize(ref m_items, newSize);
            }

            // Append items
            Array.Copy(newItems, 0, m_items, m_count, newCount);

            // Finally, we increase the count
            this.m_count += newCount;
        }

        /// <summary>
        /// Insert an item at the given index
        /// </summary>
        /// <param name="index">The item to insert</param>
        /// <param name="item">The index to insert the item in</param>
        public void Insert(int index, T item)
        {
            // We ensure the capacity is high enough
            if (m_count == m_items.Length)
            {
                int newSize = Math.Max(1, m_count + 1);
                Array.Resize(ref m_items, newSize);
            }

            // Do we need to shift items before ? (insertion rather than addition)
            if (index < m_count)
            {
                Array.Copy(this.m_items, index, this.m_items, index + 1, this.m_count - index);
            }
            m_items[index] = item;

            // Finally, we increase the count
            m_count++;
        }

        /// <summary>
        /// Removes the item located at the specified index
        /// </summary>
        /// <param name="index">The index to remove the item from</param>
        public void RemoveAt(int index)
        {
            // When item is not the last item, shift the items after it to the left
            if (index < m_count - 1)
            {
                Array.Copy(m_items, index + 1, m_items, index, m_count - (index + 1));
            }

            // Make sure we don't hold a reference anymore over the left item
            m_items[m_count - 1] = default(T);

            // Updates the count
            m_count--;
        }

        /// <summary>
        /// Removes the specified item from the list
        /// </summary>
        /// <param name="item">The item to remove</param>
        /// <returns>True if the item was found, false otherwise</returns>
        public bool Remove(T item)
        {
            var comparer = EqualityComparer<T>.Default;

            for (int i = 0; i < m_count; i++)
            {
                if (comparer.Equals(m_items[i], item))
                {
                    RemoveAt(i);
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Clears the list
        /// </summary>
        public void Clear()
        {
            m_count = 0;
            Array.Clear(m_items, 0, m_count);
        }

        /// <summary>
        /// Checks whether a given item is present in the list
        /// </summary>
        /// <param name="item">The item to search for</param>
        /// <returns>True if the item was found in this list, false otherwise</returns>
        public bool Contains(T item)
        {
            var comparer = EqualityComparer<T>.Default;

            for (int i = 0; i < m_count; i++)
            {
                if (comparer.Equals(m_items[i], item)) return true;
            }
            return false;
        }

        /// <summary>
        /// Copy those list's items in the specified array
        /// </summary>
        /// <param name="array">The destination array</param>
        /// <param name="arrayIndex">The starynig index in the destination array</param>
        public void CopyTo(T[] array, int arrayIndex)
        {
            Array.Copy(m_items, 0, array, arrayIndex, m_count);
        }

        /// <summary>
        /// Reverses the list
        /// </summary>
        public void Reverse()
        {
            Array.Reverse(m_items);
        }

        /// <summary>
        /// Gets an enumerator over this list
        /// </summary>
        /// <returns>The enumerator over this list</returns>
        public IEnumerator<T> GetEnumerator()
        {
            if (m_items == null || m_count == 0) return EmptyEnumerator<T>.Instance;
            return new Enumerator(m_items, m_count);
        }

        /// <summary>
        /// Gets a non-generic enumerator over this list
        /// </summary>
        /// <returns>The enumerator over this list</returns>
        IEnumerator IEnumerable.GetEnumerator()
        {
            if (m_items == null || m_count == 0) return EmptyEnumerator<T>.Instance;
            return new Enumerator(m_items, m_count);
        }

        /// <summary>
        /// Gets false, since the list is not readonly
        /// </summary>
        bool ICollection<T>.IsReadOnly
        {
            get { return false; }
        }

        /// <summary>
        /// Trims the list.
        /// </summary>
        internal void Trim()
        {
            Array.Resize(ref m_items, m_count);
        }
    }
}
