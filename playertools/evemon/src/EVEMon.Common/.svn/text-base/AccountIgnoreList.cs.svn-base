using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a collection of character identities an account should never import.
    /// </summary>
    public sealed class AccountIgnoreList : ReadonlyCollection<CharacterIdentity>
    {
        private readonly Account m_owner;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="account"></param>
        internal AccountIgnoreList(Account account)
        {
            m_owner = account;
        }

        /// <summary>
        /// Checks whether the given character's associated identity is contained in this list.
        /// </summary>
        /// <param name="character"></param>
        /// <returns></returns>
        public bool Contains(Character character)
        {
            return Contains(character.Identity);
        }

        /// <summary>
        /// Removes this character and attempts to return a CCP character. 
        /// The resulting character will be the existing one matching this id, or if it does not exist, a new character. 
        /// If the identity was not in the collection, the method won't attempt to create a new character and will return either the existing one or null.
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public CCPCharacter Remove(CharacterIdentity id)
        {
            // If the id was not in list, returns the existing character or null if it does not exist.
            var ccpCharacter = id.CCPCharacter;
            if (!m_items.Remove(id))
                return ccpCharacter;

            // If character exists, returns it.
            if (ccpCharacter != null)
                return ccpCharacter;
                
            // Create a new CCP character
            ccpCharacter = new CCPCharacter(id);
            EveClient.Characters.Add(ccpCharacter, true);
            return ccpCharacter;
        }

        /// <summary>
        /// Adds a character to the ignore list and, if it belonged to this account, removes it from the global collection 
        /// (all associated data and plans won't be written on next serialization !).
        /// </summary>
        /// <param name="character"></param>
        public void Add(CCPCharacter character)
        {
            var id = character.Identity;
            if (m_items.Contains(id))
                return;

            m_items.Add(id);

            // If the identity was belonging to this account, remove the character (won't be serialized anymore !)
            if (id.Account == m_owner)
            {
                EveClient.Characters.Remove(character, true);
            }
        }

        /// <summary>
        /// Imports the deserialization objects.
        /// </summary>
        /// <param name="serialIDList"></param>
        internal void Import(List<SerializableCharacterIdentity> serialIDList)
        {
            m_items.Clear();
            foreach (var serialID in serialIDList)
            {
                var id = EveClient.CharacterIdentities[serialID.ID];
                if (id == null)
                {
                    id = EveClient.CharacterIdentities.Add(serialID.ID, serialID.Name);
                }
                m_items.Add(id);
            }
        }

        /// <summary>
        /// Create serialization objects.
        /// </summary>
        /// <returns></returns>
        internal List<SerializableCharacterIdentity> Export()
        {
            var serial = new List<SerializableCharacterIdentity>();
            foreach (var id in m_items)
            {
                serial.Add(new SerializableCharacterIdentity { ID = id.CharacterID, Name = id.Name });
            }
            return serial;
        }
    }
}
