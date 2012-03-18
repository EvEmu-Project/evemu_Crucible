using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a monitor for all queries related to characters and their corporations.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public sealed class CharacterQueryMonitor<T> : QueryMonitor<T>
    {
        private readonly Character m_character;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="character"></param>
        internal CharacterQueryMonitor(Character character, APIMethods method)
            : base(method)
        {
            m_character = character;
        }

        /// <summary>
        /// Check all the required account informations are known.
        /// </summary>
        /// <returns>False if an account was required and not found.</returns>
        protected override bool CheckAccount()
        {
            var account = m_character.Identity.Account;
            return account != null;
        }

        /// <summary>
        /// Check whether there is a full key.
        /// </summary>
        /// <returns></returns>
        protected override bool HasFullKey()
        {
            var account = m_character.Identity.Account;
            return account != null && account.KeyLevel == CredentialsLevel.Full;
        }

        /// <summary>
        /// Performs the query to the provider, passing the required arguments.
        /// </summary>
        /// <param name="provider">The API provider to use.</param>
        /// <param name="callback">The callback invoked on the UI thread after a result has been queried.</param>
        protected override void QueryAsyncCore(APIProvider provider, QueryCallback<T> callback)
        {
            var account = m_character.Identity.Account;
            provider.QueryMethodAsync<T>(m_method, account.UserID, account.APIKey, m_character.CharacterID, callback);
        }
    }
}
