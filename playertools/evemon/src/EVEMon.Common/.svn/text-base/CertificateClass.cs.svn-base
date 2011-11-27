using System;
using System.Collections.Generic;
using EVEMon.Common.Attributes;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a certificate class from a character's point of view.
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class CertificateClass : ReadonlyVirtualCollection<Certificate>
    {
        private readonly Character m_character;
        private readonly StaticCertificateClass m_staticData;
        private readonly Certificate[] m_items = new Certificate[4];
        private readonly CertificateCategory m_category;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="character"></param>
        /// <param name="src"></param>
        internal CertificateClass(Character character, StaticCertificateClass src, CertificateCategory category)
        {
            m_character = character;
            m_category = category;
            m_staticData = src;

            foreach (var srcCert in src)
            {
                var cert = new Certificate(character, srcCert, this);
                m_items[(int)cert.Grade] = cert;
            }
        }

        /// <summary>
        /// Core implementation of the <see cref="ReadonlyVirtualCollection{T}"/> collection.
        /// </summary>
        /// <returns></returns>
        protected override IEnumerable<Certificate> Enumerate()
        {
 	        foreach(var cert in m_items)
            {
                if (cert != null) yield return cert;
            }
        }

        /// <summary>
        /// Gets the static data associated with this object.
        /// </summary>
        public StaticCertificateClass StaticData
        {
            get { return m_staticData; }
        }

        /// <summary>
        /// Gets the category for this certificate class
        /// </summary>
        public CertificateCategory Category
        {
            get { return m_category; }
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

        /// <summary>
        /// Gets a certificate from this class by its grade. May be null if there is no such grade for this class.
        /// </summary>
        /// <param name="grade"></param>
        /// <returns></returns>
        public Certificate this[CertificateGrade grade]
        {
            get { return m_items[(int)grade]; }
        }

        /// <summary>
        /// Gets the lowest grade certificate.
        /// </summary>
        public Certificate LowestGradeCertificate
        {
            get
            {
                var scert = m_staticData.LowestGradeCertificate;
                return m_items[(int)scert.Grade];
            }
        }


        /// <summary>
        /// Gets the highest grade certificate.
        /// </summary>
        public Certificate HighestGradeCertificate
        {
            get
            {
                var scert = m_staticData.HighestGradeCertificate;
                return m_items[(int)scert.Grade];
            }
        }

        /// <summary>
        /// Gets the lowest untrained (neither granted nor claimable). Null if all certificates have been granted or are claimable.
        /// </summary>
        public Certificate LowestUntrainedGrade
        {
            get
            {
                foreach (var cert in m_items)
                {
                    if (cert != null)
                    {
                        if (cert.Status != CertificateStatus.Claimable && cert.Status != CertificateStatus.Granted) return cert;
                    }
                }
                return null;
            }
        }

        /// <summary>
        /// Gets the highest claimed grade. May be null if no grade has been granted.
        /// </summary>
        public Certificate HighestClaimedGrade
        {
            get
            {
                Certificate lastCert = null;
                foreach (var cert in m_items)
                {
                    if (cert != null)
                    {
                        if (cert.Status != CertificateStatus.Granted) return lastCert;
                        lastCert = cert;
                    }
                }
                return lastCert;
            }
        }

        /// <summary>
        /// Gets true if the provided character has completed this class
        /// </summary>
        /// <param name="character"></param>
        /// <returns></returns>
        public bool IsCompleted
        {
            get
            {
                foreach (var cert in m_items)
                {
                    if (cert != null && cert.Status != CertificateStatus.Granted) return false;
                }
                return true;
            }
        }

        /// <summary>
        /// Gets true if the provided character can train to the next grade, false if the class has already been completed or if the next grade is untrainable.
        /// </summary>
        /// <param name="character"></param>
        /// <returns></returns>
        public bool IsFurtherTrainable
        {
            get
            {
                foreach (var cert in m_items)
                {
                    if (cert != null)
                    {
                        if (cert.Status == CertificateStatus.PartiallyTrained) return true;
                        else if (cert.Status == CertificateStatus.Untrained) return false;
                    }
                }
                return false;
            }
        }

        /// <summary>
        /// Gets the name of the class.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return m_staticData.Name;
        }
    }
}
