using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Attributes;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a certificate category.
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class CertificateCategoryCollection : ReadonlyKeyedCollection<string, CertificateCategory>
    {
        private readonly Character m_character;

        /// <summary>
        /// Constructor for the character initialization
        /// </summary>
        /// <param name="character"></param>
        internal CertificateCategoryCollection(Character character)
        {
            m_character = character;

            foreach (var srcCategory in StaticCertificates.Categories)
            {
                var category = new CertificateCategory(character, srcCategory);
                m_items[category.Name] = category;
            }
        }
    }
}
