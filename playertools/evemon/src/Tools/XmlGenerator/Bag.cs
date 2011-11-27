using System.Collections.Generic;

namespace EVEMon.XmlGenerator
{
    /// <summary>
    /// A dictionary-based implementation.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public sealed class Bag<T> : IEnumerable<T>
        where T : IHasID
    {
        private Dictionary<int, T> m_items;

        public Bag()
        {
            m_items = new Dictionary<int, T>();
        }

        public Bag(IndexedList<T> list)
            : this()
        {
            foreach (var item in list.Items)
            {
                m_items[item.ID] = item;
            }
        }

        public bool HasValue(int id)
        {
            return m_items.ContainsKey(id);
        }

        public T this[int id]
        {
            get
            {
                return m_items[id];
            }
            private set
            {
                m_items[id] = value;
            }
        }

        public IEnumerator<T> GetEnumerator()
        {
            foreach (var T in m_items.Values)
            {
                yield return T;
            }
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
