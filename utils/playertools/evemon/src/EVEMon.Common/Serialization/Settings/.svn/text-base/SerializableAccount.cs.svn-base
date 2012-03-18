using System;
using System.Linq;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common.Serialization.Settings
{
    public sealed class SerializableAccount
    {
        public SerializableAccount()
        {
            IgnoreList = new List<SerializableCharacterIdentity>();
        }

        [XmlAttribute("id")]
        public long ID
        {
            get;
            set;
        }

        [XmlAttribute("key")]
        public string Key
        {
            get;
            set;
        }

        [XmlAttribute("keyLevel")]
        public CredentialsLevel KeyLevel
        {
            get;
            set;
        }

        [XmlElement("paidUntil")]
        public DateTime PaidUntil
        {
            get;
            set;
        }

        [XmlElement("createDate")]
        public DateTime CreateDate
        {
            get;
            set;
        }

        [XmlElement("lastAccountStatusUpdate")]
        public DateTime LastAccountStatusUpdate
        {
            get;
            set;
        }

        [XmlElement("lastCharacterListUpdate")]
        public DateTime LastCharacterListUpdate
        {
            get;
            set;
        }

        [XmlElement("ignored")]
        public List<SerializableCharacterIdentity> IgnoreList
        {
            get;
            set;
        }

        internal object Clone()
        {
            var clone = new SerializableAccount();
            clone.ID = this.ID;
            clone.Key = this.Key;
            clone.KeyLevel = this.KeyLevel;
            clone.PaidUntil = this.PaidUntil;
            clone.CreateDate = this.CreateDate;
            clone.LastAccountStatusUpdate = this.LastAccountStatusUpdate;
            clone.LastCharacterListUpdate = this.LastCharacterListUpdate;
            clone.IgnoreList.AddRange(this.IgnoreList.Select(x => x.Clone()));
            return clone;
        }
    }
}
