using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a monitor for all queries related to accounts.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public sealed class AccountQueryMonitor<T> : QueryMonitor<T>
    {
        private readonly Account m_account;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="account"></param>
        internal AccountQueryMonitor(Account account, APIMethods method)
            : base(method)
        {
            m_account = account;
        }

        /// <summary>
        /// Check whether there is a full key.
        /// </summary>
        /// <returns></returns>
        protected override bool HasFullKey()
        {
            return m_account.KeyLevel == CredentialsLevel.Full;
        }

        /// <summary>
        /// Performs the query to the provider, passing the required arguments.
        /// </summary>
        /// <param name="provider">The API provider to use.</param>
        /// <param name="callback">The callback invoked on the UI thread after a result has been queried.</param>
        protected override void QueryAsyncCore(APIProvider provider, QueryCallback<T> callback)
        {
            provider.QueryMethodAsync<T>(m_method, m_account.UserID, m_account.APIKey, callback);
        }
    }
}
