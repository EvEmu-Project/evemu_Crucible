using System;
using System.Collections.Generic;
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
    [XmlRoot("skillGroup")]
    public sealed class OldExportedSkillGroup
    {
        private string m_name = String.Empty;
        private int m_id;
        private List<OldExportedSkill> m_skills = new List<OldExportedSkill>();

        [XmlAttribute("groupName")]
        public string Name
        {
            get { return m_name; }
            set { m_name = value; }
        }

        [XmlAttribute("groupID")]
        public int Id
        {
            get { return m_id; }
            set { m_id = value; }
        }

        [XmlElement("skill")]
        public List<OldExportedSkill> Skills
        {
            get { return m_skills; }
            set { m_skills = value; }
        }
    }
}
