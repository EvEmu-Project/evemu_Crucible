using System;
using System.Xml.Serialization;
using EVEMon.Common.Attributes;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Settings for the updates from CCP and others
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class UpdateSettings
    {
        private string m_updatesUrl;
        private int m_updateFrequency;

        public UpdateSettings()
        {
            CheckTimeOnStartup = true;
            CheckEVEMonVersion = true;
            HttpTimeout = 20;
            Periods = new SerializableDictionary<APIMethods, UpdatePeriod>();
            IgnoreNetworkStatus = false;
            UpdateFrequency = 240;
            UseCustomUpdatesUrl = false;
            UpdatesUrl = NetworkConstants.BattleclinicUpdates;
        }

        /// <summary>
        /// When true, EVEMon will check its version from BattleClinic
        /// </summary>
        [XmlElement("checkEVEMonVersion")]
        public bool CheckEVEMonVersion
        {
            get;
            set;
        }

        /// <summary>
        /// When true, EVEMon will check its version from BattleClinic
        /// </summary>
        [XmlElement("checkTimeOnStartup")]
        public bool CheckTimeOnStartup
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the latest upgrade version the user choose to reject.
        /// </summary>
        [XmlElement("mostRecentDeniedUpdgrade")]
        public string MostRecentDeniedUpgrade
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the length of time between updates in minutes.
        /// </summary>
        /// <remarks>
        /// Hidden Setting. The value of this setting must be equal to or higher than 240 minutes, the default is 240 minutes (4 hours).
        /// </remarks>
        [XmlElement("updateFrequency")]
        public int UpdateFrequency
        {
            get
            {
                if (m_updateFrequency < 240)
                    return 240;

                return m_updateFrequency;
            }
            set
            {
                m_updateFrequency = value;
            }
        }

        [XmlElement("useCustomUpdatesUrl")]
        public bool UseCustomUpdatesUrl { get; set; }

        /// <summary>
        /// Url to patch.xml
        /// </summary>
        /// <remarks>
        /// Hidden Setting.
        /// </remarks>
        [XmlElement("updatesUrl")]
        public string UpdatesUrl
        {
            get
            {
                if (!UseCustomUpdatesUrl)
                    return NetworkConstants.BattleclinicUpdates;

                if (String.IsNullOrEmpty(m_updatesUrl))
                    return NetworkConstants.BattleclinicUpdates;

                // We don't want this to be abused, so we lock the custom update url to localhost.
                // For convenience any localhost path can be used on any port. file:// does not work anyway.
                if (!m_updatesUrl.StartsWith("http://localhost:"))
                    return NetworkConstants.BattleclinicUpdates;
                
                return m_updatesUrl;
            }
            set
            {
                m_updatesUrl = value;
            }
        }

        [XmlElement("periods")]
        public SerializableDictionary<APIMethods, UpdatePeriod> Periods
        {
            get;
            set;
        }

        [XmlElement("httpTimeout")]
        public int HttpTimeout
        {
            get;
            set;
        }

        /// <summary>
        /// Short circuit the check for network connectivity and try and connect anyway.
        /// </summary>
        /// <remarks>
        /// Hidden setting, no UI. Used for the hand full of people using Wine/Darwine with a broken .NET Network Stack.
        /// </remarks>
        [XmlElement("ignoreNetworkStatus")]
        public bool IgnoreNetworkStatus
        {
            get;
            set;
        }
        
        internal UpdateSettings Clone()
        {
            var clone = new UpdateSettings();
            clone.CheckEVEMonVersion = this.CheckEVEMonVersion;
            clone.CheckTimeOnStartup = this.CheckTimeOnStartup;
            clone.MostRecentDeniedUpgrade = this.MostRecentDeniedUpgrade;
            clone.HttpTimeout = this.HttpTimeout;
            clone.IgnoreNetworkStatus = this.IgnoreNetworkStatus;
            clone.UpdateFrequency = this.UpdateFrequency;
            clone.UseCustomUpdatesUrl = this.UseCustomUpdatesUrl;
            clone.UpdatesUrl = this.UpdatesUrl;

            foreach (var pair in this.Periods)
            {
                clone.Periods.Add(pair.Key, pair.Value);
            }
            return clone;
        }
    }
}
