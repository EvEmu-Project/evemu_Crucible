using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Configuration settings for the tray icon popup window
    /// </summary>
    public sealed class TrayPopupSettings
    {
        public TrayPopupSettings()
        {
            PortraitSize = PortraitSizes.x48;
            HighlightConflicts = true;
            ShowSkillQueueTrainingTime = true;
            ShowSkillInTraining = true;
            ShowRemainingTime = true;
            ShowPortrait = true;
            ShowServerStatus = true;
            ShowEveTime = true;
            ShowWarning = true;
            IndentGroupedAccounts = true;
        }


        [XmlElement("style")]
        public TrayPopupStyles Style
        {
            get;
            set;
        }

        [XmlElement("groupBy")]
        public TrayPopupGrouping GroupBy
        {
            get;
            set;
        }

        [XmlElement("primarySortOrder")]
        public TrayPopupSort PrimarySortOrder
        {
            get;
            set;
        }

        [XmlElement("secondarySortOrder")]
        public TrayPopupSort SecondarySortOrder
        {
            get;
            set;
        }

        [XmlElement("portraitSize")]
        public PortraitSizes PortraitSize
        {
            get;
            set;
        }

        [XmlElement("showCharNotTraining")]
        public bool ShowCharNotTraining
        {
            get;
            set;
        }

        [XmlElement("showSkillInTraining")]
        public bool ShowSkillInTraining
        {
            get;
            set;
        }

        [XmlElement("showRemainingTime")]
        public bool ShowRemainingTime
        {
            get;
            set;
        }

        [XmlElement("showTimeToCompletion")]
        public bool ShowCompletionTime
        {
            get;
            set;
        }

        [XmlElement("showWallet")]
        public bool ShowWallet
        {
            get;
            set;
        }

        [XmlElement("showPortrait")]
        public bool ShowPortrait
        {
            get;
            set;
        }

        [XmlElement("showSkillQueueTrainingTime")]
        public bool ShowSkillQueueTrainingTime
        {
            get;
            set;
        }

        [XmlElement("showWarning")]
        public bool ShowWarning
        {
            get;
            set;
        }

        [XmlElement("showServerStatus")]
        public bool ShowServerStatus
        {
            get;
            set;
        }

        [XmlElement("showEveTime")]
        public bool ShowEveTime
        {
            get;
            set;
        }

        [XmlElement("highlightConflicts")]
        public bool HighlightConflicts
        {
            get;
            set;
        }

		[XmlElement("indentGroupedAccounts")]
		public bool IndentGroupedAccounts
		{
			get;
			set;
		}

        [XmlElement("useIncreasedContrast")]
        public bool UseIncreasedContrast
        {
            get;
            set;
        }

        public TrayPopupSettings Clone()
        {
            return this.MemberwiseClone() as TrayPopupSettings;
        }
    }

    public enum TrayPopupGrouping
    {
        None = 0,
        TrainingAtTop = 1,
        TrainingAtBottom = 2,
        Account = 3
    }

    public enum TrayPopupSort
    {
        TrainingCompletionTimeDESC = 0,
        TrainingCompletionTimeASC = 1,
        NameASC = 2,
        NameDESC = 3
    }

    public enum PortraitSizes
    {
        x16 = 0,
        x24 = 1,
        x32 = 2,
        x48 = 3,
        x64 = 4,
        x80 = 5,
        x96 = 6
    }

    public enum TrayPopupStyles
    {
        PopupForm = 0,
        WindowsTooltip = 1,
        Disabled = 2
    }
}