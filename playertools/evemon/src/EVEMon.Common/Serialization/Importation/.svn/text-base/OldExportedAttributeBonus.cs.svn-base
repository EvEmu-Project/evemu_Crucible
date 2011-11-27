using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Importation
{
    /// <summary>
    /// Facilitates importation of file characters from versions of
    /// EVEMon prior to 1.3.0.
    /// </summary>
    /// <remarks>
    /// These changes were released early 2010, it is safe to assume
    /// that they can be removed from the project early 2012.
    /// </remarks>
    public sealed class OldExportedAttributeBonus
    {
        private string m_name;
        private int m_amount;

        [XmlElement("augmentatorName")]
        public string Name
        {
            get { return m_name; }
            set { m_name = value; }
        }

        [XmlElement("augmentatorValue")]
        public int Amount
        {
            get { return m_amount; }
            set { m_amount = value; }
        }

        private bool m_manual = false;

        [XmlAttribute]
        public bool Manual
        {
            get { return m_manual; }
            set { m_manual = value; }
        }
    }
}
