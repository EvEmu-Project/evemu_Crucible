using System.Collections.Generic;

namespace EVEMon.XmlGenerator
{
    public sealed class RelationSet<T> : IEnumerable<T>
        where T : class, IRelation
    {
        private readonly Dictionary<long, T> m_dictionary;

        public RelationSet(IEnumerable<T> src)
        {
            m_dictionary = new Dictionary<long, T>();
            foreach (var item in src)
            {
                m_dictionary[GetKey(item)] = item;
            }
        }

        public bool Contains(int left, int right)
        {
            return m_dictionary.ContainsKey(GetKey(left, right));
        }

        public T Get(int left, int right)
        {
            T value = null;
            m_dictionary.TryGetValue(GetKey(left, right), out value);
            return value;
        }

        public static long GetKey(IRelation relation)
        {
            return GetKey(relation.Left, relation.Right);
        }

        public static long GetKey(int left, int right)
        {
            return (((long)left) << 32) | (uint)right;
        }

        public IEnumerator<T> GetEnumerator()
        {
            foreach (var item in m_dictionary.Values)
            {
                yield return item;
            }
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return (System.Collections.IEnumerator)GetEnumerator();
        }
    }
}
