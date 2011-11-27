using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Serialization;

using EVEMon.Common.Serialization;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.Threading;

namespace EVEMon.Common
{
    public static class EveIDtoName
    {
        private static readonly string s_file = LocalXmlCache.GetFile("EveIDToName").FullName;

        private static List<string> s_listOfIDs = new List<string>();
        private static List<string> s_listOfNames = new List<string>();
        private static List<string> s_listOfIDsToQuery = new List<string>();
        private static Dictionary<long, string> s_cacheList = new Dictionary<long, string>();

        private static bool s_isLoaded;


        /// <summary>
        /// Gets the owner name from its ID.
        /// </summary>
        /// <param name="IDs">The ID.</param>
        /// <returns></returns>
        internal static string GetIDToName(string ID)
        {
            // If there is no ID to query return an empty string
            if (String.IsNullOrEmpty(ID))
                return String.Empty;

            // If it's a zero ID return "Unknown"
            if (ID == "0")
                return "Unknown";

            List<string> list = new List<string>();
            list.Add(ID);

            List<string> name = GetIDsToNames(list);
            return name[0];
        }

        /// <summary>
        /// Gets the owner name from its ID.
        /// </summary>
        /// <param name="IDs">The IDs.</param>
        /// <returns></returns>
        internal static List<string> GetIDsToNames(List<string> IDs)
        {
            s_listOfIDs = IDs;
            s_listOfNames.Clear();
            s_listOfIDsToQuery.Clear();

            EnsureCacheFileLoad();
            LookupForName();

            return s_listOfNames;
        }

        /// <summary>
        /// Ensures the cache file is loaded.
        /// </summary>
        private static void EnsureCacheFileLoad()
        {
            if (!File.Exists(s_file) || s_isLoaded)
                return;

            TryDeserializeCacheFile();
            s_isLoaded = true;
        }

        /// <summary>
        /// Tries to deserialize the EveIDToName file.
        /// </summary>
        private static void TryDeserializeCacheFile()
        {
            SerializableEveIDToName cache = null;

            // Deserialize the file
            cache = Util.DeserializeXML<SerializableEveIDToName>(s_file);

            // Reset the cache if anything went wrong
            if (cache.Entities.Any(x => x.ID == 0) || cache.Entities.Any(x=> x.Name == String.Empty))
                cache = null;

            if (cache == null)
            {
                EveClient.Trace("Deserializing EveIDToName failed. File may be corrupt. Deleting file.");
                File.Delete(s_file);
                return;
            }

            // Add the data to the dictionary
            foreach (var entity in cache.Entities)
            {
                s_cacheList.Add(entity.ID, entity.Name);
            }
        }

        /// <summary>
        /// Lookups for name.
        /// </summary>
        private static void LookupForName()
        {
            if (!s_cacheList.IsEmpty())
            {
                QueryCacheList();
            }
            else
            {
                s_listOfIDsToQuery = s_listOfIDs;
            }

            if (!s_listOfIDsToQuery.IsEmpty())
                QueryAPICharacterName();

            // In case the list is empty, add an "Unknown" entry
            if (s_listOfNames.Count == 0)
                s_listOfNames.Add("Unknown");
        }

        /// <summary>
        /// Queries the cache list.
        /// </summary>
        private static void QueryCacheList()
        {
            foreach (string id in s_listOfIDs)
            {
                string name = s_cacheList.FirstOrDefault(x => x.Key.ToString() == id).Value;

                if (name == null)
                {
                    s_listOfIDsToQuery.Add(id);
                }
                else
                {
                    s_listOfNames.Add(name);
                }
            }
        }

        /// <summary>
        /// Queries the API Character Name.
        /// </summary>
        private static void QueryAPICharacterName()
        {
            string IDs = string.Join(",", s_listOfIDsToQuery);
            var result = EveClient.APIProviders.CurrentProvider.QueryCharacterName(IDs);
            OnQueryAPICharacterNameUpdated(result);
        }

        /// <summary>
        /// Called when the query updated.
        /// </summary>
        /// <param name="result">The result.</param>
        private static void OnQueryAPICharacterNameUpdated(APIResult<SerializableAPICharacterName> result)
        {
            if (result.HasError)
            {
                // Checks if EVE Backend Database is temporarily disabled
                if (result.EVEBackendDatabaseDisabled)
                    return;
                
                EveClient.Notifications.NotifyCharacterNameError(result);
                return;
            }

            EveClient.Notifications.InvalidateAPIError();

            // Deserialize the result
            Import(result.Result.Entities);
        }

        /// <summary>
        /// Imports the data from the query result.
        /// </summary>
        /// <param name="entities">The entities.</param>
        private static void Import(List<SerializableCharacterNameListItem> entities)
        {
            foreach (var entity in entities)
            {
                // Add the name to the list of names
                s_listOfNames.Add(entity.Name);

                // Add the query result to our cache list if it doesn't exist already
                if (!s_cacheList.ContainsKey(entity.ID))
                    s_cacheList.Add(entity.ID, entity.Name);
            }

            // In case the list is empty, add an "Unknown" entry
            if (s_listOfNames.Count == 0)
                s_listOfNames.Add("Unknown");
        }

        /// <summary>
        /// Saves this cache list to a file.
        /// </summary>
        public static void Save()
        {
            SerializableEveIDToName serial = Export();
            XmlSerializer xs = new XmlSerializer(typeof(SerializableEveIDToName));

            // Save in file
            FileHelper.OverwriteOrWarnTheUser(s_file, fs =>
            {
                xs.Serialize(fs, serial);
                fs.Flush();
                return true;
            });
        }

        /// <summary>
        /// Exports the cache list to a serializable object.
        /// </summary>
        /// <returns></returns>
        private static SerializableEveIDToName Export()
        {
            var serial = new SerializableEveIDToName();
            var entitiesList = new List<SerializableEveIDToNameListItem>();

            foreach (var item in s_cacheList)
            {
                entitiesList.Add(new SerializableEveIDToNameListItem()
                                {
                                    ID = item.Key,
                                    Name = item.Value,
                                });
            }

            serial.Entities.AddRange(entitiesList);

            return serial;
        }
    }
}
