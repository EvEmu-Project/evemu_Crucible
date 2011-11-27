using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;
using EVEMon.Common.Attributes;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Represents the settings for a character.
    /// </summary>
    public sealed class CharacterUISettings
    {
        public CharacterUISettings()
        {
            CollapsedGroups = new List<String>();
            FullAPIKeyEnabledPages = new List<String>();
            SelectedPage = String.Empty;
        }

        [XmlElement("selectedPage")]
        public string SelectedPage
        {
            get;
            set;
        }

        [XmlElement("collapsedGroup")]
        public List<String> CollapsedGroups
        {
            get;
            set;
        }

        [XmlElement("fullAPIKeyEnabledPage")]
        public List<String> FullAPIKeyEnabledPages
        {
            get;
            set;
        }

        [XmlElement("ordersGroupBy")]
        public MarketOrderGrouping OrdersGroupBy
        {
            get;
            set;
        }

        [XmlElement("jobsGroupBy")]
        public IndustryJobGrouping JobsGroupBy
        {
            get;
            set;
        }

        [XmlElement("mailMessagesGroupBy")]
        public EVEMailMessagesGrouping EVEMailMessagesGroupBy
        {
            get;
            set;
        }

        [XmlElement("eveNotificationsGroupBy")]
        public EVENotificationsGrouping EVENotificationsGroupBy
        {
            get;
            set;
        }

        /// <summary>
        /// Clones this serialization object.
        /// </summary>
        /// <returns></returns>
        internal CharacterUISettings Clone()
        {
            return (CharacterUISettings)MemberwiseClone();
        }
    }
}
