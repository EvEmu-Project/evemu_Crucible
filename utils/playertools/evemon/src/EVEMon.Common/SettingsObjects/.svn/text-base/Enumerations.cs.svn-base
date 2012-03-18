using EVEMon.Common.Attributes;

namespace EVEMon.Common.SettingsObjects
{
    #region UISettings

    /// <summary>
    /// Describes the target platform to allow EVEMon to apply different tweaks at runtime
    /// </summary>
    public enum CompatibilityMode
    {
        /// <summary>
        /// Windows and Linux + Wine
        /// </summary>
        Default = 0,
        /// <summary>
        /// Wine
        /// </summary>
        Wine = 1
    }

    /// <summary>
    /// Describes the behaviour employed to remove Obsolete Entries from plans.
    /// </summary>
    public enum ObsoleteEntryRemovalBehaviour
    {
        /// <summary>
        /// Never remove entries from the plan, always ask the user.
        /// </summary>
        AlwaysAsk = 0,
        /// <summary>
        /// Only remove confirmed completed (by API) entries from the plan, ask about unconfirmed entries.
        /// </summary>
        RemoveConfirmed = 1,
        /// <summary>
        /// Always remove all entries automatically.
        /// </summary>
        RemoveAll = 2
    }

    /// <summary>
    /// Represents the behaviour for the system tray icon
    /// </summary>
    public enum SystemTrayBehaviour
    {
        /// <summary>
        /// The tray icon is always hidden
        /// </summary>
        Disabled = 0,
        /// <summary>
        /// The tray icon is visible when the main window is minimized
        /// </summary>
        ShowWhenMinimized = 1,
        /// <summary>
        /// The tray icon is always visible
        /// </summary>
        AlwaysVisible = 2
    }

    /// <summary>
    /// Represents the behaviour when closing the main form
    /// </summary>
    public enum CloseBehaviour
    {
        /// <summary>
        /// Exit the application
        /// </summary>
        Exit = 0,
        /// <summary>
        /// Minimize to the system tray
        /// </summary>
        MinimizeToTray = 1,
        /// <summary>
        /// Minimize to the task bar
        /// </summary>
        MinimizeToTaskbar = 2
    }

    #endregion


    #region MarketOrderSettings

    /// <summary>
    /// Enumeration for the market orders to be group by.
    /// </summary>
    /// <remarks>The integer value determines the sort order.</remarks>
    public enum MarketOrderGrouping
    {
        [Header("Group by order status")]
        State = 0,
        [Header("Group by order status (Desc)")]
        StateDesc = 1,
        [Header("Group by buying/selling")]
        OrderType = 2,
        [Header("Group by buying/selling (Desc)")]
        OrderTypeDesc = 3,
        [Header("Group by issue day")]
        Issued = 4,
        [Header("Group by issue day (Desc)")]
        IssuedDesc = 5,
        [Header("Group by item type")]
        ItemType = 6,
        [Header("Group by item type (Desc)")]
        ItemTypeDesc = 7,
        [Header("Group by station")]
        Location = 8,
        [Header("Group by station (Desc)")]
        LocationDesc = 9
    }
    
    #endregion


    #region IndustryJobSettings

    /// <summary>
    /// Enumeration for the industry jobs to be group by.
    /// </summary>
    /// <remarks>The integer value determines the sort order.</remarks>
    public enum IndustryJobGrouping
    {
        [Header("Group by job state")]
        State = 0,
        [Header("Group by job state (Desc)")]
        StateDesc = 1,
        [Header("Group by ending date")]
        EndDate = 2,
        [Header("Group by ending date (Desc)")]
        EndDateDesc = 3,
        [Header("Group by installed blueprint type")]
        InstalledItemType = 4,
        [Header("Group by installed blueprint type (Desc)")]
        InstalledItemTypeDesc = 5,
        [Header("Group by output item type")]
        OutputItemType = 6,
        [Header("Group by output item type (Desc)")]
        OutputItemTypeDesc = 7,
        [Header("Group by job activity")]
        Activity = 8,
        [Header("Group by job activity (Desc)")]
        ActivityDesc = 9,
        [Header("Group by installed location")]
        Location = 10,
        [Header("Group by installed location (Desc)")]
        LocationDesc = 11
    }

    #endregion


    #region EVEMailMessagesSettings

    /// <summary>
    /// Enumeration for the EVE mail messages to be group by.
    /// </summary>
    /// <remarks>The integer value determines the sort order.</remarks>
    public enum EVEMailMessagesGrouping
    {
        [Header("Group by mail state")]
        State = 0,
        [Header("Group by mail state (Desc)")]
        StateDesc = 1,
        [Header("Group by received date")]
        SentDate = 2,
        [Header("Group by received date (Desc)")]
        SentDateDesc = 3,
        [Header("Group by sender")]
        Sender = 4,
        [Header("Group by sender (Desc)")]
        SenderDesc = 5,
        [Header("Group by subject")]
        Subject = 6,
        [Header("Group by subject (Desc)")]
        SubjectDesc = 7,
        [Header("Group by recipient")]
        Recipient = 8,
        [Header("Group by recipient (Desc)")]
        RecipientDesc = 9,
        [Header("Group by Corp or Alliance")]
        CorpOrAlliance = 10,
        [Header("Group by Corp or Alliance (Desc)")]
        CorpOrAllianceDesc = 11,
        [Header("Group by mailing list")]
        MailingList = 12,
        [Header("Group by mailing list (Desc)")]
        MailingListDesc = 13
    }

    #endregion


    #region EVENotificationsSettings

    /// <summary>
    /// Enumeration for the EVE notifications to be group by.
    /// </summary>
    /// <remarks>The integer value determines the sort order.</remarks>
    public enum EVENotificationsGrouping
    {
        [Header("Group by type")]
        Type = 0,
        [Header("Group by type (Desc)")]
        TypeDesc = 1,
        [Header("Group by received date")]
        SentDate = 2,
        [Header("Group by received date (Desc)")]
        SentDateDesc = 3,
        [Header("Group by sender")]
        Sender = 4,
        [Header("Group by sender (Desc)")]
        SenderDesc = 5,
    }

    #endregion


    #region ReadingPanePositioning

    /// <summary>
    /// Enumeration for the reading pane position of the EVE mail bodies.
    /// </summary>
    public enum ReadingPanePositioning
    {
        /// <summary>
        /// No reading pane
        /// </summary>
        Off = 0,
        /// <summary>
        /// Position the reading pane positioning at the bottom
        /// </summary>
        Bottom = 1,
        /// <summary>
        /// Position the reading pane positioning at the right
        /// </summary>
        Right = 2,
    }

    #endregion


    #region Full API Key Features

    public enum FullAPIKeyFeatures
    {
        [Header ("Market")]
        MarketOrders,
        [Header("Industry")]
        IndustryJobs,
        [Header("Research")]
        ResearchPoints,
        [Header("Mail")]
        MailMessages,
        [Header("Notification")]
        Notifications,
    }

    #endregion


    #region EVE API Update Period

    public enum UpdatePeriod
    {
        [Header("Never")]
        Never,
        [Header("5 Minutes")]
        Minutes5,
        [Header("15 Minutes")]
        Minutes15,
        [Header("30 Minutes")]
        Minutes30,
        [Header("1 Hour")]
        Hours1,
        [Header("2 Hours")]
        Hours2,
        [Header("3 Hours")]
        Hours3,
        [Header("6 Hours")]
        Hours6,
        [Header("12 Hours")]
        Hours12,
        [Header("Day")]
        Day,
        [Header("Week")]
        Week
    }
    
    #endregion


    #region EVE API Cache Style

    public enum CacheStyle
    {
        /// <summary>
        /// Short cache style, data will always be returned from CCP,
        /// however it will only be updated once the cache timer
        /// expires.
        /// </summary>
        [Header("Short")]
        Short,
        /// <summary>
        /// Long cache style, data will only be returned from CCP after
        /// the cahce timer has expired.
        /// </summary>
        [Header("Long")]
        Long
    }
    
    #endregion
}
