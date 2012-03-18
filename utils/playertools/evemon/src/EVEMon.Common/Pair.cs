using System.Xml.Serialization;

namespace EVEMon.Common
{
    [XmlRoot("pair")]
    public class Pair<TypeA, TypeB>
    {
        private TypeA m_a;
        private TypeB m_b;

        public TypeA A
        {
            get { return m_a; }
            set { m_a = value; }
        }

        public TypeB B
        {
            get { return m_b; }
            set { m_b = value; }
        }

        public Pair()
        {
        }

        public Pair(TypeA a, TypeB b)
        {
            m_a = a;
            m_b = b;
        }
    }
}