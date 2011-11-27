using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.IO.Compression;
using System.Xml;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{

    /// <summary>
    /// The static list of the certificates
    /// </summary>
    public static class StaticCertificates
    {
        private static readonly Dictionary<string, StaticCertificateClass> m_classesByName = new Dictionary<string, StaticCertificateClass>();
        private static readonly Dictionary<long, StaticCertificate> m_certificatesByID = new Dictionary<long, StaticCertificate>();
        private static readonly List<StaticCertificateCategory> m_categories = new List<StaticCertificateCategory>();

        /// <summary>
        /// Gets the categories, sorted by name
        /// </summary>
        public static IEnumerable<StaticCertificateCategory> Categories
        {
            get 
            {
                return m_categories; 
            }
        }

        /// <summary>
        /// Gets the certificate classes, hierarchically sorted (category's name, class's name)
        /// </summary>
        public static IEnumerable<StaticCertificateClass> AllClasses
        {
            get
            {
                foreach (var category in m_categories)
                {
                    foreach (var certClass in category)
                    {
                        yield return certClass;
                    }
                }
            }
        }

        /// <summary>
        /// Gets the certificates, hierarchically sorted (category's name, class's name, grade)
        /// </summary>
        public static IEnumerable<StaticCertificate> AllCertificates
        {
            get 
            {
                foreach (var category in m_categories)
                {
                    foreach (var certClass in category)
                    {
                        foreach (var cert in certClass)
                        {
                            yield return cert;
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Gets the certificate with the specified ID
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public static StaticCertificate GetCertificate(int id)
        {
            return m_certificatesByID[id];
        }

        /// <summary>
        /// Gets the certificates class with the specified name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static StaticCertificateClass GetCertificateClass(string name)
        {
            return m_classesByName[name];
        }

        /// <summary>
        /// Initialize static certificates
        /// </summary>
        internal static void Load()
        {
            var datafile = Util.DeserializeDatafile<CertificatesDatafile>(DatafileConstants.CertificatesDatafile);

            foreach (var srcCat in datafile.Categories)
            {
                m_categories.Add(new StaticCertificateCategory(srcCat));
            }

            // Sort categories by name
            m_categories.Sort((c1, c2) => String.CompareOrdinal(c1.Name, c2.Name));

            // Build inner collections
            foreach(var certCategory in m_categories)
            {
                foreach(var certClass in certCategory)
                {
                    m_classesByName[certClass.Name] = certClass;
                    foreach(var cert in certClass)
                    {
                        m_certificatesByID[cert.ID] = cert;
                    }
                }
            }

            // Completes intialization
            foreach (var srcCat in datafile.Categories)
            {
                foreach (var srcClass in srcCat.Classes)
                {
                    var certClass = m_classesByName[srcClass.Name];
                    foreach (var srcCert in srcClass.Certificates)
                    {
                        certClass[srcCert.Grade].CompleteInitialization(srcCert.Prerequisites);
                    }
                }
            }
        }
    }
}