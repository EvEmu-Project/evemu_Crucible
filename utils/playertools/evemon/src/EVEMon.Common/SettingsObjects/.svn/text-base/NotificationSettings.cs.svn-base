using System;
using System.Xml.Serialization;
using EVEMon.Common.Notifications;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class NotificationSettings
    {
        public NotificationSettings()
        {
            Categories = new SerializableDictionary<NotificationCategory, NotificationCategorySettings>();
            Categories[NotificationCategory.AccountNotInTraining] = 
                new NotificationCategorySettings(ToolTipNotificationBehaviour.RepeatUntilClicked);
            EmailPortNumber = 25;
        }

        [XmlElement("categories")]
        public SerializableDictionary<NotificationCategory, NotificationCategorySettings> Categories
        {
            get;
            set;
        }

        [XmlElement("playSoundOnSkillCompletion")]
        public bool PlaySoundOnSkillCompletion
        {
            get;
            set;
        }

        [XmlElement("sendMailAlert")]
        public bool SendMailAlert
        {
            get;
            set;
        }

        [XmlElement("useEmailShortFormat")]
        public bool UseEmailShortFormat
        {
            get;
            set;
        }

        [XmlElement("emailFromAddress")]
        public string EmailFromAddress
        {
            get;
            set;
        }

        [XmlElement("emailToAddress")]
        public string EmailToAddress
        {
            get;
            set;
        }

        [XmlElement("emailSmtpServer")]
        public string EmailSmtpServer
        {
            get;
            set;
        }

        [XmlElement("emailPortNumber")]
        public int EmailPortNumber
        {
            get;
            set;
        }

        [XmlElement("emailAuthenticationRequired")]
        public bool EmailAuthenticationRequired
        {
            get;
            set;
        }

        [XmlElement("emailAuthenticationUserName")]
        public string EmailAuthenticationUserName
        {
            get;
            set;
        }

        [XmlElement("emailAuthenticationPassword")]
        public string EmailAuthenticationPassword
        {
            get;
            set;
        }

        [XmlElement("emailServerRequiresSSL")]
        public bool EmailServerRequiresSSL
        {
            get;
            set;
        }

        internal NotificationSettings Clone()
        {
            var clone = (NotificationSettings)MemberwiseClone();

            // Add copy of behaviors
            clone.Categories = new SerializableDictionary<NotificationCategory, NotificationCategorySettings>();
            foreach (var pair in Categories) 
            {
                clone.Categories[pair.Key] = pair.Value.Clone();
            }

            return clone;
        }
    }
}
