using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Attributes;
using EVEMon.Common.Collections;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a collection of certificates
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class CertificateCollection : ReadonlyKeyedCollection<long, Certificate>
    {
        private readonly Character m_character;

        /// <summary>
        /// Constructor
        /// </summary>
        internal CertificateCollection(Character character)
        {
            m_character = character;

            // Builds the list
            foreach(var category in character.CertificateCategories)
            {
                foreach(var certClass in category)
                {
                    foreach(var cert in certClass)
                    {
                        m_items[cert.ID] = cert;
                    }
                }
            }

            // Builds the prerequisites certificates list.
            foreach (var cert in m_items.Values)
            {
                cert.CompleteInitialization(m_items);
            }
        }

        /// <summary>
        /// Gets a certificate from its ID
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public Certificate this[long id]
        {
            get { return GetByKey(id); }
        }

        /// <summary>
        /// Gets the certificates with the specified status
        /// </summary>
        /// <param name="status">The status the certificates must have</param>
        /// <returns></returns>
        public IEnumerable<Certificate> FilterByStatus(CertificateStatus status)
        {
            foreach(var cert in m_items.Values)
            {
                if (cert.Status == status) 
                {
                    yield return cert;
                }
            }
        }

        /// <summary>
        /// Gets the certificates granted to that character
        /// </summary>
        public IEnumerable<Certificate> GrantedCertificates
        {
            get
            {
                return FilterByStatus(CertificateStatus.Granted);
            }
        }
    }
}
