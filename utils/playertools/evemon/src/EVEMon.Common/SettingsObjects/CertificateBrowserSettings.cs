using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class CertificateBrowserSettings
    {
        [XmlElement("textSearch")]
        public string TextSearch
        {
            get;
            set;
        }

        [XmlElement("filter")]
        public CertificateFilter Filter
        {
            get;
            set;
        }

        [XmlElement("sort")]
        public CertificateSort Sort
        {
            get;
            set;
        }

        internal CertificateBrowserSettings Clone()
        {
            return (CertificateBrowserSettings)MemberwiseClone();
        }
    }

    public enum CertificateFilter
    {
        All = 0,
        Claimable = 1,
        HideElite = 2,
        NextGradeTrainable = 3,
        NextGradeUntrainable = 4
    }

    public enum CertificateSort
    {
        Name = 0,
        TimeToNextGrade = 1,
        TimeToEliteGrade = 2
    }
}
