using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.BattleClinic
{
    [XmlRoot("BattleClinicApi")]
    public sealed class BCAPIResult<T>
    {
        [XmlElement("cacheExpires")]
        public string CacheExpiresXml
        {
            get { return CacheExpires.DateTimeToTimeString(); }
            set
            {
                if (String.IsNullOrEmpty(value))
                    return;

                CacheExpires = value.TimeStringToDateTime();
            }
        }

        [XmlElement("result")]
        public T Result;

        [XmlElement("error")]
        public BCAPIError Error
        {
            get;
            set;
        }

        [XmlIgnore]
        public DateTime CacheExpires;

        [XmlIgnore]
        public bool HasError
        {
            get { return Error != null; }
        }
    }
}
