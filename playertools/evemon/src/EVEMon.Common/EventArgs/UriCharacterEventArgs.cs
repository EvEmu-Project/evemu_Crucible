using System;

using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents the argument for the callback for uri characters addition
    /// </summary>
    public sealed class UriCharacterEventArgs : EventArgs
    {
        private Uri m_uri;
        private APIResult<SerializableAPICharacterSheet> m_apiResult;
        private SerializableCharacterSheetBase m_result;

        /// <summary>
        /// Constructor for API Characters.
        /// </summary>
        /// <param name="uri">URI of the character</param>
        /// <param name="result">API Result</param>
        public UriCharacterEventArgs(Uri uri, APIResult<SerializableAPICharacterSheet> result)
        {
            m_uri = uri;
            m_apiResult = result;
            m_result = m_apiResult.Result;
            HasError = m_apiResult.HasError;
            Error = m_apiResult.ErrorMessage;
        }

        /// <summary>
        /// Constructor for CCP Characters.
        /// </summary>
        /// <param name="uri">URI of the character</param>
        /// <param name="result">Serialized Result</param>
        public UriCharacterEventArgs(Uri uri, SerializableCCPCharacter result)
        {
            m_uri = uri;
            m_result = result;
            HasError = false;
            Error = String.Empty;
        }

        /// <summary>
        /// Constructor for characters that throw errors.
        /// </summary>
        /// <param name="uri">URI of the chracter</param>
        /// <param name="error"></param>
        public UriCharacterEventArgs(Uri uri, string error)
        {
            m_uri = uri;
            HasError = true;
            Error = error;
        }

        /// <summary>
        /// Gets the created Uri character.
        /// </summary>
        public Uri Uri
        {
            get { return m_uri; }
        }

        /// <summary>
        /// Gets or sets a value indicating if there was an error.
        /// </summary>
        public bool HasError
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets or sets the error which occurred.
        /// </summary>
        public string Error
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the character's name.
        /// </summary>
        public string CharacterName
        {
            get { return m_result.Name; }
        }

        /// <summary>
        /// Creates the character.
        /// </summary>
        public UriCharacter CreateCharacter()
        {
            var identity = GetIdentity(m_result);

            // Instantiates characters, adds, notify
            UriCharacter uriCharacter;
            if (m_apiResult != null)
            {
                uriCharacter = new UriCharacter(identity, m_uri, m_apiResult);
            }
            else
            {
                var ccpCharacter = m_result as SerializableCCPCharacter;
                uriCharacter = new UriCharacter(identity, m_uri, ccpCharacter);
            }

            EveClient.Characters.Add(uriCharacter, true);
            return uriCharacter;
        }

        /// <summary>
        /// Updates the given character.
        /// </summary>
        /// <param name="character"></param>
        public void UpdateCharacter(UriCharacter character)
        {
            var identity = GetIdentity(m_result);

            // Updates
            if (m_apiResult != null)
            {
                character.Update(identity, m_uri, m_apiResult);
            }
            else
            {
                var ccpCharacter = m_result as SerializableCCPCharacter;
                character.Update(identity, m_uri, ccpCharacter);
            }
        }

        /// <summary>
        /// Gets the character identity.
        /// </summary>
        /// <param name="character">The character.</param>
        /// <returns></returns>
        private CharacterIdentity GetIdentity(SerializableCharacterSheetBase character)
        {
            // Retrieve the identity and create one if needed
            var identity = EveClient.CharacterIdentities[character.ID];
            if (identity == null)
            {
                identity = EveClient.CharacterIdentities.Add(character.ID, character.Name);
            }
            return identity;
        }
    }
}
