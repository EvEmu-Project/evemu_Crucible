using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a certificate class. Every category (i.e. "Business and Industry") contains certificate classes (i.e. "Production Manager"), which contain certificates (i.e. "Production Manager Basic").
    /// </summary>
    public sealed class StaticCertificateClass : ReadonlyVirtualCollection<StaticCertificate>
    {
        private readonly long m_id;
        private readonly string m_name;
        private readonly string m_description;
        private readonly StaticCertificateCategory m_category;
        private readonly StaticCertificate[] m_certificates = new StaticCertificate[4];

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="category"></param>
        /// <param name="element"></param>
        internal StaticCertificateClass(StaticCertificateCategory category, SerializableCertificateClass src)
        {
            this.m_id = src.ID;
            this.m_name = src.Name;
            this.m_description = src.Description;
            this.m_category = category;

            foreach (var srcCert in src.Certificates)
            {
                var cert = new StaticCertificate(this, srcCert);
                this.m_certificates[(int)cert.m_grade] = cert;
            }
        }

        /// <summary>
        /// Gets this category's id
        /// </summary>
        public long ID
        {
            get { return m_id; }
        }

        /// <summary>
        /// Gets this category's name
        /// </summary>
        public string Name
        {
            get { return m_name; }
        }

        /// <summary>
        /// Gets this category's description
        /// </summary>
        public string Description
        {
            get { return m_description; }
        }

        /// <summary>
        /// Gets the certificates class's category.
        /// </summary>
        public StaticCertificateCategory Category
        {
            get { return m_category; }
        }

        /// <summary>
        /// Gets the certificate with the specified grade
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public StaticCertificate this[CertificateGrade grade]
        {
            get { return this.m_certificates[(int)grade]; }
        }

        /// <summary>
        /// Gets the lowest grade certificate.
        /// </summary>
        public StaticCertificate LowestGradeCertificate
        {
            get
            {
                foreach (var cert in m_certificates)
                {
                    if (cert != null) return cert;
                }
                throw new NotImplementedException();
            }
        }


        /// <summary>
        /// Gets the highest grade certificate.
        /// </summary>
        public StaticCertificate HighestGradeCertificate
        {
            get
            {
                // Look for the next grade
                StaticCertificate lastCert = null;
                foreach (var cert in m_certificates)
                {
                    if (cert != null) lastCert = cert;
                }
                return lastCert;
            }
        }

        /// <summary>
        /// Enumerates over the items in this collection
        /// </summary>
        /// <returns></returns>
        protected override IEnumerable<StaticCertificate> Enumerate()
        {
            foreach (var cert in m_certificates)
            {
                if (cert != null) yield return cert;
            }
        }

        /// <summary>
        /// Gets a string representation of this class
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return this.m_name;
        }
    }
}
