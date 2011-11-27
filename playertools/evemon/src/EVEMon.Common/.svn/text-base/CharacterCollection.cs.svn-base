using System;
using System.Collections.Generic;

using EVEMon.Common.Attributes;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents the list of characters for this identiity
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class CharacterCollection : IEnumerable<Character>
    {
        private readonly List<UriCharacter> m_uriCharacters = new List<UriCharacter>();
        private readonly CharacterIdentity m_characterID;
        private CCPCharacter m_ccp;

        /// <summary>
        /// Default constructor, this class is only instantiated by CharacterIdentity
        /// </summary>
        /// <param name="character">The character identitity this collection is bound to.</param>
        internal CharacterCollection(CharacterIdentity characterID)
        {
            m_characterID = characterID;
            m_ccp = new CCPCharacter(characterID);
        }

        /// <summary>
        /// Gets the CCP character
        /// </summary>
        public CCPCharacter CCPCharacter
        {
            get { return m_ccp; }
        }

        /// <summary>
        /// Gets an enumeration over the URI character
        /// </summary>
        public IEnumerable<UriCharacter> UriCharacters
        {
            get
            {
                foreach (var source in m_uriCharacters)
                {
                    yield return source;
                }
            }
        }

        /// <summary>
        /// Gets a character by its guid.
        /// </summary>
        /// <param name="uri"></param>
        /// <returns></returns>
        public Character this[Guid guid]
        {
            get
            {
                if (m_ccp.Guid == guid) return m_ccp;

                foreach (var source in m_uriCharacters)
                {
                    if (source.Guid == guid) return source;
                }

                return null;
            }
        }

        /// <summary>
        /// Gets an uri character with the given uri
        /// </summary>
        /// <param name="uri"></param>
        /// <returns></returns>
        public UriCharacter this[Uri uri]
        {
            get
            {
                foreach (var source in m_uriCharacters)
                {
                    if (source.Uri == uri) return source;
                }
                return null;
            }
        }

        /// <summary>
        /// Clears the characters before settings refresh
        /// </summary>
        internal void Clear()
        {
            m_uriCharacters.Clear();
        }

        /// <summary>
        /// Adds a character from a deserialization object
        /// </summary>
        /// <param name="serial"></param>
        internal void Add(SerializableUriCharacter serial)
        {
            var uriCharacter = this[new Uri(serial.Uri)];
            if (uriCharacter == null)
            {
                m_uriCharacters.Add(new UriCharacter(m_characterID, serial));
                EveClient.OnCharacterCollectionChanged();
            }
            else
            {
                uriCharacter.Import(serial);
            }
        }

        /// <summary>
        /// Adds a character from a deserialization object
        /// </summary>
        /// <param name="ccpCharacter"></param>
        internal void Add(SerializableCCPCharacter ccpCharacter)
        {
            m_ccp = new CCPCharacter(m_characterID, ccpCharacter);
        }

        /// <summary>
        /// Addsa new UriCharacter with the specified Uri and deserialization object, then returns it
        /// </summary>
        /// <param name="uri">The source uri</param>
        /// <param name="result">The deserialization object acquired from the given uri</param>
        /// <returns>The created character, or null if there was errors on the provided CCP data.</returns>
        internal UriCharacter Add(Uri uri, APIResult<SerializableAPICharacterSheet> result)
        {
            if (result.HasError) return null;

            var character = new UriCharacter(m_characterID, uri, result);
            m_uriCharacters.Add(character);

            EveClient.OnCharacterCollectionChanged();
            return character;
        }

        /// <summary>
        /// Removes the provided character for the uri characters
        /// </summary>
        /// <param name="character">The character to remove</param>
        /// <exception cref="InvalidOperationException">This character does not have that identity</exception>
        public void Remove(UriCharacter character)
        {
            if (!m_uriCharacters.Remove(character))
            {
                throw new InvalidOperationException("This source does not belong to this character's sources");
            }
            EveClient.OnCharacterCollectionChanged();
        }

        #region Enumerators
        IEnumerator<Character> IEnumerable<Character>.GetEnumerator()
        {
            List<Character> result = new List<Character>();
            result.Add(m_ccp);
            foreach (var uriCharacter in m_uriCharacters) result.Add(uriCharacter);

            return result.GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            List<Character> result = new List<Character>();
            result.Add(m_ccp);
            foreach (var uriCharacter in m_uriCharacters) result.Add(uriCharacter);

            return ((System.Collections.IEnumerable)result).GetEnumerator();
        }
        #endregion
    }
}
