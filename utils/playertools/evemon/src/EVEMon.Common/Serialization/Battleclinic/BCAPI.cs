using System;
using System.Collections.Generic;
using System.Configuration;
using System.IO;
using System.Web;
using System.Windows.Forms;

using EVEMon.Common.Net;

namespace EVEMon.Common.Serialization.BattleClinic
{
    public static class BCAPI
    {
        private static List<BCAPIMethod> s_methods = new List<BCAPIMethod>(BCAPIMethod.CreateDefaultSet());
        private static string s_apiKey;
        private static bool m_queryPending;


        #region Properties

        /// <summary>
        /// Gets or sets a value indicating whether the BattleClinic API credentials are authenticated.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if the BattleClinic API credentials are authenticated; otherwise, <c>false</c>.
        /// </value>
        public static bool IsAuthenticated { get; private set; }

        /// <summary>
        /// Gets a value indicating whether the BattleClinic API credentials are stored.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if the BattleClinic API credentials are stored; otherwise, <c>false</c>.
        /// </value>
        public static bool HasCredentialsStored
        {
            get
            {
                return BCAPISettings.Default.BCUserID != 0
                    && !String.IsNullOrEmpty(BCAPISettings.Default.BCAPIKey);
            }
        }

        /// <summary>
        /// Gets the content of the settings file.
        /// </summary>
        /// <value>The content of the settings file.</value>
        private static string SettingsFileContent
        {
            get
            {
                var settingsFileContent = File.ReadAllText(EveClient.SettingsFileNameFullPath);
                return HttpUtility.UrlEncode(settingsFileContent);
            }
        }

        #endregion


        #region Helper Methods

        /// <summary>
        /// Upgrades the settings.
        /// </summary>
        public static void UpgradeSettings()
        {
            // Find the settings file
            Configuration settings = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.PerUserRoamingAndLocal);

            // Quit if the settings file is of the current version
            if (settings.HasFile)
                return;

            // Find the parent directories of the settings file
            string configFileParentDir = Directory.GetParent(settings.FilePath).FullName;
            string configFileParentParentDir = Directory.GetParent(configFileParentDir).FullName;

            // Quits if the parent directory doesn't exist
            if (!Directory.Exists(configFileParentParentDir))
                return;

            // Upgrade the settings file to the current version
            BCAPISettings.Default.Upgrade();

            // Delete all old settings files
            foreach (string directory in Directory.GetDirectories(configFileParentParentDir))
            {
                if (directory == configFileParentDir)
                    continue;

                // Delete the folder recursively
                Directory.Delete(directory, true);                        
            }
        }
        

        /// <summary>
        /// Returns the request method.
        /// </summary>
        /// <param name="requestMethod">A BCAPIMethods enumeration member specfying the method for which the URL is required.</param>
        public static BCAPIMethod GetMethod(BCAPIMethods requestMethod)
        {
            foreach (BCAPIMethod method in s_methods)
            {
                if (method.Method == requestMethod)
                    return method;
            }

            throw new InvalidOperationException();
        }

        /// <summary>
        /// Returns the full canonical URL for the specified BCAPIMethod as constructed from the Server and BCAPIMethod properties.
        /// </summary>
        /// <param name="requestMethod">A BCAPIMethods enumeration member specfying the method for which the URL is required.</param>
        /// <returns>A String representing the full URL path of the specified method.</returns>
        public static string GetMethodUrl(BCAPIMethods requestMethod)
        {
            // Gets the proper data
            var url = NetworkConstants.BCAPIBase;
            var path = GetMethod(requestMethod).Path;

            // Build the uri
            var baseUri = new Uri(url);
            var uriBuilder = new UriBuilder(baseUri);
            uriBuilder.Path = uriBuilder.Path.TrimEnd("/".ToCharArray()) + path;
            return uriBuilder.Uri.ToString();
        }

        /// <summary>
        /// Checks the BattleClinic API credentials.
        /// </summary>
        /// <param name="userID">The user ID.</param>
        /// <param name="apiKey">The API key.</param>
        public static void CheckAPICredentials(uint userID, string apiKey)
        {
            if (m_queryPending)
                return;

            m_queryPending = true;
            IsAuthenticated = false;

            s_apiKey = apiKey;
            CheckAPICredentialsAsync(userID, apiKey, OnCredentialsQueried);
        }

        /// <summary>
        /// Uploads the settings file.
        /// </summary>
        public static void UploadSettingsFile()
        {
            if (!BCAPISettings.Default.UploadAlways || !HasCredentialsStored)
                return;

            FileSave();
            EveClient.Trace("BCAPI.UploadSettingsFile() - Done");
        }

        /// <summary>
        /// Downloads the settings file.
        /// </summary>
        public static void DownloadSettingsFile()
        {
            if (!BCAPISettings.Default.DownloadAlways || !HasCredentialsStored)
                return;

            EveClient.Trace("BCAPI.DownloadSettingsFile() - Initiated");

            FileGetByNameAsync(OnFileGetByName);
        }

        #endregion


        #region Queries

        /// <summary>
        /// Saves a file content to the BattleClinic server.
        /// </summary>
        public static void FileSave()
        {
            HttpPostData postData = new HttpPostData(String.Format("userID={0}&apiKey={1}&applicationKey={2}&id=0&name={3}&content={4}",
               BCAPISettings.Default.BCUserID, BCAPISettings.Default.BCAPIKey, BCAPISettings.Default.BCApplicationKey,
               EveClient.SettingsFileName, SettingsFileContent));

            QueryMethod(BCAPIMethods.FileSave, postData);
        }

        /// <summary>
        /// Gets the file content by the file name.
        /// </summary>
        public static void FileGetByName()
        {
            HttpPostData postData = new HttpPostData(String.Format("userID={0}&apiKey={1}&applicationKey={2}&fileName={3}",
                BCAPISettings.Default.BCUserID, BCAPISettings.Default.BCAPIKey,
                BCAPISettings.Default.BCApplicationKey, EveClient.SettingsFileName));

            QueryMethod(BCAPIMethods.FileGetByName, postData);
        }

        /// <summary>
        /// Asyncronously checks the BattleClinic API credentials.
        /// </summary>
        /// <param name="userID">The user ID.</param>
        /// <param name="apiKey">The API key.</param>
        /// <param name="callback">The callback.</param>
        public static void CheckAPICredentialsAsync(uint userID, string apiKey, DownloadCallback<BCAPIResult<SerializableBCAPICredentials>> callback)
        {
            HttpPostData postData = new HttpPostData(String.Format("userID={0}&apiKey={1}&applicationKey={2}",
                userID, apiKey, BCAPISettings.Default.BCApplicationKey));

            QueryMethodAsync<SerializableBCAPICredentials>(BCAPIMethods.CheckCredentials, postData, callback);
        }

        /// <summary>
        /// Asyncronously saves a file content to the BattleClinic server.
        /// </summary>
        /// <param name="callback">The callback.</param>
        public static void FileSaveAsync(DownloadCallback<BCAPIResult<SerializableBCAPIFiles>> callback)
        {
            HttpPostData postData = new HttpPostData(String.Format("userID={0}&apiKey={1}&applicationKey={2}&id=0&name={3}&content={4}",
                BCAPISettings.Default.BCUserID, BCAPISettings.Default.BCAPIKey, BCAPISettings.Default.BCApplicationKey,
                EveClient.SettingsFileName, SettingsFileContent));

            QueryMethodAsync<SerializableBCAPIFiles>(BCAPIMethods.FileSave, postData, callback);
        }

        /// <summary>
        /// Asyncronously gets the file content by the file name.
        /// </summary>
        /// <param name="callback">The callback.</param>
        public static void FileGetByNameAsync(DownloadCallback<BCAPIResult<SerializableBCAPIFiles>> callback)
        {
            HttpPostData postData = new HttpPostData(String.Format("userID={0}&apiKey={1}&applicationKey={2}&fileName={3}",
                BCAPISettings.Default.BCUserID, BCAPISettings.Default.BCAPIKey,
                BCAPISettings.Default.BCApplicationKey, EveClient.SettingsFileName));

            QueryMethodAsync<SerializableBCAPIFiles>(BCAPIMethods.FileGetByName, postData, callback);
        }

        #endregion


        #region Querying helpers

        /// <summary>
        /// Query this method with the provided HTTP POST data.
        /// </summary>
        /// <param name="method">The method.</param>
        /// <param name="postData">The post data.</param>
        private static void QueryMethod(BCAPIMethods method, HttpPostData postData)
        {
            string url = GetMethodUrl(method);
            EveClient.HttpWebService.DownloadXml(url, postData);
        }

        /// <summary>
        /// Asynchrnoneously queries this method with the provided HTTP POST data.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="method">The method.</param>
        /// <param name="postData">The post data.</param>
        /// <param name="callback">The callback.</param>
        private static void QueryMethodAsync<T>(BCAPIMethods method, HttpPostData postData, DownloadCallback<BCAPIResult<T>> callback)
        {
            // Check callback not null
            if (callback == null)
                throw new ArgumentNullException("The callback cannot be null.", "callback");

            string url = GetMethodUrl(method);
            Util.DownloadXMLAsync<BCAPIResult<T>>(url, postData, callback);
        }

        /// <summary>
        /// Occurs when the BattleClinic credentials get queried.
        /// </summary>
        /// <param name="result">The result.</param>
        /// <param name="errorMessage">The error message.</param>
        private static void OnCredentialsQueried(BCAPIResult<SerializableBCAPICredentials> result, string errorMessage)
        {
            m_queryPending = false;

            if (!String.IsNullOrEmpty(errorMessage))
            {
                EveClient.OnBCAPICredentialsUpdated(errorMessage);
                return;
            }

            if (result.HasError)
            {
                EveClient.OnBCAPICredentialsUpdated(result.Error.ErrorMessage);
                return;
            }

            IsAuthenticated = true;

            BCAPISettings.Default.BCUserID = Convert.ToUInt32(result.Result.UserID);
            BCAPISettings.Default.BCAPIKey = s_apiKey;
            BCAPISettings.Default.Save();

            EveClient.OnBCAPICredentialsUpdated(String.Empty);
        }

        /// <summary>
        /// Occurs when the FileGetByName get queried.
        /// </summary>
        /// <param name="result">The result.</param>
        /// <param name="errorMessage">The error message.</param>
        private static void OnFileGetByName(BCAPIResult<SerializableBCAPIFiles> result, string errorMessage)
        {
            if (!String.IsNullOrEmpty(errorMessage))
            {
                MessageBox.Show(errorMessage, "Network Error", MessageBoxButtons.OK);
                return;
            }

            if (result.HasError)
            {
                MessageBox.Show(result.Error.ErrorMessage, "BattleClinic API Error", MessageBoxButtons.OK);
                return;
            }

            EveClient.Trace("BCAPI.DownloadSettingsFile() - Completed");

            SaveSettingsFile(result.Result.Files[0]);
        }

        /// <summary>
        /// Saves the queried settings file.
        /// </summary>
        /// <param name="settingsFile">The settings file.</param>
        public static void SaveSettingsFile(SerializableFilesListItem settingsFile)
        {
            using (var saveFileDialog = new SaveFileDialog())
            {
                saveFileDialog.Title = "EVEMon Settings Backup File Save";
                saveFileDialog.DefaultExt = "xml";
                saveFileDialog.Filter = "EVEMon Settings Backup Files (.bak)|*.bak";
                saveFileDialog.FilterIndex = 1;

                // Save current directory
                var currentDirectory = Directory.GetCurrentDirectory();

                // Prompts the user for a location
                saveFileDialog.FileName = String.Format("{0}.bak", settingsFile.FileName);
                saveFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Personal).ToString();
                var result = saveFileDialog.ShowDialog();

                // Restore current directory
                Directory.SetCurrentDirectory(currentDirectory);

                // Save settings file if OK
                if (result != DialogResult.OK)
                    return;

                // Save the file to destination
                File.WriteAllText(saveFileDialog.FileName, settingsFile.FileContent);
            }
        }

        #endregion

    }
}
