using System;
using EVEMon.Common.Serialization.API;
using System.ComponentModel;

namespace EVEMon.Common
{
    public interface IQueryMonitor
    {
        /// <summary>
        /// Gets or sets a value indicating whether this <see cref="IQueryMonitor"/> is enabled.
        /// </summary>
        /// <value><c>true</c> if enabled; otherwise, <c>false</c>.</value>
        bool Enabled { get; set; }
        
        /// <summary>
        /// Gets the status of the query.
        /// </summary>
        QueryStatus Status { get; }

        /// <summary>
        /// Gets the API method monitored by this instance.
        /// </summary>
        APIMethods Method { get; }

        /// <summary>
        /// Gets the last time this instance was updated (UTC).
        /// </summary>
        DateTime LastUpdate { get; }

        /// <summary>
        /// Gets the next time this instance should be updated (UTC), based on both the CCP cache time and the user preferences.
        /// </summary>
        DateTime NextUpdate { get; }

        /// <summary>
        /// Gets true whether the method is curently being requeried.
        /// </summary>
        bool IsUpdating { get; }

        /// <summary>
        /// Gets the last API result.
        /// </summary>
        IAPIResult LastResult { get; }

        /// <summary>
        /// Gets true whether a full key is needed.
        /// </summary>
        bool IsFullKeyNeeded { get; }

        /// <summary>
        /// Gets true when a force update is within CCP cache time.
        /// </summary>
        bool ForceUpdateWillCauseError { get; }
    }

    /// <summary>
    /// Adds the internal methods for a query monitor.
    /// </summary>
    internal interface IQueryMonitorEx : IQueryMonitor
    {
        /// <summary>
        /// Resets the monitor with the given last update time.
        /// </summary>
        /// <param name="lastUpdate">The UTC time of the last update.</param>
        void Reset(DateTime lastUpdate);

        /// <summary>
        /// Updates on every second.
        /// </summary>
        void UpdateOnOneSecondTick();

        /// <summary>
        /// Forces an update.
        /// </summary>
        /// <param name="retryOnError">When true, the update will be reattempted until succesful.</param>
        void ForceUpdate(bool retryOnError);
    }

    /// <summary>
    /// 
    /// </summary>
    public enum QueryStatus
    {
        /// <summary>
        /// The query will be updated on due time.
        /// </summary>
        [Description("Pending...")]
        Pending,
        /// <summary>
        /// The query is being updated.
        /// </summary>
        [Description("Updating...")]
        Updating,
        /// <summary>
        /// The query is disabled by the settings.
        /// </summary>
        [Description("Disabled by settings.")]
        Disabled,
        /// <summary>
        /// There is no network connection.
        /// </summary>
        [Description("No TCP/IP connection.")]
        NoNetwork,
        /// <summary>
        /// A full key was needed.
        /// </summary>
        [Description("Full API Key required.")]
        NoFullKey,
        /// <summary>
        /// The character is not on any account.
        /// </summary>
        [Description("No associated account.")]
        NoAccount
    }
}
