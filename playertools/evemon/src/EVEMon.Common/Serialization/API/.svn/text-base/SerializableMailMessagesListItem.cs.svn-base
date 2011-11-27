using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    public sealed class SerializableMailMessagesListItem
    {
        public SerializableMailMessagesListItem()
        {
            ToCharacterIDs = new List<string>();
            ToListID = new List<string>();
        }

        [XmlAttribute("messageID")]
        public long MessageID
        {
            get;
            set;
        }

        [XmlAttribute("senderID")]
        public long SenderID
        {
            get;
            set;
        }

        [XmlAttribute("sentDate")]
        public string SentDateXml
        {
            get { return SentDate.DateTimeToTimeString(); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    SentDate = value.TimeStringToDateTime();
            }
        }

        [XmlAttribute("title")]
        public string Title
        {
            get;
            set;
        }

        [XmlAttribute("toCorpOrAllianceID")]
        public string ToCorpOrAllianceID
        {
            get;
            set;
        }

        [XmlAttribute("toCharacterIDs")]
        public string ToCharacterIDsXml
        {
            get { return string.Join(",", ToCharacterIDs); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    ToCharacterIDs = value.Split(',').ToList();
            }
        }

        [XmlAttribute("toListID")]
        public string ToListIDXml
        {
            get { return string.Join(",", ToListID); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    ToListID = value.Split(',').ToList();
            }
        }

        [XmlIgnore]
        public DateTime SentDate
        {
            get;
            set;
        }

        [XmlIgnore]
        public List<string> ToCharacterIDs
        {
            get;
            set;
        }

        [XmlIgnore]
        public List<string> ToListID
        {
            get;
            set;
        }
    }
}
