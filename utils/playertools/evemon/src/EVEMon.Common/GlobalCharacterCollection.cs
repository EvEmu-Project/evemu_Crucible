using System;
using System.Collections.Generic;

using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Importation;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents the global collection of characters
    /// </summary>
    public sealed class GlobalCharacterCollection : ReadonlyCollection<Character>
    {
        /// <summary>
        /// Constructor
        /// </summary>
        internal GlobalCharacterCollection()
        {
        }

        /// <summary>
        /// Gets a character by its guid
        /// </summary>
        /// <param name="guid"></param>
        /// <returns></returns>
        public Character this[Guid guid]
        {
            get
            {
                foreach (var character in m_items)
                {
                    if (character.Guid == guid)
                        return character;
                }
                return null;
            }
        }

        /// <summary>
        /// Adds a character to this collection.
        /// </summary>
        /// <param name="character"></param>
        /// <param name="notify"></param>
        internal void Add(Character character, bool notify)
        {
            m_items.Add(character);
            if (notify)
                EveClient.OnCharacterCollectionChanged();
        }

        /// <summary>
        /// Removes a character from all the collections.
        /// Also removes it from the monitored characters collection, and assign it to the ignore list of its account.
        /// </summary>
        /// <param name="character"></param>
        public void Remove(Character character)
        {
            Remove(character, true);
        }

        /// <summary>
        /// Removes a character from this collection.
        /// Also removes it from the monitored characters collection, and assign it to the ignore list of its account.
        /// </summary>
        /// <param name="character"></param>
        /// <param name="notify"></param>
        internal void Remove(Character character, bool notify)
        {
            m_items.Remove(character);
            character.Monitored = false;

            // For CCP characters, also put it on the account's ignore list.
            if (character is CCPCharacter)
            {
                var account = character.Identity.Account;
                if (account != null)
                    account.IgnoreList.Add(character as CCPCharacter);
            }

            if (notify)
                EveClient.OnCharacterCollectionChanged();
        }

        /// <summary>
        /// Asynchronously adds a character from the given uri, adding a new identity when needed.
        /// </summary>
        /// <param name="uri">The uri to load the character sheet from</param>
        /// <param name="callback">A callback invoked on the UI thread (whatever the result, success or failure)</param>
        public void TryAddOrUpdateFromUriAsync(Uri uri, EventHandler<UriCharacterEventArgs> callback)
        {
            // We have a file, let's just deserialize it synchronously
            if (uri.IsFile)
            {
                string format = Util.GetXmlRootElement(uri.LocalPath);

                switch (format.ToLower(CultureConstants.DefaultCulture))
                {
                    case "eveapi":
                        var apiResult = Util.DeserializeAPIResult<SerializableAPICharacterSheet>(uri.ToString(), APIProvider.RowsetsTransform);
                        callback(null, new UriCharacterEventArgs(uri, apiResult));
                        break;
                    case "serializableccpcharacter":
                        try
                        {
                            var ccpResult = Util.DeserializeXML<SerializableCCPCharacter>(uri.ToString());
                            callback(null, new UriCharacterEventArgs(uri, ccpResult));
                        }
                        catch (NullReferenceException ex)
                        {
                            callback(null, new UriCharacterEventArgs(uri, String.Format(CultureConstants.DefaultCulture, "Unable to load file (SerializableCCPCharacter). ({0})", ex.Message)));
                        }
                        break;
                    case "character":
                        try
                        {
                            var oldCharacterResult = Util.DeserializeXML<OldExportedCharacter>(uri.ToString());
                            var ccpCharacterResult = oldCharacterResult.ToSerializableCCPCharacter();
                            callback(null, new UriCharacterEventArgs(uri, ccpCharacterResult));
                        }
                        catch (NullReferenceException ex)
                        {
                            callback(null, new UriCharacterEventArgs(uri, String.Format(CultureConstants.DefaultCulture, "Unable to load file (Character). ({0})", ex.Message)));
                        }
                        break;
                    default:
                        callback(null, new UriCharacterEventArgs(uri, "Format Not Recognized"));
                        break;
                }
            }
            // So, it's a web address, let's do it in an async way.
            else
            {
                Util.DownloadAPIResultAsync<SerializableAPICharacterSheet>(uri.ToString(), null, APIProvider.RowsetsTransform, 
                    (result) => callback(null, new UriCharacterEventArgs(uri, result)));
            }
        }

        /// <summary>
        /// Imports the character identities from a serialization object
        /// </summary>
        /// <param name="serial"></param>
        /// <param name="charactersGuid"></param>
        internal void Import(IEnumerable<SerializableSettingsCharacter> serial)
        {
            // Clear the accounts on every identity
            foreach (var id in EveClient.CharacterIdentities)
            {
                id.Account = null;
            }

            // Import the characters, their identies, etc
            m_items.Clear();
            foreach (var serialCharacter in serial)
            {
                // Gets the identity or create it
                var id = EveClient.CharacterIdentities[serialCharacter.ID];
                if (id == null)
                    id = EveClient.CharacterIdentities.Add(serialCharacter.ID, serialCharacter.Name);

                // Imports the character
                var ccpCharacter = serialCharacter as SerializableCCPCharacter;
                if (ccpCharacter != null)
                {
                    m_items.Add(new CCPCharacter(id, ccpCharacter));
                }
                else
                {
                    var uriCharacter = serialCharacter as SerializableUriCharacter;
                    m_items.Add(new UriCharacter(id, uriCharacter));
                }
            }

            // Notify the change
            EveClient.OnCharacterCollectionChanged();
        }

        /// <summary>
        /// Exports this collection to a serialization object
        /// </summary>
        /// <returns></returns>
        internal List<SerializableSettingsCharacter> Export()
        {
            var serial = new List<SerializableSettingsCharacter>();

            foreach (var character in m_items)
            {
                serial.Add(character.Export());
            }

            return serial;
        }

        /// <summary>
        /// imports the plans from serialization objects
        /// </summary>
        /// <param name="list"></param>
        internal void ImportPlans(List<SerializablePlan> serial)
        {
            foreach (var character in m_items)
            {
                character.ImportPlans(serial);
            }
        }

        /// <summary>
        /// Exports the plans as serialization objects
        /// </summary>
        /// <returns></returns>
        internal List<SerializablePlan> ExportPlans()
        {
            var serial = new List<SerializablePlan>();
            foreach (var character in m_items)
            {
                character.ExportPlans(serial);
            }

            return serial;
        }
    }
}
