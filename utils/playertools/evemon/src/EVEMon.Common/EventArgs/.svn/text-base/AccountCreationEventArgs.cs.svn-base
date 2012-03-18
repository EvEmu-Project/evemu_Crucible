using System;
using System.Collections.Generic;

using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    /// <summary>
    /// Event arguments for the creation of a new account.
    /// </summary>
    public sealed class AccountCreationEventArgs : EventArgs
    {
        private readonly long m_userID;
        private readonly string m_apiKey;
        private readonly APIResult<SerializableAPIAccountStatus> m_serialAccountStatus;
        private readonly APIResult<SerializableAPICharacters> m_serialCharacterList;
        private readonly List<CharacterIdentity> m_identities = new List<CharacterIdentity>();
        private readonly string m_fullKeyError;
        private readonly CredentialsLevel m_keyLevel;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="userID"></param>
        /// <param name="apiKey"></param>
        /// <param name="serialCharacterList"></param>
        /// <param name="serialBalanceList"></param>
        internal AccountCreationEventArgs(long userID, string apiKey, 
            APIResult<SerializableAPIAccountStatus> serialAccountStatus,
            APIResult<SerializableAPICharacters> serialCharacterList)
        {
            m_userID = userID;
            m_apiKey = apiKey;
            m_serialAccountStatus = serialAccountStatus;
            m_serialCharacterList = serialCharacterList;
            m_keyLevel = CredentialsLevel.Unknown;
            m_fullKeyError = String.Empty;

            //Determine the API key level
            m_keyLevel = Account.GetCredentialsLevel(serialAccountStatus);

            // On error, retrieve the error message
            if (m_keyLevel == CredentialsLevel.Unknown)
                m_fullKeyError = serialAccountStatus.ErrorMessage;

            // Retrieves the characters list
            if (m_serialCharacterList.HasError)
                return;

            foreach (var serialID in m_serialCharacterList.Result.Characters)
            {
                // Look for an existing char ID and update its name
                var id = EveClient.CharacterIdentities[serialID.ID];
                if (id != null)
                {
                    id.Name = serialID.Name;
                }
                else
                {
                    // Create an identity if necessary
                    id = EveClient.CharacterIdentities.Add(serialID.ID, serialID.Name);
                }

                m_identities.Add(id);
            }
        }

        /// <summary>
        /// Gets the account's user ID.
        /// </summary>
        public long UserID
        {
            get { return m_userID; }
        }

        /// <summary>
        /// Gets the API key.
        /// </summary>
        public string ApiKey
        {
            get { return m_apiKey; }
        }

        /// <summary>
        /// Gets the API key level.
        /// </summary>
        public CredentialsLevel KeyLevel
        {
            get { return m_keyLevel; }
        }

        /// <summary>
        /// Gets the possible error message gotten when testing the key security (excluding the error meaning the key was a limited one).
        /// </summary>
        public string FullKeyTestError
        {
            get { return m_fullKeyError; }
        }

        /// <summary>
        /// Gets the result which occurred when the characters list was queried.
        /// </summary>
        public IAPIResult Result
        {
            get { return m_serialCharacterList; }
        }

        /// <summary>
        /// Gets the enumeration of identities available on this account.
        /// </summary>
        public IEnumerable<CharacterIdentity> Identities
        {
            get
            {
                foreach (var id in m_identities)
                {
                    yield return id;
                }
            }
        }

        /// <summary>
        /// Creates or updates the account.
        /// </summary>
        /// <returns></returns>
        public Account CreateOrUpdate()
        {
            // Checks whether this account already exists to update it.
            var account = EveClient.Accounts[m_userID];
            if (account != null)
            {
                account.UpdateAPIKey(m_apiKey, m_keyLevel, m_identities, m_serialCharacterList, m_serialAccountStatus);

                // Collection did not change but there is no "AccountChanged" event
                EveClient.OnAccountCollectionChanged();
            }
            else
            {
                account = new Account(m_userID);
                account.UpdateAPIKey(m_apiKey, m_keyLevel, m_identities, m_serialCharacterList, m_serialAccountStatus);
                EveClient.Accounts.Add(account, true);
            }

            return account;
        }

    }
}
