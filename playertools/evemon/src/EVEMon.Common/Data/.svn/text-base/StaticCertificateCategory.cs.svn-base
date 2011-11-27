using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a certificate category. Every category (i.e. "Business and Industry") contains certificate classes (i.e. "Production Manager"), which contain certificates (i.e. "Production Manager Basic").
    /// </summary>
    public sealed class StaticCertificateCategory : ReadonlyCollection<StaticCertificateClass>
    {
        private readonly long m_id;
        private readonly string m_name;
        private readonly string m_description;

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="element"></param>
        internal StaticCertificateCategory(SerializableCertificateCategory src)
            : base(src.Classes.Length)
        {
            this.m_id = src.ID;
            this.m_name = src.Name;
            this.m_description = src.Description;

            foreach (var srcClass in src.Classes)
            {
                m_items.Add(new StaticCertificateClass(this, srcClass));
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
        /// Gets a string representation of this category
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return this.m_name;
        }
    }
}
