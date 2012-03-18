using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Attributes;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a certificate category from a character's point of view.
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class CertificateCategory : ReadonlyKeyedCollection<string, CertificateClass>
    {
        private readonly Character m_character;
        private readonly StaticCertificateCategory m_staticData;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="character"></param>
        /// <param name="src"></param>
        internal CertificateCategory(Character character, StaticCertificateCategory src)
        {
            m_character = character;
            m_staticData = src;

            foreach (var srcClass in src)
            {
                var certClass = new CertificateClass(character, srcClass, this);
                m_items[certClass.Name] = certClass;
            }
        }

        /// <summary>
        /// Gets the static data associated with this object.
        /// </summary>
        public StaticCertificateCategory StaticData
        {
            get { return m_staticData; }
        }

        /// <summary>
        /// Gets this skill's id
        /// </summary>
        public long ID
        {
            get { return m_staticData.ID; }
        }

        /// <summary>
        /// Gets this skill's name
        /// </summary>
        public string Name
        {
            get { return m_staticData.Name; }
        }

        /// <summary>
        /// Gets this skill's description
        /// </summary>
        public string Description
        {
            get { return m_staticData.Description; }
        }
    }
}
