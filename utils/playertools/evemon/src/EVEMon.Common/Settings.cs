using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.Threading;
using System.IO;
using EVEMon.Common.Threading;
using System.Xml.Serialization;
using System.Reflection;
using EVEMon.Common.Serialization;
using System.Windows.Forms;

using EVEMon.Common.Attributes;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common.Scheduling;
using EVEMon.Common.Serialization.Importation;
using EVEMon.Common.Notifications;

namespace EVEMon.Common
{
    /// <summary>
    /// The settings class is bound 
    /// </summary>
    [EnforceUIThreadAffinity]
    public static partial class Settings
    {
        /// <summary>
        /// Static constructor
        /// </summary>
        static Settings()
        {
            UI = new UISettings();
            G15 = new G15Settings();
            IGB = new IGBSettings();
            Proxy = new ProxySettings(); 
            Updates = new UpdateSettings();
            Calendar = new CalendarSettings();
            Exportation = new ExportationSettings();
            Notifications = new NotificationSettings();
        }


        #region Serialization - Core - Methods to update to add a property
        /// <summary>
        /// Creates new empty Settings file, overwriting the existing file
        /// </summary>
        public static void Reset()
        {
            // Append new properties here
            Import(new SerializableSettings(), false);

            // Notifies the client and save
            SaveImmediate();
        }

        /// <summary>
        /// Updates from the provided serialization object
        /// </summary>
        /// <param name="serial">The serializable version of the new settings. May be null (acts as a reset)</param>
        /// <param name="preferencesOnly">When true, only the user preferences will be reimported, not plans, characters, accounts and such.</param>
        public static void Import(SerializableSettings serial, bool preferencesOnly)
        {
            // When null, we just reset
            if (serial == null)
            {
                Reset();
                return;
            }

            IsRestoringSettings = true;
            try
            {
                EveClient.Trace("Settings.Import() - begin");

                // Import the characters, accounts and plans
                if (!preferencesOnly)
                {
                    // The above check prevents the settings form to trigger a 
                    // characters updates since the last queried infos would be lost.
                    EveClient.Characters.Import(serial.Characters);
                    EveClient.Characters.ImportPlans(serial.Plans);
                    EveClient.MonitoredCharacters.Import(serial.MonitoredCharacters);
                    EveClient.Accounts.Import(serial.Accounts);
                }

                // Global settings
                Settings.Compatibility = serial.Compatibility;

                // API providers
                EveClient.APIProviders.Import(serial.APIProviders);

                // Scheduler
                Scheduler.Import(serial.Scheduler);

                // User settings
                Settings.UI = serial.UI.Clone();
                Settings.G15 = serial.G15.Clone();
                Settings.IGB = serial.IGB.Clone();
                Settings.Proxy = serial.Proxy.Clone();
                Settings.Updates = serial.Updates.Clone();
                Settings.Notifications = serial.Notifications.Clone();
                Settings.Exportation = serial.Exportation.Clone();
                Settings.Calendar = serial.Calendar.Clone();

                // Trim the data
                OnImportCompleted();

                // Save
                SaveImmediate();

                // Updates the data right now
                EveClient.UpdateOnOneSecondTick();
                EveClient.OnSettingsChanged();

                EveClient.Trace("Settings.Import() - done");
            }
            finally
            {
                IsRestoringSettings = false;
            }
        }

        /// <summary>
        /// Corrects the imported data and add missing stuff.
        /// </summary>
        private static void OnImportCompleted()
        {
            // Add missing notification behaviours
            foreach (NotificationCategory cat in Enum.GetValues(typeof(NotificationCategory)))
            {
                if (!Settings.Notifications.Categories.ContainsKey(cat))
                    Settings.Notifications.Categories[cat] = new NotificationCategorySettings();
            }

            // Add missing API methods update periods
            foreach(APIMethods method in Enum.GetValues(typeof(APIMethods)))
            {
                if (!Settings.Updates.Periods.ContainsKey(method))
                {
                    var updateAttribute = method.GetAttribute<UpdateAttribute>();
                    if (updateAttribute != null)
                        Settings.Updates.Periods.Add(method, updateAttribute.DefaultPeriod);
                }
            }

            // Add missing plan order columns
            var planColumns = UI.PlanWindow.Columns.ToList();
            planColumns.AddRange(EnumExtensions.GetValues<PlanColumn>().
                Where(x => x != PlanColumn.None && planColumns.All(y => y.Column != x)).
                Select(x => new PlanColumnSettings { Column = x, Visible = false, Width = -1 }).ToArray());
            UI.PlanWindow.Columns = planColumns.ToArray();

            // Add missing market order columns
            var ordersColumns = UI.MainWindow.MarketOrders.Columns.ToList();
            ordersColumns.AddRange(EnumExtensions.GetValues<MarketOrderColumn>().
                Where(x => x != MarketOrderColumn.None && ordersColumns.All(y => y.Column != x)).
                Select(x => new MarketOrderColumnSettings { Column = x, Visible = false, Width = -1 }).ToArray());
            UI.MainWindow.MarketOrders.Columns = ordersColumns.ToArray();

            // Add missing industry jobs columns
            var jobsColumns = UI.MainWindow.IndustryJobs.Columns.ToList();
            jobsColumns.AddRange(EnumExtensions.GetValues<IndustryJobColumn>().
                Where(x => x != IndustryJobColumn.None && jobsColumns.All(y => y.Column != x)).
                Select(x => new IndustryJobColumnSettings { Column = x, Visible = false, Width = -1 }).ToArray());
            UI.MainWindow.IndustryJobs.Columns = jobsColumns.ToArray();

            // Add missing research points columns
            var researchColumns = UI.MainWindow.Research.Columns.ToList();
            researchColumns.AddRange(EnumExtensions.GetValues<ResearchColumn>().
                Where(x => x != ResearchColumn.None && researchColumns.All(y => y.Column != x)).
                Select(x => new ResearchColumnSettings { Column = x, Visible = false, Width = -1 }).ToArray());
            UI.MainWindow.Research.Columns = researchColumns.ToArray();

            // Add missing EVE mail messages columns
            var eveMailMessagesColumns = UI.MainWindow.EVEMailMessages.Columns.ToList();
            eveMailMessagesColumns.AddRange(EnumExtensions.GetValues<EveMailMessagesColumn>().
                Where(x => x != EveMailMessagesColumn.None && eveMailMessagesColumns.All(y => y.Column != x)).
                Select(x => new EveMailMessagesColumnSettings { Column = x, Visible = false, Width = -1 }).ToArray());
            UI.MainWindow.EVEMailMessages.Columns = eveMailMessagesColumns.ToArray();

            // Add missing EVE notifications columns
            var eveNotificationsColumns = UI.MainWindow.EVENotifications.Columns.ToList();
            eveNotificationsColumns.AddRange(EnumExtensions.GetValues<EveNotificationsColumn>().
                Where(x => x != EveNotificationsColumn.None && eveNotificationsColumns.All(y => y.Column != x)).
                Select(x => new EveNotificationsColumnSettings { Column = x, Visible = false, Width = -1 }).ToArray());
            UI.MainWindow.EVENotifications.Columns = eveNotificationsColumns.ToArray();
        }

        /// <summary>
        /// Creates a seriablizable version of the settings.
        /// </summary>
        /// <returns></returns>
        public static SerializableSettings Export()
        {
            SerializableSettings serial = new SerializableSettings();

            serial.Revision = Settings.Revision;
            serial.Compatibility = Settings.Compatibility;

            // Export characters and such
            serial.Characters = EveClient.Characters.Export();
            serial.Accounts = EveClient.Accounts.Export();
            serial.Plans = EveClient.Characters.ExportPlans();
            serial.MonitoredCharacters = EveClient.MonitoredCharacters.Export();

            // API providers
            serial.APIProviders = EveClient.APIProviders.Export();

            // Scheduler
            serial.Scheduler = Scheduler.Export();

            // User settings
            serial.Calendar = Settings.Calendar.Clone();
            serial.Notifications = Settings.Notifications.Clone();
            serial.Exportation = Settings.Exportation.Clone();
            serial.Updates = Settings.Updates.Clone();
            serial.Proxy = Settings.Proxy.Clone();
            serial.IGB = Settings.IGB.Clone();
            serial.G15 = Settings.G15.Clone();
            serial.UI = Settings.UI.Clone();

            return serial;
        }
        #endregion


        #region Initialization and loading
        /// <summary>
        /// Gets the current assembly's revision, which is also used for files versioning.
        /// </summary>
        internal static int Revision
        {
            get { return Assembly.GetExecutingAssembly().GetName().Version.Revision; }
        }

        /// <summary>
        /// Initialization for the EVEMon client. Will automatically load the settings file.
        /// </summary>
        /// <param name="applicationForm">The main application form</param>
        /// <exception cref="InvalidOperationException">The instance has been initialized already</exception>
        public static void InitializeFromFile()
        {
            // Creates the settings from the file
            SerializableSettings settings = TryDeserializeSettings();

            // Loading from file failed, we create settings from scratch
            if (settings == null)
            {
                Reset();
            }
            else
            {
                Import(settings, false);
            }
        }

        /// <summary>
        /// Loads a settings file from a specified filepath and sets m_instance.
        /// </summary>
        /// <param name="filename">The fully qualified filename of the settings file to load</param>
        /// <returns>The Settings object loaded</returns>
        public static void Restore(string filename)
        {
            // Try deserialize
            string settingsFile = EveClient.SettingsFileNameFullPath;
            SerializableSettings settings = TryDeserializeBackup(filename, settingsFile, false);

            // Loading from file failed, we abort and keep our current settings
            if (settings == null)
            {
                MessageBox.Show(String.Format(CultureConstants.DefaultCulture, "Cannot restore the settings from {0}, the file is corrupted.", filename), "Bad settings file.", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            // Updates and save
            Import(settings, false);
        }

        /// <summary>
        /// loads the settings file, or the backup, prompting the user for errors.
        /// </summary>
        /// <returns><c>Null</c> if we have been unable to load anything from files, the generated settings otherwise</returns>
        private static SerializableSettings TryDeserializeSettings()
        {
            SerializableSettings settings = null;
            string settingsFile = EveClient.SettingsFileNameFullPath;
            string backupFile = settingsFile + ".bak";

            // Check that a settings file or backup exists
            if (File.Exists(settingsFile))
            {
                EveClient.Trace("Settings.TryDeserializeSettings() - begin");

                // Check settings file length
                FileInfo settingsInfo = new FileInfo(settingsFile);
                if (settingsInfo.Length > 0)
                {
                    // Gets the revision number of the assembly which generated this file
                    int revision = Util.GetRevisionNumber(settingsFile);

                    // Try to load from a file
                    if (revision == 0)
                    {
                        // Old format
                        settings = DeserializeOldFormat(settingsFile);
                    }
                    else
                    {
                        // New format
                        settings = Util.DeserializeXML<SerializableSettings>(settingsFile);
                    }

                    // If the settings loaded OK, make a backup as 'last good settings' and return
                    if (settings != null)
                    {
                        CheckSettingsVersion(settings);
                        FileHelper.OverwriteOrWarnTheUser(settingsFile, backupFile);
                        EveClient.Trace("Settings.TryDeserializeSettings() - done");
                        return settings;
                    }
                }
            }

            // Try to recover from the backup
            return TryDeserializeBackup(backupFile, settingsFile, true);
        }

        /// <summary>
        /// Try to deserialize from the backup file.
        /// </summary>
        /// <param name="backupFile"></param>
        /// <returns></returns>
        private static SerializableSettings TryDeserializeBackup(string backupFile, string settingsFile, bool recover)
        {
            SerializableSettings settings = null;

            // Load failed, so check for backup
            if (File.Exists(backupFile))
            {
                EveClient.Trace("Settings.TryDeserializeBackup() - begin");

                FileInfo backupInfo = new FileInfo(backupFile);
                if (backupInfo.Length > 0)
                {
                    if (recover)
                    {
                        // Prompts the user to use the backup
                        String fileDate = String.Format("{0} at {1}",
                                                    backupInfo.LastWriteTime.ToLocalTime().ToShortDateString(),
                                                    backupInfo.LastWriteTime.ToLocalTime().ToCustomShortTimeString());
                        DialogResult dr = MessageBox.Show(String.Format(CultureConstants.DefaultCulture,
                            "Your settings file is missing or corrupt. There is a backup available from {0}. Do you want to use the backup file?", fileDate),
                            "Corrupt Settings", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation);

                        if (dr == DialogResult.No)
                        {
                            MessageBox.Show("Your settings file is corrupt, and no backup is available. A new settings file will be created."
                                            + " You may wish to close down EVEMon and restore a saved copy of your file.", "Corrupt Settings",
                                            MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                            return null;
                        }
                    }
                    // Gets the revision number of the assembly which generated this file
                    int revision = Util.GetRevisionNumber(backupFile);

                    // Try to load from a backup file
                    if (revision == 0)
                    {
                        // Old format
                        settings = DeserializeOldFormat(backupFile);
                    }
                    else
                    {
                        // New format
                        settings = Util.DeserializeXML<SerializableSettings>(backupFile);
                    }

                    // If the settings loaded OK, copy to the main settings file, then copy back to stamp date
                    if (settings != null)
                    {
                        CheckSettingsVersion(settings);
                        FileHelper.OverwriteOrWarnTheUser(backupFile, settingsFile);
                        FileHelper.OverwriteOrWarnTheUser(settingsFile, backupFile);
                        EveClient.Trace("Settings.TryDeserializeBackup() - done");
                        return settings;
                    }

                    // Backup failed too, notify the user we have a problem
                    MessageBox.Show("Load from backup failed. A new settings file will be created."
                                    + " You may wish to close down EVEMon and restore a saved copy of your file.",
                                    "Corrupt Settings", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return null;
                }
            }

            return null;
        }

        /// <summary>
        /// Deserializes a settings file from an old format.
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        private static SerializableSettings DeserializeOldFormat(string filename)
        {
            var oldSerial = Util.DeserializeXML<OldSettings>(filename, Util.LoadXSLT(Properties.Resources.SettingsAndPlanImport));
            var serial = new SerializableSettings();

            // Accounts
            serial.Accounts.AddRange(oldSerial.Accounts);

            // Characters
            foreach (var oldCharacter in oldSerial.Characters)
            {
                // Adds the char both to the characters list and the monitored characters list.
                var character = new SerializableCCPCharacter {
                    ID = oldCharacter.ID, 
                    Name = oldCharacter.Name, 
                    Guid = Guid.NewGuid()
                };
                serial.MonitoredCharacters.Add(new MonitoredCharacterSettings { CharacterGuid = character.Guid });
                serial.Characters.Add(character);
            }

            // Plans
            foreach (var oldPlan in oldSerial.Plans)
            {
                // Look for the owner by his name
                var owner = serial.Characters.SingleOrDefault(x => x.Name == oldPlan.Owner);
                if (owner == null)
                    continue;

                // Imports the plan
                var plan = new SerializablePlan { Owner = owner.Guid, Name = oldPlan.Name };
                plan.Entries.AddRange(oldPlan.Entries);
                serial.Plans.Add(plan);
            }

            return serial;
        }

        /// <summary>
        /// Compare the settings version with this version and, when different, update and prompt the user for a backup.
        /// </summary>
        /// <param name="notifyUser"></param>
        private static void CheckSettingsVersion(SerializableSettings settings)
        {
#if !DEBUG
            int revision = Assembly.GetExecutingAssembly().GetName().Version.Revision;
            if (revision != settings.Revision)
            {
                DialogResult backupSettings =
                    MessageBox.Show("The current EVEMon settings file is from a previous version of EVEMon. Backup the current file before proceeding (recommended)?",
                        "EVEMon version changed", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1);

                if (backupSettings == DialogResult.Yes)
                {
                    using (SaveFileDialog fileDialog = new SaveFileDialog())
                    {
                        fileDialog.Title = "Settings file backup";
                        fileDialog.Filter = "Settings Backup Files (*.bak)|*.bak";
                        fileDialog.FileName = String.Format(CultureConstants.DefaultCulture, "EVEMon_Settings_{0}.xml.bak", revision.ToString());
                        fileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
                        DialogResult saveFile = fileDialog.ShowDialog();
                        if (saveFile != DialogResult.OK)
                            return;

                        FileHelper.OverwriteOrWarnTheUser(EveClient.SettingsFileNameFullPath, fileDialog.FileName);
                    }
                }
            }
#endif
        }
        #endregion


        #region Save

        /// <summary>
        /// Flag to indicate if a save is pending but not committed.
        /// </summary>
        private static bool m_savePending = false;

        /// <summary>
        /// The last time the settings were saved.
        /// </summary>
        private static DateTime m_lastSaveTime = DateTime.MinValue;

        /// <summary>
        /// Every 10s, the timer ticks and check whether we should save the settings.
        /// </summary>
        internal static void UpdateOnOneSecondTick()
        {
            // Is a save requested and is the last save older than 10s ?
            if (m_savePending && DateTime.UtcNow > m_lastSaveTime.AddSeconds(10))
                SaveImmediate();
        }

        /// <summary>
        /// Saves settings to disk.
        /// </summary>
        /// <remarks>
        /// Saves will be cached for 10 seconds to avoid thrashing the disk when this method is called very rapidly
        /// such as at startup. If a save is currently pending, no action is needed. 
        /// </remarks>
        public static void Save()
        {
            m_savePending = true;
        }

        /// <summary>
        /// Saves settings immediately.
        /// </summary>
        public static void SaveImmediate()
        {
            SerializableSettings settings = Export();
            XmlSerializer xs = new XmlSerializer(typeof(SerializableSettings));

            // Save in settings file
            FileHelper.OverwriteOrWarnTheUser(EveClient.SettingsFileNameFullPath, fs =>
            {
                xs.Serialize(fs, settings);
                fs.Flush();
                return true;
            });

            // Reset savePending flag
            m_lastSaveTime = DateTime.UtcNow;
            m_savePending = false;
        }

        /// <summary>
        /// Copies the current Settings file to the specified location.
        /// </summary>
        /// <param name="copyFileName">The fully qualified filename of the destination file</param>
        public static void CopySettings(string copyFileName)
        {
            // Ensure we have the latest settings saved
            SaveImmediate();
            FileHelper.OverwriteOrWarnTheUser(EveClient.SettingsFileNameFullPath, copyFileName);
        }

        /// <summary>
        /// Gets true if we're currently restoring the settings.
        /// </summary>
        public static bool IsRestoringSettings
        {
            get;
            private set;
        }
        
        #endregion


        #region The very settings
        /// <summary>
        /// Gets or sets the compatibility mode.
        /// </summary>
        public static CompatibilityMode Compatibility
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the settings for updates.
        /// </summary>
        public static UpdateSettings Updates
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the settings for UI (look'n feel)
        /// </summary>
        public static UISettings UI
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the settings for the G15 keyboard.
        /// </summary>
        public static G15Settings G15
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the settings for the notifications (alerts).
        /// </summary>
        public static NotificationSettings Notifications
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the settings for the network.
        /// </summary>
        public static IGBSettings IGB
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the calendar settings.
        /// </summary>
        public static CalendarSettings Calendar
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the exportation settings.
        /// </summary>
        public static ExportationSettings Exportation
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the custom proxy settings.
        /// </summary>
        public static ProxySettings Proxy
        {
            get;
            set;
        }

        #endregion
    }
}
