using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Collections
{
    /// <summary>
    /// A base class for lists when we want to be able to connect or disconnect items, or fire colection changes.
    /// </summary>
    public abstract class BaseList<T> : IList<T>
        where T : class
    {
        protected List<T> m_items = new List<T>();

        /// <summary>
        /// Constructor
        /// </summary>
        protected BaseList()
        {
        }

        /// <summary>
        /// Executed any time an item is going to be added to the list.
        /// </summary>
        /// <param name="item"></param>
        protected virtual void OnAdding(ref T item)
        {
        }

        /// <summary>
        /// Executed any time an item is going to be removed from the list.
        /// </summary>
        /// <param name="item"></param>
        protected virtual void OnRemoving(T oldItem)
        {
        }

        /// <summary>
        /// Executed any time a change is going to occur to the list.
        /// </summary>
        /// <param name="item"></param>
        protected virtual void OnChanged()
        {
        }

        /// <summary>
        /// Moves the given item to the target index.
        /// </summary>
        /// <param name="item"></param>
        /// <param name="targetIndex"></param>
        public void MoveTo(T item, int targetIndex)
        {
            int oldIndex = m_items.IndexOf(item);
            if (oldIndex == -1) throw new InvalidOperationException("The item was not found in the collection.");

            m_items.RemoveAt(oldIndex);
            m_items.Insert(targetIndex, item);

            OnChanged();
        }

        /// <summary>
        /// Rebuild the list from the given enumeration
        /// </summary>
        /// <param name="items"></param>
        public void RebuildFrom(IEnumerable<T> items)
        {
            // Removing old items
            foreach (var item in m_items) OnRemoving(item);
            m_items.Clear();

            // Adding new items
            foreach(var item in items)
            {
                var copy = item;
                OnAdding(ref copy);
                m_items.Add(copy);
            }
            OnChanged();
        }


        #region IList<T> Members
        /// <summary>
        /// Gets the index of the given item, -1 when not found.
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public int IndexOf(T item)
        {
            return m_items.IndexOf(item);
        }

        /// <summary>
        /// Inserts the given item at the provided index
        /// </summary>
        /// <param name="index"></param>
        /// <param name="item"></param>
        public void Insert(int index, T item)
        {
            OnAdding(ref item);
            m_items.Insert(index, item);
            OnChanged();
        }

        /// <summary>
        /// Removes the item at the provided index
        /// </summary>
        /// <param name="index"></param>
        public void RemoveAt(int index)
        {
            T item = m_items[index];

            OnRemoving(item);
            m_items.RemoveAt(index);
            OnChanged();
        }

        /// <summary>
        /// Gets or sets the item at the given index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public T this[int index]
        {
            get
            {
                return m_items[index];
            }
            set
            {
                T item = m_items[index];

                OnRemoving(item);
                OnAdding(ref value);
                m_items[index] = value;
                OnChanged();
            }
        }
        #endregion


        #region ICollection<T> Members
        /// <summary>
        /// Adds an item at the end of the list.
        /// </summary>
        /// <param name="item"></param>
        public void Add(T item)
        {
            OnAdding(ref item);
            m_items.Add(item);
            OnChanged();
        }

        /// <summary>
        /// Removes the item from the list
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public bool Remove(T item)
        {
            OnRemoving(item);
            if (m_items.Remove(item))
            {
                OnChanged();
                return true;
            }
            return false;
        }

        /// <summary>
        /// Clears the list
        /// </summary>
        public void Clear()
        {
            foreach (var item in m_items) OnRemoving(item);
            m_items.Clear();
            OnChanged();
        }

        /// <summary>
        /// Gets true whether the list contains the given item.
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public bool Contains(T item)
        {
            return m_items.Contains(item);
        }

        /// <summary>
        /// Copy the list to the given array.
        /// </summary>
        /// <param name="array"></param>
        /// <param name="arrayIndex"></param>
        public void CopyTo(T[] array, int arrayIndex)
        {
            m_items.CopyTo(array, arrayIndex);
        }

        /// <summary>
        /// Gets the number of items in the list
        /// </summary>
        public int Count
        {
            get { return m_items.Count; }
        }

        /// <summary>
        /// Gets false
        /// </summary>
        public bool IsReadOnly
        {
            get { return false; }
        }
        #endregion


        #region IEnumerable<T> Members
        /// <summary>
        /// Gets an enumerator over this list.
        /// </summary>
        /// <returns></returns>
        public IEnumerator<T> GetEnumerator()
        {
            return m_items.GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return m_items.GetEnumerator();
        }
        #endregion
    }
}
