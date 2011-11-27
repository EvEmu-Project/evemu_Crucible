using System;
using EVEMon.Common.Attributes;
using EVEMon.Common.Net;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    /// <summary>
    /// This class monitors a querying process. It provides services for autoupdating, update notification, etc.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    [EnforceUIThreadAffinity]
    public class QueryMonitor<T> : IQueryMonitorEx, INetworkChangeSubscriber
    {
        public event QueryCallback<T> Updated;

        protected readonly APIMethods m_method;
        protected readonly bool m_isFullKeyNeeded;
        protected readonly string m_methodHeader;

        protected QueryStatus m_status;
        protected DateTime m_lastUpdate;
        protected APIResult<T> m_lastResult;
        protected CacheStyle m_cacheStyle;
        protected bool m_forceUpdate;
        protected bool m_retryOnForceUpdateError;
        protected bool m_isUpdating;
        protected bool m_isCanceled;
        protected bool m_enabled;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="method"></param>
        internal QueryMonitor(APIMethods method)
        {
            m_lastUpdate = DateTime.MinValue;
            m_isFullKeyNeeded = method.HasAttribute<FullKeyAttribute>();
            m_methodHeader = (method.HasHeader() ? method.GetHeader() : String.Empty);
            m_forceUpdate = true;
            m_method = method;
            m_enabled = true;

            bool methodHasAttribute = m_method.HasAttribute<UpdateAttribute>();
            m_cacheStyle = (methodHasAttribute ? m_method.GetAttribute<UpdateAttribute>().CacheStyle : CacheStyle.Short);

            NetworkMonitor.Register(this);
        }

        /// <summary>
        /// Gets true if the query is enabled.
        /// </summary>
        public bool Enabled
        {
            get { return m_enabled; }
            set { m_enabled = value; }
        }

        /// <summary>
        /// Gets the API method monitored by this instance.
        /// </summary>
        public APIMethods Method
        {
            get { return m_method; }
        }

        /// <summary>
        /// Gets the last time this instance was updated (UTC).
        /// </summary>
        public DateTime LastUpdate
        {
            get { return m_lastUpdate; }
        }

        /// <summary>
        /// Gets the status of the query.
        /// </summary>
        public QueryStatus Status
        {
            get { return m_status; }
        }

        /// <summary>
        /// Gets true when the API provider is CCP and a force update is triggered before the next available update.
        /// </summary>
        public bool ForceUpdateWillCauseError
        {
            get 
            {
                if (EveClient.APIProviders.CurrentProvider != APIProvider.DefaultProvider &&
                    EveClient.APIProviders.CurrentProvider != APIProvider.TestProvider)
                    return false;

                var cachedTime = (m_lastResult == null ? NextUpdate : m_lastResult.CachedUntil);

                return DateTime.UtcNow < cachedTime;
            }
        }

        /// <summary>
        /// Gets the next time this instance should be updated (UTC), based on both the CCP cache time and the user preferences.
        /// </summary>
        public DateTime NextUpdate
        {
            get
            {
                // If there was an error on last try, we use the cached time
                // (we exclude the corporation roles error for characters corporation issued queries)
                if (m_lastResult != null
                    && m_lastResult.HasError
                    && m_lastResult.CCPError != null
                    && !m_lastResult.CCPError.IsOrdersRelatedCorpRolesError
                    && !m_lastResult.CCPError.IsJobsRelatedCorpRolesError)
                    return m_lastResult.CachedUntil;

                // No error ? Then we compute the next update according to the settings.
                UpdatePeriod period = Settings.Updates.Periods[m_method];
                if (period == UpdatePeriod.Never)
                    return DateTime.MaxValue;

                DateTime nextUpdate = m_lastUpdate.Add(period.ToDuration());

                // If CCP "cached until" is greater than what we computed, return CCP cached time.
                if (m_lastResult != null && m_lastResult.CachedUntil > nextUpdate)
                    return m_lastResult.CachedUntil;

                return nextUpdate;
            }
        }

        /// <summary>
        /// Gets the last result queried from the API provider.
        /// </summary>
        public APIResult<T> LastResult
        {
            get { return m_lastResult; }
        }

        /// <summary>
        /// Gets true whether the method is curently being requeried.
        /// </summary>
        public bool IsUpdating
        {
            get { return m_isUpdating; }
        }

        /// <summary>
        /// Gets true whether a full key is needed.
        /// </summary>
        public bool IsFullKeyNeeded
        {
            get { return m_isFullKeyNeeded; }
        }

        /// <summary>
        /// Manually updates this monitor with the provided data, like if it has just been updated from CCP.
        /// </summary>
        /// <remarks>
        /// This method does not fire the <see cref="Updated"/> event.
        /// </remarks>
        /// <param name="result"></param>
        internal void UpdateWith(APIResult<T> result)
        {
            m_lastResult = result;
            m_lastUpdate = DateTime.UtcNow;
        }

        /// <summary>
        /// Forces an update.
        /// </summary>
        internal void ForceUpdate(bool retryOnError)
        {
            m_forceUpdate = true;
            m_retryOnForceUpdateError |= retryOnError;
            UpdateOnOneSecondTick();
        }

        /// <summary>
        /// Updates on every second.
        /// </summary>
        internal void UpdateOnOneSecondTick()
        {
            // Are we already updating ?
            if (m_isUpdating)
                return;

            m_isCanceled = false;

            // Is it enabled ?
            if (!m_enabled)
            {
                m_status = QueryStatus.Disabled;
                return;
            }

            // Do we have a network ?
            if (!NetworkMonitor.IsNetworkAvailable)
            {
                m_status = QueryStatus.NoNetwork;
                return;
            }

            // Check for an account
            if (!CheckAccount())
            {
                m_status = QueryStatus.NoAccount;
                return;
            }

            // Check for a full key
            if (m_isFullKeyNeeded && !HasFullKey())
            {
                m_status = QueryStatus.NoFullKey;
                return;
            }

            // Is it an auto-update test ?
            if (!m_forceUpdate)
            {
                // If not due time yet, quits
                var nextUpdate = NextUpdate;
                if (nextUpdate > DateTime.UtcNow)
                {
                    m_status = QueryStatus.Pending;
                    return;
                }
            }

            // Starts the update
            m_isUpdating = true;
            m_status = QueryStatus.Updating;
            QueryAsyncCore(EveClient.APIProviders.CurrentProvider, OnQueried);
        }

        /// <summary>
        /// Check all the required account informations are known.
        /// </summary>
        /// <returns>False if an account was required and not found.</returns>
        protected virtual bool CheckAccount()
        {
            return true;
        }

        /// <summary>
        /// Check whether there is a full key.
        /// </summary>
        /// <returns></returns>
        protected virtual bool HasFullKey()
        {
            return false;
        }

        /// <summary>
        /// Performs the query to the provider, passing the required arguments.
        /// </summary>
        /// <param name="provider">The API provider to use.</param>
        /// <param name="callback">The callback invoked on the UI thread after a result has been queried.</param>
        protected virtual void QueryAsyncCore(APIProvider provider, QueryCallback<T> callback)
        {
            provider.QueryMethodAsync<T>(m_method, callback);
        }

        /// <summary>
        /// Occurs when a new result has been queried.
        /// </summary>
        /// <param name="result">The downloaded result</param>
        protected void OnQueried(APIResult<T> result)
        {
            m_isUpdating = false;
            m_status = QueryStatus.Pending;

            // Do we need to retry the force update ?
            m_forceUpdate = (m_retryOnForceUpdateError ? result.HasError : false);

            // Was it canceled ?
            if (m_isCanceled)
                return;

            // Updates the stored data
            m_retryOnForceUpdateError = false;
            m_lastUpdate = DateTime.UtcNow;
            m_lastResult = result;

            // Notify subscribers
            if (Updated != null)
                Updated(result);
        }

        /// <summary>
        /// Resets the monitor with the given last update time.
        /// </summary>
        /// <param name="lastUpdate">The UTC time of the last update.</param>
        public void Reset(DateTime lastUpdate)
        {
            Cancel();
            m_lastUpdate = lastUpdate;
            m_lastResult = null;
        }

        /// <summary>
        /// Cancels the running update.
        /// </summary>
        public void Cancel()
        {
            m_isCanceled = true;
            m_forceUpdate = false;
        }

        /// <summary>
        /// Gets the bound method header.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return m_methodHeader;
        }

        /// <summary>
        /// Notifies the network availability changed.
        /// </summary>
        /// <param name="isAvailable"></param>
        internal void SetNetworkStatus(bool isAvailable)
        {
        }


        #region Interfaces implementations.
        void INetworkChangeSubscriber.SetNetworkStatus(bool isAvailable)
        {
            SetNetworkStatus(isAvailable);
        }

        void IQueryMonitorEx.Reset(DateTime lastUpdate)
        {
            Reset(lastUpdate);
        }

        void IQueryMonitorEx.UpdateOnOneSecondTick()
        {
            UpdateOnOneSecondTick();
        }

        void IQueryMonitorEx.ForceUpdate(bool retryOnError)
        {
            ForceUpdate(retryOnError);
        }

        IAPIResult IQueryMonitor.LastResult
        {
            get { return m_lastResult; }
        }

        #endregion
    }
}
