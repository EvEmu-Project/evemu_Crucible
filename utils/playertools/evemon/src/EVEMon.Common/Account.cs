using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common.Attributes;
using EVEMon.Common.Notifications;
using EVEMon.Common.Net;
using EVEMon.Common.Serialization;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a player account
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class Account
    {
        private readonly AccountQueryMonitor<SerializableAPIAccountStatus> m_accountStatusMonitor;
        private readonly AccountQueryMonitor<SerializableAPICharacters> m_charactersListMonitor;
        private readonly AccountIgnoreList m_ignoreList;

        private readonly Dictionary<String, SkillInTrainingResponse> m_skillInTrainingCache =
            new Dictionary<String, SkillInTrainingResponse>();

        private readonly long m_userId;
        private string m_apiKey;
        private CredentialsLevel m_keyLevel;
        private DateTime m_lastKeyLevelUpdate = DateTime.MinValue;
        private DateTime m_accountExpirationDate;
        private DateTime m_accountCreated;
        private bool m_updatePending;
        private bool m_characterListUpdated;

        #region Constructors

        /// <summary>
        /// Common constructor base.
        /// </summary>
        private Account()
        {
            m_charactersListMonitor = new AccountQueryMonitor<SerializableAPICharacters>(this, APIMethods.CharacterList);
            m_charactersListMonitor.Updated += OnCharactersListUpdated;

            m_accountStatusMonitor = new AccountQueryMonitor<SerializableAPIAccountStatus>(this, APIMethods.AccountStatus);
            m_accountStatusMonitor.Updated += OnAccountStatusUpdated;

            m_ignoreList = new AccountIgnoreList(this);
        }

        /// <summary>
        /// Deserialization constructor.
        /// </summary>
        /// <param name="serial"></param>
        internal Account(SerializableAccount serial)
            : this()
        {
            m_userId = serial.ID;
            m_apiKey = serial.Key;
            m_keyLevel = serial.KeyLevel;
            m_accountExpirationDate = serial.PaidUntil;
            m_accountCreated = serial.CreateDate;
            m_ignoreList.Import(serial.IgnoreList);
        }

        /// <summary>
        /// Constructor from the provided informations.
        /// </summary>
        /// <param name="userID"></param>
        internal Account(long userID)
            : this()
        {
            m_userId = userID;
            m_apiKey = String.Empty;
        }

        #endregion

        #region Public Properties

        /// <summary>
        /// Gets true whether this key is a full key.
        /// </summary>
        public CredentialsLevel KeyLevel
        {
            get { return m_keyLevel; }
        }

        /// <summary>
        /// Gets / sets the user's id.
        /// </summary>
        public long UserID
        {
            get { return m_userId; }
        }

        /// <summary>
        /// Gets / sets the API key.
        /// </summary>
        public string APIKey
        {
            get { return m_apiKey; }
        }

        /// <summary>
        /// Gets the list of items to never import.
        /// </summary>
        public AccountIgnoreList IgnoreList
        {
            get { return m_ignoreList; }
        }

        /// <summary>
        /// Gets the account expiration date and time.
        /// </summary>
        public DateTime AccountExpiration
        {
            get { return m_accountExpirationDate.ToLocalTime(); }
        }

        /// <summary>
        /// Gets the account expiration date and time.
        /// </summary>
        public DateTime AccountCreated
        {
            get { return m_accountCreated.ToLocalTime(); }
        }

        /// <summary>
        /// Gets true if this account has a character in training.
        /// </summary>
        public bool HasCharacterInTraining
        {
            get { return TrainingCharacter != null; }
        }

        /// <summary>
        /// Gets the character identities for this account.
        /// </summary>
        public IEnumerable<CharacterIdentity> CharacterIdentities
        {
            get
            {
                foreach (CharacterIdentity characterID in EveClient.CharacterIdentities)
                {
                    if (characterID.Account == this)
                        yield return characterID;
                }
            }
        }

        /// <summary>
        /// Gets true if at least one of the CCP characters is monitored.
        /// </summary>
        public bool HasMonitoredCharacters
        {
            get
            {
                foreach (CharacterIdentity id in CharacterIdentities)
                {
                    CCPCharacter ccpCharacter = id.CCPCharacter;
                    if (ccpCharacter != null && ccpCharacter.Monitored)
                        return true;
                }
                return false;
            }
        }

        /// <summary>
        /// Gets the character in training on this account, or null if none are in training.
        /// </summary>
        /// <remarks>Returns null if the character is in the ignored list.</remarks>
        public CCPCharacter TrainingCharacter
        {
            get
            {
                // Scroll through owned identities
                foreach (CharacterIdentity id in CharacterIdentities)
                {
                    CCPCharacter ccpCharacter = id.CCPCharacter;
                    if (ccpCharacter != null && ccpCharacter.IsTraining)
                        return ccpCharacter;
                }
                return null;
            }
        }

        #endregion

        #region Internal Methods

        /// <summary>
        /// Query skills in training for characters on this account.
        /// </summary>
        internal void CharacterInTraining()
        {
            if (m_updatePending)
                return;

            m_updatePending = true;
            m_skillInTrainingCache.Clear();

            // Quits if no network
            if (!NetworkMonitor.IsNetworkAvailable)
                return;

            foreach (CharacterIdentity id in CharacterIdentities)
            {
                string identity = id.Name;

                if (!m_skillInTrainingCache.ContainsKey(identity))
                    m_skillInTrainingCache.Add(identity, new SkillInTrainingResponse());

                EveClient.APIProviders.CurrentProvider.QueryMethodAsync<SerializableAPISkillInTraining>(
                    APIMethods.CharacterSkillInTraining,
                    m_userId,
                    m_apiKey,
                    id.CharacterID,
                    x => OnSkillInTrainingUpdated(x, identity));
            }
        }

        /// <summary>
        /// Updates the characters list on a timer tick.
        /// </summary>
        internal void UpdateOnOneSecondTick()
        {
            m_charactersListMonitor.UpdateOnOneSecondTick();
            
            // We trigger the account status check when we have the character list of the account
            // in order to have better acccount related info in the trace file
            if (m_characterListUpdated)
                m_accountStatusMonitor.UpdateOnOneSecondTick();

            // While the key status is unknown, every five minutes, we try to update it
            if (m_keyLevel == CredentialsLevel.Unknown && DateTime.UtcNow >= m_lastKeyLevelUpdate.AddMinutes(5))
            {
                // Quits if no network
                if (!NetworkMonitor.IsNetworkAvailable)
                    return;

                // Query the account status
                EveClient.APIProviders.CurrentProvider.QueryMethodAsync<SerializableAPIAccountStatus>(
                    APIMethods.AccountStatus, m_userId, m_apiKey, OnKeyLevelUpdated);
            }
        }

        /// <summary>
        /// Gets the credential level from the given result.
        /// </summary>
        /// <param name="result"></param>
        internal static CredentialsLevel GetCredentialsLevel(APIResult<SerializableAPIAccountStatus> result)
        {
            // No error ? Then it is a full key
            if (!result.HasError)
                return CredentialsLevel.Full;

            // Error code 200 means it was a limited key
            if (result.CCPError != null && result.CCPError.IsLimitedKeyError)
                return CredentialsLevel.Limited;

            // Another error occurred 
            return CredentialsLevel.Unknown;
        }

        /// <summary>
        /// Updates the characters list with the given CCP data.
        /// </summary>
        /// <param name="result"></param>
        internal void Import(APIResult<SerializableAPICharacters> result)
        {
            if (result.HasError)
            {
                ImportIdentities(null);
            }
            else
            {
                ImportIdentities(result.Result.Characters);
            }

            // Fires the event regarding the account character list update
            EveClient.OnCharacterListChanged(this);
        }

        /// <summary>
        /// Exports the data to a serialization object.
        /// </summary>
        /// <returns></returns>
        internal SerializableAccount Export()
        {
            return new SerializableAccount
                       {
                           ID = m_userId,
                           Key = m_apiKey,
                           KeyLevel = m_keyLevel,
                           PaidUntil = m_accountExpirationDate,
                           CreateDate = m_accountCreated,
                           LastAccountStatusUpdate = m_accountStatusMonitor.LastUpdate,
                           LastCharacterListUpdate = m_charactersListMonitor.LastUpdate,
                           IgnoreList = m_ignoreList.Export()
                       };
        }

        #endregion

        #region Response To Events

        /// <summary>
        /// Used when the character list has been queried.
        /// </summary>
        /// <param name="result"></param>
        private void OnCharactersListUpdated(APIResult<SerializableAPICharacters> result)
        {
            // Notify on error
            if (result.HasError)
            {
                // Checks if EVE Backend Database is temporarily disabled
                if (result.EVEBackendDatabaseDisabled)
                    return;

                EveClient.Notifications.NotifyCharacterListError(this, result);
                return;
            }

            // Invalidates the notification and update
            EveClient.Notifications.InvalidateAccountError(this);
            Import(result);

            m_characterListUpdated = true;
        }

        /// <summary>
        /// Update when we can update the key level.
        /// </summary>
        /// <param name="result"></param>
        private void OnKeyLevelUpdated(APIResult<SerializableAPIAccountStatus> result)
        {
            m_lastKeyLevelUpdate = DateTime.UtcNow;
            m_keyLevel = GetCredentialsLevel(result);

            // Notify error if any
            if (m_keyLevel == CredentialsLevel.Unknown)
            {
                EveClient.Notifications.NotifyKeyLevelError(this, result);
                return;
            }

            // Notify characters changed
            foreach (CharacterIdentity id in CharacterIdentities)
            {
                CCPCharacter ccpCharacter = id.CCPCharacter;
                if (ccpCharacter != null)
                    EveClient.OnCharacterChanged(ccpCharacter);
            }
        }

        /// <summary>
        /// Called when character's skill in training gets updated.
        /// </summary>
        /// <param name="result">The result.</param>
        /// <param name="character">The character's name.</param>
        private void OnSkillInTrainingUpdated(APIResult<SerializableAPISkillInTraining> result, string characterName)
        {
            CCPCharacter ccpCharacter = EveClient.Characters.FirstOrDefault(x => x.Name == characterName) as CCPCharacter;

            // Return on error
            if (result.HasError)
            {
                // Checks if EVE Backend Database is temporarily disabled
                if (result.EVEBackendDatabaseDisabled)
                    return;
                
                if (ccpCharacter != null)
                    EveClient.Notifications.NotifySkillInTrainingError(ccpCharacter, result);
                
                m_skillInTrainingCache[characterName].State = ResponseState.InError;
                return;
            }

            if (ccpCharacter != null)
                EveClient.Notifications.InvalidateCharacterAPIError(ccpCharacter);

            m_skillInTrainingCache[characterName].State = result.Result.SkillInTraining == 1
                                                     ? ResponseState.Training
                                                     : ResponseState.NotTraining;

            // In the event this becomes a very long running process because of latency
            // and characters have been removed from the account since they were queried
            // remove those characters from the cache
            IEnumerable<KeyValuePair<string, SkillInTrainingResponse>> toRemove =
                m_skillInTrainingCache.Where(x => !CharacterIdentities.Any(y => y.Name == x.Key));

            foreach (var charToRemove in toRemove)
            {
                m_skillInTrainingCache.Remove(charToRemove.Key);
            }

            // If we did not get response from a character in account yet
            // or there was an error in any responce,
            // we are not sure so wait until next time
            if (m_skillInTrainingCache.Any(x => x.Value.State == ResponseState.Unknown
                                             || x.Value.State == ResponseState.InError))
                return;

            // We have successful responces from all characters in account,
            // so we notify the user and fire the event
            NotifyAccountNotInTraining();

            // Fires the event regarding the account characters skill in training update
            EveClient.OnAccountCharactersSkillInTrainingUpdated(this);

            // Reset update pending flag
            m_updatePending = false;
        }

        /// <summary>
        /// Called when the account status has been updated.
        /// </summary>
        /// <param name="result">The result.</param>
        private void OnAccountStatusUpdated(APIResult<SerializableAPIAccountStatus> result)
        {
            // Return on error
            if (result.HasError)
            {
                // Checks if EVE Backend Database is temporarily disabled
                if (result.EVEBackendDatabaseDisabled)
                    return;
                
                EveClient.Notifications.NotifyAccountStatusError(this, result);
                return;
            }

            EveClient.Notifications.InvalidateAccountError(this);

            m_accountCreated = result.Result.CreateDate;
            m_accountExpirationDate = result.Result.PaidUntil;

            // Notifies for the account expiration
            NotifyAccountExpiration();

            // Fires the event regarding the account status update
            EveClient.OnAccountStatusUpdated(this);
        }

        #endregion

        #region Helper Methods

        /// <summary>
        /// Updates the characters list with the given CCP data.
        /// </summary>
        /// <param name="identities"></param>
        private void ImportIdentities(IEnumerable<ISerializableCharacterIdentity> identities)
        {
            // Clear the accounts on this character
            foreach (CharacterIdentity id in EveClient.CharacterIdentities)
            {
                if (id.Account == this)
                    id.Account = null;
            }

            // Return if there were errors in the query
            if (identities == null)
                return;

            // Assign owned identities to this account
            foreach (ISerializableCharacterIdentity serialID in identities)
            {
                CharacterIdentity id = EveClient.CharacterIdentities[serialID.ID];
                if (id == null)
                    id = EveClient.CharacterIdentities.Add(serialID.ID, serialID.Name);

                id.Account = this;
            }
        }

        /// <summary>
        /// Notifies if an account is not in training.
        /// </summary>
        private void NotifyAccountNotInTraining()
        {
            // One of the remaining characters was training; account is training
            if (m_skillInTrainingCache.Any(x => x.Value.State == ResponseState.Training))
            {
                EveClient.Notifications.InvalidateAccountNotInTraining(this);
                return;
            }

            // No training characters found up until
            EveClient.Notifications.NotifyAccountNotInTraining(this);
        }

        /// <summary>
        /// Notifies for the account expiration.
        /// </summary>
        private void NotifyAccountExpiration()
        {
            // Is it to expire within 7 days? Send an informative notification
            TimeSpan daysToExpire = m_accountExpirationDate.Subtract(DateTime.UtcNow);
            if (daysToExpire < TimeSpan.FromDays(7) && daysToExpire > TimeSpan.FromDays(1))
            {
                EveClient.Notifications.NotifyAccountExpiration(this, m_accountExpirationDate, NotificationPriority.Information);
                return;
            }

            // Is it to expire within the day? Send a warning notification
            if (daysToExpire <= TimeSpan.FromDays(1) && daysToExpire > TimeSpan.Zero)
            {
                EveClient.Notifications.NotifyAccountExpiration(this, m_accountExpirationDate, NotificationPriority.Warning);
                return;
            }

            EveClient.Notifications.InvalidateAccountExpiration(this);
        }

        #endregion

        #region Public Methods

        /// <summary>
        /// Asynchronously updates this account through a <see cref="AccountCreationEventArgs"/>.
        /// </summary>
        /// <param name="apiKey"></param>
        /// <param name="callback">A callback invoked on the UI thread (whatever the result, success or failure)</param>
        /// <returns></returns>
        public void TryUpdateAsync(string apiKey, EventHandler<AccountCreationEventArgs> callback)
        {
            EveClient.Accounts.TryAddOrUpdateAsync(m_userId, apiKey, callback);
        }

        /// <summary>
        /// Updates the account with the informations extracted from the API by <see cref="AccountCreationEventArgs"/>.
        /// </summary>
        /// <param name="apiKey"></param>
        /// <param name="keyLevel"></param>
        /// <param name="identities"></param>
        /// <param name="charListQueryResult"></param>
        internal void UpdateAPIKey(string apiKey, CredentialsLevel keyLevel, IEnumerable<CharacterIdentity> identities,
                                    APIResult<SerializableAPICharacters> charListQueryResult,
                                    APIResult<SerializableAPIAccountStatus> accountStatusQueryResult)
        {
            m_apiKey = apiKey;
            m_keyLevel = keyLevel;
            m_charactersListMonitor.UpdateWith(charListQueryResult);
            m_accountStatusMonitor.UpdateWith(accountStatusQueryResult);

            // Clear the account for the currently associated identities
            foreach (CharacterIdentity id in EveClient.CharacterIdentities)
            {
                if (id.Account == this)
                    id.Account = null;
            }

            // Assign this account to the new identities and create CCP characters
            foreach (CharacterIdentity id in identities)
            {
                // Skip if in the ignore list
                id.Account = this;
                if (m_ignoreList.Contains(id))
                    continue;

                // Retrieves the ccp character and create one if none
                CCPCharacter ccpCharacter = id.CCPCharacter;
                if (ccpCharacter == null)
                {
                    ccpCharacter = new CCPCharacter(id);
                    EveClient.Characters.Add(ccpCharacter, true);
                    ccpCharacter.Monitored = true;
                }
            }
        }

        #endregion

        #region Overridden Methods

        /// <summary>
        /// Gets a string representation of this account, under the given format : 123456 (John Doe, Joe Dohn).
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            // If no characters on this account, return a "no characters" mention
            if (CharacterIdentities.Count() == 0)
                return String.Format("{0} (no characters)", m_userId);

            // Otherwise, return the chars' names into parenthesis
            string names = String.Empty;
            foreach (CharacterIdentity id in CharacterIdentities)
            {
                names += id.Name;
                names += ", ";
            }
            return String.Format("{0} ({1})", m_userId, names.TrimEnd(", ".ToCharArray()));
        }

        #endregion

        #region Helper Class

        #region Nested type: ResponseState

        private enum ResponseState
        {
            Unknown,
            InError,
            Training,
            NotTraining
        }

        #endregion

        #region Nested type: SkillInTrainingResponse

        private class SkillInTrainingResponse
        {
            private ResponseState m_state;

            public SkillInTrainingResponse()
            {
                State = ResponseState.Unknown;
                Timestamp = DateTime.MinValue;
            }

            public ResponseState State
            {
                get { return m_state; }
                set
                {
                    Timestamp = DateTime.Now;
                    m_state = value;
                }
            }

            public DateTime Timestamp { get; set; }
        }

        #endregion

        #endregion
    }
}