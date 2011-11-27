using System;
using System.Collections.Generic;
using System.IO;

using EVEMon.Common.Data;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Threading;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a conquerable station inside the EVE universe.
    /// </summary>
    public sealed class ConquerableStation : Station
    {
        private readonly static Dictionary<long, ConquerableStation> s_conqStationsByID = new Dictionary<long, ConquerableStation>();
        private readonly static string s_filename = "ConquerableStationList";

        private static bool s_loaded;
        private string m_corp;

        /// <summary>
        /// Constructor.
        /// </summary>
        private ConquerableStation(SerializableOutpost src)
            : base (src)
        {
            m_corp = src.CorporationName;
        }


        #region Properties

        /// <summary>
        /// Gets an enumeration of all the stations in the universe.
        /// </summary>
        internal static IEnumerable<ConquerableStation> AllStations
        {
            get
            {
                // Ensure list importation
                EnsureImportation();

                foreach (var conquerableStation in s_conqStationsByID.Values)
                {
                    yield return conquerableStation;
                }
            }
        }

        /// <summary>
        /// Gets something like OwnerName - StationName.
        /// </summary>
        public string FullName
        {
            get { return m_corp + " - " + Name; }
        }

        /// <summary>
        /// Gets something like Region > Constellation > SolarSystem > CorpName - OutpostName.
        /// </summary>
        public new string FullLocation
        {
            get { return SolarSystem.FullLocation + " > " + FullName; }
        }
        
        #endregion


        #region File Updating
        /// <summary>
        /// Downloads the conquerable station list,
        /// while doing a file up to date check.
        /// </summary>
        private static void UpdateList()
        {
            // Set the update time and period
            DateTime updateTime = DateTime.Today.AddHours(EveConstants.DowntimeHour).AddMinutes(EveConstants.DowntimeDuration);
            TimeSpan updatePeriod = TimeSpan.FromDays(1);

            // Check to see if file is up to date
            bool fileUpToDate = LocalXmlCache.CheckFileUpToDate(s_filename, updateTime, updatePeriod);

            // Up to date ? Quit
            if (fileUpToDate)
                return;

            // Query the API
            var result = EveClient.APIProviders.CurrentProvider.QueryConquerableStationList();
            OnUpdated(result);
        }

        /// <summary>
        /// Processes the conquerable station list.
        /// </summary>
        private static void OnUpdated(APIResult<SerializableAPIConquerableStationList> result)
        {
            // Was there an error ?
            if (result.HasError)
            {
                EveClient.Notifications.NotifyConquerableStationListError(result);
                return;
            }

            EveClient.Notifications.InvalidateAPIError();

            // Deserialize the list
            Import(result.Result.Outposts);

            // Notify the subscribers
            EveClient.OnConquerableStationListUpdated();
        }
        #endregion

        
        #region Importation
        /// <summary>
        /// Deserialize the file and import the list.
        /// </summary>
        private static void Import()
        {
            // Exit if we have already imported the list
            if (s_loaded)
                return;

            var file = LocalXmlCache.GetFile(s_filename).FullName;

            // Abort if the file hasn't been obtained for any reason
            if (!File.Exists(file))
                return;

            var result = Util.DeserializeAPIResult<SerializableAPIConquerableStationList>(file, APIProvider.RowsetsTransform);

            // Checks if EVE Backend Database is temporarily disabled
            if (result.EVEBackendDatabaseDisabled)
                return;

            // In case the file has an error we prevent the deserialization
            if (result.HasError)
                return;

            // Deserialize the list
            Import(result.Result.Outposts);
        }

        /// <summary>
        /// Import the query result list.
        /// </summary>
        private static void Import(IEnumerable<SerializableOutpost> outposts)
        {
            EveClient.Trace("ConquerableStationList.Import() - begin");
            s_conqStationsByID.Clear();

            try
            {
                foreach (var outpost in outposts)
                {
                    s_conqStationsByID.Add(outpost.StationID, new ConquerableStation(outpost));
                }
            }
            catch (Exception exc)
            {
                ExceptionHandler.LogException(exc, true);
            }
            finally
            {
                s_loaded = true;
                EveClient.Trace("ConquerableStationList.Import() - done");
            }
        }
        #endregion


        /// <summary>
        /// Ensures the list has been imported.
        /// </summary>
        private static void EnsureImportation()
        {
            UpdateList();
            Import();
        }

        /// <summary>
        /// Gets the conquerable station with the provided ID.
        /// </summary>
        internal static ConquerableStation GetStation(long id)
        {
            // Ensure list importation
            EnsureImportation();

            ConquerableStation result = null;
            s_conqStationsByID.TryGetValue(id, out result);
            return result;
        }
    }
}
