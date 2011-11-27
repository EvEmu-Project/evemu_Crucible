using System;
using System.Collections.Generic;
using System.Xml.Serialization;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// This class is a temporary representation of the <see cref="Settings"/> class for serialization purposes through automatic serialization
    /// </summary>
    [XmlRoot("Settings")]
    public sealed partial class SerializableSettings
    {
        public SerializableSettings()
        {
            Plans = new List<SerializablePlan>();
            Accounts = new List<SerializableAccount>();
            Characters = new List<SerializableSettingsCharacter>();
            MonitoredCharacters = new List<MonitoredCharacterSettings>();

            APIProviders = new SerializableAPIProviders();
            Notifications = new NotificationSettings();
            Exportation = new ExportationSettings();
            Scheduler = new SerializableScheduler();
            Calendar = new CalendarSettings();
            Updates = new UpdateSettings();
            Proxy = new ProxySettings();
            IGB = new IGBSettings();
            G15 = new G15Settings();
            UI = new UISettings();
        }

        [XmlAttribute("revision")]
        public int Revision
        {
            get;
            set;
        }

        [XmlElement("compatibility")]
        public CompatibilityMode Compatibility
        {
            get;
            set;
        }

        [XmlArray("accounts")]
        [XmlArrayItem("account")]
        public List<SerializableAccount> Accounts
        {
            get;
            set;
        }

        [XmlArray("characters")]
        [XmlArrayItem("ccp", typeof(SerializableCCPCharacter))]
        [XmlArrayItem("uri", typeof(SerializableUriCharacter))]
        public List<SerializableSettingsCharacter> Characters
        {
            get;
            set;
        }

        [XmlArray("plans")]
        [XmlArrayItem("plan")]
        public List<SerializablePlan> Plans
        {
            get;
            set;
        }

        [XmlArray("monitoredCharacters")]
        [XmlArrayItem("character")]
        public List<MonitoredCharacterSettings> MonitoredCharacters
        {
            get;
            set;
        }

        [XmlElement("apiProviders")]
        public SerializableAPIProviders APIProviders
        {
            get;
            set;
        }

        [XmlElement("updates")]
        public UpdateSettings Updates
        {
            get;
            set;
        }

        [XmlElement("notifications")]
        public NotificationSettings Notifications
        {
            get;
            set;
        }

        [XmlElement("network")]
        public IGBSettings IGB
        {
            get;
            set;
        }

        [XmlElement("scheduler")]
        public SerializableScheduler Scheduler
        {
            get;
            set;
        }

        [XmlElement("calendar")]
        public CalendarSettings Calendar
        {
            get;
            set;
        }

        [XmlElement("exportation")]
        public ExportationSettings Exportation
        {
            get;
            set;
        }

        [XmlElement("G15")]
        public G15Settings G15
        {
            get;
            set;
        }

        [XmlElement("UI")]
        public UISettings UI
        {
            get;
            set;
        }

        [XmlElement("proxy")]
        public ProxySettings Proxy
        {
            get;
            set;
        }
    }
}
