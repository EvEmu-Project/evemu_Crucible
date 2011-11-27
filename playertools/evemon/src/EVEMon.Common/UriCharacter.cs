using System;

using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a character bound to an uri (pointing at a character sheet) rather than CCP API
    /// </summary>
    public sealed class UriCharacter : Character
    {
        private Uri m_uri;

        /// <summary>
        /// Default constructor for new uri characters
        /// </summary>
        /// <param name="identity">The identitiy for this character</param>
        /// <param name="uri">The uri the provided deserialization object was acquired from</param>
        /// <param name="source">A deserialization object for characters</param>
        internal UriCharacter(CharacterIdentity identity, Uri uri, APIResult<SerializableAPICharacterSheet> source)
            : base(identity, Guid.NewGuid())
        {
            m_uri = uri;
            Import(source);
        }

        /// <summary>
        /// Exported character constructor
        /// </summary>
        /// <param name="identity">The identitiy for this character</param>
        /// <param name="uri">The uri the provided deserialization object was acquired from</param>
        /// <param name="source">A deserialization object for CcpCharacters</param>
        internal UriCharacter(CharacterIdentity identity, Uri uri, SerializableCCPCharacter serial)
            : base(identity, Guid.NewGuid())
        {
            m_uri = uri;
            Import(serial);
        }

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="identity"></param>
        /// <param name="serial"></param>
        internal UriCharacter(CharacterIdentity identity, SerializableUriCharacter serial)
            : base(identity, serial.Guid)
        {
            Import(serial);
        }

        /// <summary>
        /// Gets an adorned name, with (file), (url) or (cached) labels.
        /// </summary>
        public override string AdornedName
        {
            get 
            {
                return String.Format("{0} {1}", m_name, (m_uri.IsFile ? "(file)" : "(url)")); 
            }
        }

        /// <summary>
        /// Gets or sets the source's name.
        /// By default, it's the character's name
        /// but it may be overriden to help distinct tabs on the main window.
        /// </summary>
        public Uri Uri
        {
            get { return m_uri; }
            set
            {
                if (m_uri != value)
                {
                    m_uri = value;
                    EveClient.OnCharacterChanged(this);
                }
            }
        }

        /// <summary>
        /// Create a serializable character sheet for this character
        /// </summary>
        /// <returns></returns>
        public override SerializableSettingsCharacter Export()
        {
            var serial = new SerializableUriCharacter();
            Export(serial);

            serial.Uri = m_uri.ToString();
            return serial;
        }

        /// <summary>
        /// Imports data from a serialization object
        /// </summary>
        /// <param name="serial"></param>
        public void Import(SerializableUriCharacter serial)
        {
            Import((SerializableSettingsCharacter)serial);

            m_uri = new Uri(serial.Uri);

            EveClient.OnCharacterChanged(this);
        }

        /// <summary>
        /// Updates this character with the given informations
        /// </summary>
        /// <param name="identity"></param>
        /// <param name="uri"></param>
        /// <param name="result"></param>
        internal void Update(CharacterIdentity identity, Uri uri, APIResult<SerializableAPICharacterSheet> result)
        {
            m_characterID = identity.CharacterID;
            m_identity = identity;
            m_uri = uri;
            Import(result);
        }

        /// <summary>
        /// Updates this character with the given informations
        /// </summary>
        /// <param name="identity"></param>
        /// <param name="uri"></param>
        /// <param name="result"></param>
        internal void Update(CharacterIdentity identity, Uri uri, SerializableCCPCharacter result)
        {
            m_characterID = identity.CharacterID;
            m_identity = identity;
            m_uri = uri;
            Import(result);
        }
    }
}
