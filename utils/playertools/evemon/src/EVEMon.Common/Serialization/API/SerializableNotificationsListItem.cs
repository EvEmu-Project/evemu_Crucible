using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    public sealed class SerializableNotificationsListItem
    {
        [XmlAttribute("notificationID")]
        public long NotificationID
        {
            get;
            set;
        }

        [XmlAttribute("typeID")]
        public int TypeID
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

        [XmlAttribute("read")]
        public bool Read
        {
            get;
            set;
        }

        [XmlIgnore]
        public DateTime SentDate
        {
            get;
            set;
        }
    }
}
