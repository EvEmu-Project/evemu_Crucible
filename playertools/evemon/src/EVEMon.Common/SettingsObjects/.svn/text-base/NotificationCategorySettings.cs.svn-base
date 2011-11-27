using System;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Category settings for notifications
    /// </summary>
    public sealed class NotificationCategorySettings
    {
        public NotificationCategorySettings()
        {
            ToolTipBehaviour = ToolTipNotificationBehaviour.Once;
            ShowOnMainWindow = true;
        }

        public NotificationCategorySettings(ToolTipNotificationBehaviour toolTipBehaviour)
        {
            ToolTipBehaviour = toolTipBehaviour;
            ShowOnMainWindow = true;
        }

        [XmlAttribute("toolTipBehaviour")]
        public ToolTipNotificationBehaviour ToolTipBehaviour
        {
            get;
            set;
        }

        [XmlAttribute("showOnMainWindow")]
        public bool ShowOnMainWindow
        {
            get;
            set;
        }

        public NotificationCategorySettings Clone()
        {
            return (NotificationCategorySettings)MemberwiseClone();
        }
    }
}
