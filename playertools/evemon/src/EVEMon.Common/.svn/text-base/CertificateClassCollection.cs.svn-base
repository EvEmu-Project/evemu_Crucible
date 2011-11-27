using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Collections;

namespace EVEMon.Common
{
    /// <summary>
    /// Represetns a collection of character's certificate class
    /// </summary>
    public sealed class CertificateClassCollection : ReadonlyKeyedCollection<string, CertificateClass>
    {
        private readonly Character m_character;

        /// <summary>
        /// Constructor
        /// </summary>
        internal CertificateClassCollection(Character character)
        {
            m_character = character;

            foreach(var category in character.CertificateCategories)
            {
                foreach(var certClass in category)
                {
                    m_items[certClass.Name] = certClass;
                }
            }
        }

        /// <summary>
        /// Gets a certificate class from its name
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public CertificateClass this[string name]
        {
            get { return GetByKey(name); }
        }
    }
}
