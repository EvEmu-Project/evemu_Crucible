using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Serialization;

using EVEMon.Common.Net;
using EVEMon.Common.Threading;
using EVEMon.Common.Serialization.BattleClinic;


namespace EVEMon.Common
{
    /// <summary>
    /// Takes care of looking for new versions of EVEMon and its datafiles.
    /// </summary>
    public static class UpdateManager
    {
        private static bool s_checkScheduled = false;
        private static bool s_enabled;
        private static TimeSpan s_frequency = TimeSpan.FromMinutes(Settings.Updates.UpdateFrequency);

        /// <summary>
        /// Delete the installation files on a previous autoupdate.
        /// </summary>
        public static void DeleteInstallationFiles()
        {
            foreach (string file in Directory.GetFiles(EveClient.EVEMonDataDir, "EVEMon-install-*.exe", SearchOption.TopDirectoryOnly))
            {
                try
                {
                    File.Delete(file);
                }
                catch (Exception e)
                {
                    ExceptionHandler.LogException(e, false);
                }
            }
        }

        /// <summary>
        /// Gets or sets whether the autoupdater is enabled.
        /// </summary>
        public static bool Enabled
        {
            get { return s_enabled; }
            set 
            {
                s_enabled = value;

                if (!s_enabled)
                    return;

                if (s_checkScheduled)
                    return;

                // Schedule a check in 10 seconds
                ScheduleCheck(TimeSpan.FromSeconds(10));
            }
        }

        /// <summary>
        /// Schedules a check a specified time period in the future.
        /// </summary>
        /// <param name="time">Time period in the future to start check.</param>
        private static void ScheduleCheck(TimeSpan time)
        {
            s_checkScheduled = true;
            Dispatcher.Schedule(time, () => BeginCheck());
            EveClient.Trace("UpdateManager.ScheduleCheck() in {0}", time);
        }

        /// <summary>
        /// Performs the check asynchronously.
        /// </summary>
        /// <remarks>Invoked on the UI thread the first time and on some background thread the rest of the time.</remarks>
        /// <returns></returns>
        private static void BeginCheck()
        {
            // If update manager has been disabled since the last
            // update was triggered quit out here
            if (!s_enabled)
            {
                s_checkScheduled = false;
                return;
            }

            // No connection ? Recheck in one minute
            if (!NetworkMonitor.IsNetworkAvailable)
            {
                ScheduleCheck(TimeSpan.FromMinutes(1));
                return;
            }

            EveClient.Trace("UpdateManager.BeginCheck()");

            // Otherwise, query Batlleclinic
            Util.DownloadXMLAsync<SerializablePatch>(Settings.Updates.UpdatesUrl, null, OnCheckCompleted);
        }

        private static void OnCheckCompleted(SerializablePatch result, string errorMessage)
        {
            // If update manager has been disabled since the last
            // update was triggered quit out here
            if (!s_enabled)
            {
                s_checkScheduled = false;
                return;
            }

            // Was there an error ?
            if (!String.IsNullOrEmpty(errorMessage))
            {
                // Logs the error and reschedule
                EveClient.Trace("UpdateManager: {0}", errorMessage);
                ScheduleCheck(s_frequency);
                return;
            }

            // No error, let's try to deserialize
            try
            {
                ScanUpdateFeed(result);
            }
            // An error occurred during the deserialization
            catch (InvalidOperationException exc)
            {
                ExceptionHandler.LogException(exc, true);
            }
            finally
            {
                // Reschedule
                ScheduleCheck(s_frequency);
            }

            EveClient.Trace("UpdateManager.OnCheckCompleted()");
        }

        private static void ScanUpdateFeed(SerializablePatch result)
        {
            Version currentVersion = Assembly.GetExecutingAssembly().GetName().Version;
            Version newestVersion = new Version(result.Release.Version);
            string forumUrl = result.Release.TopicUrl;
            string updateMessage = result.Release.Message;
            string installArgs = result.Release.InstallerArgs;
            string installerUrl = result.Release.Url;
            string additionalArgs = result.Release.AdditionalArgs;
            bool canAutoInstall = (!String.IsNullOrEmpty(installerUrl) && !String.IsNullOrEmpty(installArgs));

            if (!String.IsNullOrEmpty(additionalArgs) && additionalArgs.Contains("%EVEMON_EXECUTABLE_PATH%"))
            {
                string appPath = Path.GetDirectoryName(Application.ExecutablePath);
                installArgs = String.Format(CultureConstants.DefaultCulture, "{0} {1}", installArgs, additionalArgs);
                installArgs = installArgs.Replace("%EVEMON_EXECUTABLE_PATH%", appPath);
            }

            // Is the program out of date ?
            if (newestVersion > currentVersion)
            {
                // Requests a notification to subscribers and quit
                EveClient.OnUpdateAvailable(forumUrl, installerUrl, updateMessage, currentVersion,
                                            newestVersion, canAutoInstall, installArgs);
                return;
            }

            if (result.FilesHaveChanged)
            {
                // Requests a notification to subscribers and quit
                EveClient.OnDataUpdateAvailable(forumUrl, result.ChangedDataFiles);
                return;
            }
        }
    }
}
