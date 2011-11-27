using System;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Security;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.Resources.Skill_Select;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.SettingsObjects;
using EVEMon.Controls;
using Microsoft.Win32;

namespace EVEMon.SettingsUI
{
    public partial class SettingsForm : EVEMonForm
    {
        private const string StartupRegistryKey = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Run";

        private SerializableSettings m_settings;
        private SerializableSettings m_oldSettings;
        private bool m_isLoading;

        /// <summary>
        /// Constructor.
        /// </summary>
        public SettingsForm()
        {
            InitializeComponent();
            m_settings = Settings.Export();
            m_oldSettings = Settings.Export();

            // Platform is Unix ?
            if (Environment.OSVersion.Platform == PlatformID.Unix)
            {
                runAtStartupComboBox.Enabled = false;
                treeView.Nodes["trayIconNode"].Remove();
            }

            // Run with Mono ?
            if (Type.GetType("Mono.Runtime") != null)
                treeView.Nodes["generalNode"].Nodes["g15Node"].Remove();

            // Fill the overview portraits sizes
            overviewPortraitSizeComboBox.Items.AddRange(
                Enum.GetValues(typeof(PortraitSizes)).Cast<PortraitSizes>().Select(x =>
                {
                    // Transforms x64 to 64 by 64
                    var size = x.ToString().Substring(1);
                    return String.Format("{0} by {0}", size);
                }).ToArray());

            // Expands the left panel and selects the first page and node.
            treeView.ExpandAll();
            treeView.SelectedNode = treeView.Nodes[0];
            multiPanel.SelectedPage = generalPage;
        }

        #region Core methods
        /// <summary>
        /// Occurs when the user click "Cancel".
        /// We restore the old settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnCancel_Click(object sender, EventArgs e)
        {
            // Update settings
            Settings.Import(m_oldSettings, true);
            Settings.Save();

            // Close
            DialogResult = DialogResult.Cancel;
            Close();
        }

        /// <summary>
        /// Occurs when the user click "OK".
        /// We set up the new settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnOk_Click(object sender, EventArgs e)
        {
            // Return settings
            ApplyToSettings();
            Settings.Import(m_settings, true);
            Settings.Save();

            // Close
            DialogResult = DialogResult.OK;
            Close();
        }

        /// <summary>
        /// Occurs when the user click "Apply".
        /// We set up the new settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void applyButton_Click(object sender, EventArgs e)
        {
            // Return settings
            ApplyToSettings();
            Settings.Import(m_settings, true);
            Settings.Save();
        }

        /// <summary>
        /// Occurs on form load, we update the controls values with the settings we retrieved.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void SettingsForm_Load(object sender, EventArgs e)
        {
            m_isLoading = true;

            // Misc settings
            cbWorksafeMode.Checked = m_settings.UI.SafeForWork;
            compatibilityCombo.SelectedIndex = (int)m_settings.Compatibility;

            // Skills icon sets
            cbSkillIconSet.Items.Clear();
            for (int i = 1; i < IconSettings.Default.Properties.Count; i++)
            {
                cbSkillIconSet.Items.Add(IconSettings.Default.Properties["Group" + i].DefaultValue.ToString());
            }

            // Window settings
            rbSystemTrayOptionsNever.Checked = (m_settings.UI.SystemTrayIcon == SystemTrayBehaviour.Disabled);
            rbSystemTrayOptionsAlways.Checked = (m_settings.UI.SystemTrayIcon == SystemTrayBehaviour.AlwaysVisible);
            rbSystemTrayOptionsMinimized.Checked = (m_settings.UI.SystemTrayIcon == SystemTrayBehaviour.ShowWhenMinimized);

            if (m_settings.UI.MainWindowCloseBehaviour == CloseBehaviour.MinimizeToTaskbar)
            {
                rbMinToTaskBar.Checked = true;
            }
            else if (m_settings.UI.MainWindowCloseBehaviour == CloseBehaviour.MinimizeToTray)
            {
                rbMinToTray.Checked = true;
            }
            else
            {
                rbExitEVEMon.Checked = true;
            }

            // G15
            g15CheckBox.Checked = m_settings.G15.Enabled;
            cbG15ACycle.Checked = m_settings.G15.UseCharactersCycle;
            ACycleInterval.Value = m_settings.G15.CharactersCycleInterval;
            cbG15CycleTimes.Checked = m_settings.G15.UseTimeFormatsCycle;
            ACycleTimesInterval.Value = Math.Min(m_settings.G15.TimeFormatsCycleInterval, ACycleTimesInterval.Maximum);
            cbG15ShowTime.Checked = m_settings.G15.ShowSystemTime;
            cbG15ShowEVETime.Checked = m_settings.G15.ShowEVETime;

            // Skills display on the main window
            cbShowAllPublicSkills.Checked = m_settings.UI.MainWindow.ShowAllPublicSkills;
            cbShowNonPublicSkills.Checked = m_settings.UI.MainWindow.ShowNonPublicSkills;

            // Main window
            cbTitleToTime.Checked = m_settings.UI.MainWindow.ShowCharacterInfoInTitleBar;
            cbWindowsTitleList.SelectedIndex = (int)m_settings.UI.MainWindow.TitleFormat - 1;
            cbSkillInTitle.Checked = m_settings.UI.MainWindow.ShowSkillNameInWindowTitle;
            cbShowPrereqMetSkills.Checked = m_settings.UI.MainWindow.ShowPrereqMetSkills;
            cbColorPartialSkills.Checked = m_settings.UI.MainWindow.HighlightPartialSkills;
            cbColorQueuedSkills.Checked = m_settings.UI.MainWindow.HighlightQueuedSkills;
            cbAlwaysShowSkillQueueTime.Checked = m_settings.UI.MainWindow.AlwaysShowSkillQueueTime;

            // Main Window - Overview
            cbShowOverViewTab.Checked = m_settings.UI.MainWindow.ShowOverview;
            cbUseIncreasedContrastOnOverview.Checked = m_settings.UI.MainWindow.UseIncreasedContrastOnOverview;
            overviewShowWalletCheckBox.Checked = m_settings.UI.MainWindow.ShowOverviewWallet;
            overviewShowPortraitCheckBox.Checked = m_settings.UI.MainWindow.ShowOverviewPortrait;
            overviewPortraitSizeComboBox.SelectedIndex = (int)m_settings.UI.MainWindow.OverviewItemSize;
            overviewShowSkillQueueTrainingTimeCheckBox.Checked = m_settings.UI.MainWindow.ShowOverviewSkillQueueTrainingTime;
            overviewGroupCharactersInTrainingCheckBox.Checked = m_settings.UI.MainWindow.PutTrainingSkillsFirstOnOverview;

            // IGB Server
            igbCheckBox.Checked = m_settings.IGB.IGBServerEnabled;
            cbIGBPublic.Checked = m_settings.IGB.IGBServerPublic;
            igbPortTextBox.Text = m_settings.IGB.IGBServerPort.ToString();

            // Notifications
            notificationsControl.Settings = m_settings.Notifications;
            cbPlaySoundOnSkillComplete.Checked = m_settings.Notifications.PlaySoundOnSkillCompletion;

            // Email Notifications
            mailNotificationCheckBox.Checked = m_settings.Notifications.SendMailAlert;
            tbMailServer.Text = m_settings.Notifications.EmailSmtpServer;
            emailPortTextBox.Text = m_settings.Notifications.EmailPortNumber.ToString();
            cbEmailServerRequireSsl.Checked = m_settings.Notifications.EmailServerRequiresSSL;
            cbEmailUseShortFormat.Checked = m_settings.Notifications.UseEmailShortFormat;
            cbEmailAuthRequired.Checked = m_settings.Notifications.EmailAuthenticationRequired;
            tbEmailUsername.Text = m_settings.Notifications.EmailAuthenticationUserName;
            tbEmailPassword.Text = m_settings.Notifications.EmailAuthenticationPassword;
            tbFromAddress.Text = m_settings.Notifications.EmailFromAddress;
            tbToAddress.Text = m_settings.Notifications.EmailToAddress;

            // Proxy settings
            customProxyCheckBox.Checked = m_settings.Proxy.Enabled;
            proxyPortTextBox.Text = m_settings.Proxy.Port.ToString();
            proxyHttpHostTextBox.Text = m_settings.Proxy.Host;
            proxyAuthenticationButton.Tag = m_settings.Proxy;

            // Updates
            cbCheckTimeOnStartup.Checked = m_settings.Updates.CheckTimeOnStartup;
            cbCheckForUpdates.Checked = m_settings.Updates.CheckEVEMonVersion;
            updateSettingsControl.Settings = m_settings.Updates;

            // Skill Planner
            cbHighlightPlannedSkills.Checked = m_settings.UI.PlanWindow.HighlightPlannedSkills;
            cbHighlightPrerequisites.Checked = m_settings.UI.PlanWindow.HighlightPrerequisites;
            cbHighlightConflicts.Checked = m_settings.UI.PlanWindow.HighlightConflicts;
            cbHighlightPartialSkills.Checked = m_settings.UI.PlanWindow.HighlightPartialSkills;
            cbHighlightQueuedSiklls.Checked = m_settings.UI.PlanWindow.HighlightQueuedSkills;
            cbSummaryOnMultiSelectOnly.Checked = m_settings.UI.PlanWindow.OnlyShowSelectionSummaryOnMultiSelect;
            cbAdvanceEntryAdd.Checked = m_settings.UI.PlanWindow.UseAdvanceEntryAddition;

            // Obsolete plan entry removal behaviour
            alwaysAskRadioButton.Checked = (m_settings.UI.PlanWindow.ObsoleteEntryRemovalBehaviour == ObsoleteEntryRemovalBehaviour.AlwaysAsk);
            removeAllRadioButton.Checked = (m_settings.UI.PlanWindow.ObsoleteEntryRemovalBehaviour == ObsoleteEntryRemovalBehaviour.RemoveAll);
            removeConfirmedRadioButton.Checked = (m_settings.UI.PlanWindow.ObsoleteEntryRemovalBehaviour == ObsoleteEntryRemovalBehaviour.RemoveConfirmed);

            // Skill Browser Icon Set
            if (m_settings.UI.SkillBrowser.IconsGroupIndex <= cbSkillIconSet.Items.Count && m_settings.UI.SkillBrowser.IconsGroupIndex > 0)
            {
                cbSkillIconSet.SelectedIndex = m_settings.UI.SkillBrowser.IconsGroupIndex - 1;
            }
            else
            {
                cbSkillIconSet.SelectedIndex = 0;
            }

            // System tray popup/tooltip
            trayPopupRadio.Checked = (m_settings.UI.SystemTrayPopup.Style == TrayPopupStyles.PopupForm);
            trayTooltipRadio.Checked = (m_settings.UI.SystemTrayPopup.Style == TrayPopupStyles.WindowsTooltip);
            trayPopupDisabledRadio.Checked = (m_settings.UI.SystemTrayPopup.Style == TrayPopupStyles.Disabled);

            // Calendar
            panelColorBlocking.BackColor = (Color)m_settings.UI.Scheduler.BlockingColor;
            panelColorRecurring1.BackColor = (Color)m_settings.UI.Scheduler.RecurringEventGradientStart;
            panelColorRecurring2.BackColor = (Color)m_settings.UI.Scheduler.RecurringEventGradientEnd;
            panelColorSingle1.BackColor = (Color)m_settings.UI.Scheduler.SimpleEventGradientStart;
            panelColorSingle2.BackColor = (Color)m_settings.UI.Scheduler.SimpleEventGradientEnd;
            panelColorText.BackColor = (Color)m_settings.UI.Scheduler.TextColor;

            // External calendar
            externalCalendarCheckbox.Checked = m_settings.Calendar.Enabled;

            if (m_settings.Calendar.Provider == CalendarProvider.Outlook)
            {
                rbMSOutlook.Checked = true;
            }
            else
            {
                rbGoogle.Checked = true;
            }

            OnMustEnableOrDisable(externalCalendarCheckbox, e);
            tbGoogleEmail.Text = m_settings.Calendar.GoogleEmail;
            tbGooglePassword.Text = m_settings.Calendar.GooglePassword;
            tbGoogleURI.Text = m_settings.Calendar.GoogleURL;
            cbGoogleReminder.SelectedIndex = (int)m_settings.Calendar.GoogleReminder;
            cbSetReminder.Checked = m_settings.Calendar.UseReminding;
            tbReminder.Text = m_settings.Calendar.RemindingInterval.ToString();
            cbUseAlterateReminder.Checked = m_settings.Calendar.UseRemindingRange;
            dtpEarlyReminder.Value = m_settings.Calendar.EarlyReminding;
            dtpLateReminder.Value = m_settings.Calendar.LateReminding;

            // Run at system startup
            RegistryKey rk = null;
            try
            {
                rk = Registry.CurrentUser.OpenSubKey(StartupRegistryKey, true);
            }
            catch (SecurityException ex)
            {
                ExceptionHandler.LogException(ex, true);
            }
            catch (UnauthorizedAccessException ex)
            {
                ExceptionHandler.LogException(ex, true);
            }

            if (rk == null)
            {
                // No writing rights
                runAtStartupComboBox.Checked = false;
                runAtStartupComboBox.Enabled = false;
            }
            else
            {
                // Run at startup ?
                runAtStartupComboBox.Checked = (rk.GetValue("EVEMon") != null);
            }

            // API providers
            InitialiseAPIProvidersDropDown();

            // Enables / disables controls
            m_isLoading = false;
            UpdateDisables();
        }

        /// <summary>
        /// Fetches the controls' values to <see cref="m_settings"/>.
        /// </summary>
        private bool ApplyToSettings()
        {
            // General - Compatibility
            m_settings.Compatibility = (CompatibilityMode)Math.Max(0, compatibilityCombo.SelectedIndex);
            m_settings.UI.SafeForWork = cbWorksafeMode.Checked;

            // Skill Planner
            m_settings.UI.PlanWindow.HighlightPrerequisites = cbHighlightPrerequisites.Checked;
            m_settings.UI.PlanWindow.HighlightPlannedSkills = cbHighlightPlannedSkills.Checked;
            m_settings.UI.PlanWindow.HighlightConflicts = cbHighlightConflicts.Checked;
            m_settings.UI.PlanWindow.HighlightPartialSkills = cbHighlightPartialSkills.Checked;
            m_settings.UI.PlanWindow.HighlightQueuedSkills = cbHighlightQueuedSiklls.Checked;
            m_settings.UI.PlanWindow.OnlyShowSelectionSummaryOnMultiSelect = cbSummaryOnMultiSelectOnly.Checked;
            m_settings.UI.PlanWindow.UseAdvanceEntryAddition = cbAdvanceEntryAdd.Checked;

            if (alwaysAskRadioButton.Checked)
            {
                m_settings.UI.PlanWindow.ObsoleteEntryRemovalBehaviour = ObsoleteEntryRemovalBehaviour.AlwaysAsk;
            }
            else if (removeAllRadioButton.Checked)
            {
                m_settings.UI.PlanWindow.ObsoleteEntryRemovalBehaviour = ObsoleteEntryRemovalBehaviour.RemoveAll;
            }
            else
            {
                m_settings.UI.PlanWindow.ObsoleteEntryRemovalBehaviour = ObsoleteEntryRemovalBehaviour.RemoveConfirmed;
            }

            // Skill Browser icon sets
            m_settings.UI.SkillBrowser.IconsGroupIndex = cbSkillIconSet.SelectedIndex + 1;

            // Main window skills filter
            m_settings.UI.MainWindow.ShowAllPublicSkills = cbShowAllPublicSkills.Checked;
            m_settings.UI.MainWindow.ShowNonPublicSkills = cbShowNonPublicSkills.Checked;
            m_settings.UI.MainWindow.ShowPrereqMetSkills = cbShowPrereqMetSkills.Checked;

            // System tray icon behaviour
            if (rbSystemTrayOptionsNever.Checked)
            {
                m_settings.UI.SystemTrayIcon = SystemTrayBehaviour.Disabled;
            }
            else if (rbSystemTrayOptionsMinimized.Checked)
            {
                m_settings.UI.SystemTrayIcon = SystemTrayBehaviour.ShowWhenMinimized;
            }
            else if (rbSystemTrayOptionsAlways.Checked)
            {
                m_settings.UI.SystemTrayIcon = SystemTrayBehaviour.AlwaysVisible;
            }

            // Main window close behaviour
            if (rbMinToTaskBar.Checked)
            {
                m_settings.UI.MainWindowCloseBehaviour = CloseBehaviour.MinimizeToTaskbar;
            }
            else if (rbMinToTray.Checked)
            {
                m_settings.UI.MainWindowCloseBehaviour = CloseBehaviour.MinimizeToTray;
            }
            else
            {
                m_settings.UI.MainWindowCloseBehaviour = CloseBehaviour.Exit;
            }

            // Main window
            m_settings.UI.MainWindow.ShowCharacterInfoInTitleBar = cbTitleToTime.Checked;
            m_settings.UI.MainWindow.TitleFormat = (MainWindowTitleFormat)cbWindowsTitleList.SelectedIndex + 1;
            m_settings.UI.MainWindow.ShowSkillNameInWindowTitle = cbSkillInTitle.Checked;
            m_settings.UI.MainWindow.HighlightPartialSkills = cbColorPartialSkills.Checked;
            m_settings.UI.MainWindow.HighlightQueuedSkills = cbColorQueuedSkills.Checked;
            m_settings.UI.MainWindow.AlwaysShowSkillQueueTime = cbAlwaysShowSkillQueueTime.Checked;

            // G15
            m_settings.G15.Enabled = g15CheckBox.Checked;
            m_settings.G15.UseCharactersCycle = cbG15ACycle.Checked;
            m_settings.G15.CharactersCycleInterval = (int)ACycleInterval.Value;
            m_settings.G15.UseTimeFormatsCycle = cbG15CycleTimes.Checked;
            m_settings.G15.TimeFormatsCycleInterval = (int)ACycleTimesInterval.Value;
            m_settings.G15.ShowSystemTime = cbG15ShowTime.Checked;
            m_settings.G15.ShowEVETime = cbG15ShowEVETime.Checked;

            // Notifications
            NotificationSettings notificationSettings = new NotificationSettings();
            PopulateNotificationsFromControls(out notificationSettings);
            m_settings.Notifications = notificationSettings;

            // IGB
            m_settings.IGB.IGBServerEnabled = igbCheckBox.Checked;
            m_settings.IGB.IGBServerPublic = cbIGBPublic.Checked;
            int igbServerPort = m_settings.IGB.IGBServerPort;
            Int32.TryParse(igbPortTextBox.Text, out igbServerPort);
            m_settings.IGB.IGBServerPort = igbServerPort;


            // Main window - Overview
            m_settings.UI.MainWindow.ShowOverview = cbShowOverViewTab.Checked;
            m_settings.UI.MainWindow.UseIncreasedContrastOnOverview = cbUseIncreasedContrastOnOverview.Checked;
            m_settings.UI.MainWindow.ShowOverviewWallet = overviewShowWalletCheckBox.Checked;
            m_settings.UI.MainWindow.ShowOverviewPortrait = overviewShowPortraitCheckBox.Checked;
            m_settings.UI.MainWindow.PutTrainingSkillsFirstOnOverview = overviewGroupCharactersInTrainingCheckBox.Checked;
            m_settings.UI.MainWindow.ShowOverviewSkillQueueTrainingTime = overviewShowSkillQueueTrainingTimeCheckBox.Checked;
            m_settings.UI.MainWindow.OverviewItemSize = (PortraitSizes)overviewPortraitSizeComboBox.SelectedIndex;

            // Tray icon window style
            if (trayPopupRadio.Checked)
            {
                m_settings.UI.SystemTrayPopup.Style = TrayPopupStyles.PopupForm;
            }
            else if (trayTooltipRadio.Checked)
            {
                m_settings.UI.SystemTrayPopup.Style = TrayPopupStyles.WindowsTooltip;
            }
            else
            {
                m_settings.UI.SystemTrayPopup.Style = TrayPopupStyles.Disabled;
            }

            // Proxy
            m_settings.Proxy.Enabled = customProxyCheckBox.Checked;
            int proxyPort = m_settings.Proxy.Port;
            Int32.TryParse(proxyPortTextBox.Text, out proxyPort);
            m_settings.Proxy.Port = proxyPort;
            m_settings.Proxy.Host = proxyHttpHostTextBox.Text;

            // Updates
            m_settings.Updates.CheckEVEMonVersion = cbCheckForUpdates.Checked;
            m_settings.Updates.CheckTimeOnStartup = cbCheckTimeOnStartup.Checked;

            // Scheduler colors
            m_settings.UI.Scheduler.BlockingColor = (SerializableColor)panelColorBlocking.BackColor;
            m_settings.UI.Scheduler.RecurringEventGradientStart = (SerializableColor)panelColorRecurring1.BackColor;
            m_settings.UI.Scheduler.RecurringEventGradientEnd = (SerializableColor)panelColorRecurring2.BackColor;
            m_settings.UI.Scheduler.SimpleEventGradientStart = (SerializableColor)panelColorSingle1.BackColor;
            m_settings.UI.Scheduler.SimpleEventGradientEnd = (SerializableColor)panelColorSingle2.BackColor;
            m_settings.UI.Scheduler.TextColor = (SerializableColor)panelColorText.BackColor;

            // External calendar settings
            m_settings.Calendar.Enabled = externalCalendarCheckbox.Checked;
            if (rbMSOutlook.Checked)
            {
                m_settings.Calendar.Provider = CalendarProvider.Outlook;
            }
            else
            {
                m_settings.Calendar.Provider = CalendarProvider.Google;
            }
            m_settings.Calendar.GoogleEmail = tbGoogleEmail.Text;
            m_settings.Calendar.GooglePassword = tbGooglePassword.Text;
            m_settings.Calendar.GoogleURL = tbGoogleURI.Text;
            m_settings.Calendar.GoogleReminder = cbGoogleReminder.SelectedIndex != -1 ? (GoogleCalendarReminder)cbGoogleReminder.SelectedIndex : GoogleCalendarReminder.None;
            m_settings.Calendar.UseReminding = cbSetReminder.Checked;
            m_settings.Calendar.RemindingInterval = Int32.Parse(tbReminder.Text);
            m_settings.Calendar.UseRemindingRange = cbUseAlterateReminder.Checked;
            m_settings.Calendar.EarlyReminding = dtpEarlyReminder.Value;
            m_settings.Calendar.LateReminding = dtpLateReminder.Value;

            // Updates API provider choices
            m_settings.APIProviders.CurrentProviderName = (string)cbAPIServer.SelectedItem;

            // Run at startup
            if (runAtStartupComboBox.Enabled)
            {
                RegistryKey rk = Registry.CurrentUser.OpenSubKey(StartupRegistryKey, true);
                if (runAtStartupComboBox.Checked)
                {
                    rk.SetValue("EVEMon", String.Format(CultureConstants.DefaultCulture, "\"{0}\" {1}", Application.ExecutablePath.ToString(), "-startMinimized"));
                }
                else
                {
                    rk.DeleteValue("EVEMon", false);
                }
            }

            // Success
            return true;
        }

        /// <summary>
        /// Populates the notifications from controls.
        /// </summary>
        /// <param name="notificationSettings">The notification settings.</param>
        private void PopulateNotificationsFromControls(out NotificationSettings notificationSettings)
        {
            notificationSettings = notificationsControl.Settings;
            notificationSettings.PlaySoundOnSkillCompletion = cbPlaySoundOnSkillComplete.Checked;

            notificationSettings.EmailToAddress = tbToAddress.Text;
            notificationSettings.EmailFromAddress = tbFromAddress.Text;
            notificationSettings.EmailSmtpServer = tbMailServer.Text;

            // Try and get a usable number out of the text box
            int emailPortNumber = notificationSettings.EmailPortNumber;
            if (Int32.TryParse(emailPortTextBox.Text, out emailPortNumber))
            {
                notificationSettings.EmailPortNumber = emailPortNumber;
            }
            // Failing that just set to the IANA assigned port for SMTP
            else
            {
                notificationSettings.EmailPortNumber = 25;
            }

            notificationSettings.EmailServerRequiresSSL = cbEmailServerRequireSsl.Checked;
            notificationSettings.EmailAuthenticationRequired = cbEmailAuthRequired.Checked;
            notificationSettings.EmailAuthenticationUserName = tbEmailUsername.Text;
            notificationSettings.EmailAuthenticationPassword = tbEmailPassword.Text;
            notificationSettings.UseEmailShortFormat = cbEmailUseShortFormat.Checked;
            notificationSettings.SendMailAlert = mailNotificationCheckBox.Checked;
        }

        /// <summary>
        /// Populates the combobox for API providers.
        /// </summary>
        private void InitialiseAPIProvidersDropDown()
        {
            cbAPIServer.Items.Clear();
            cbAPIServer.Items.Add(GlobalAPIProviderCollection.DefaultProvider.Name);
            cbAPIServer.Items.Add(GlobalAPIProviderCollection.TestProvider.Name);
            foreach (var provider in m_settings.APIProviders.CustomProviders)
            {
                cbAPIServer.Items.Add(provider.Name);
                if (provider.Name == m_settings.APIProviders.CurrentProviderName)
                    cbAPIServer.SelectedIndex = cbAPIServer.Items.Count - 1;
            }

            if (m_settings.APIProviders.CurrentProviderName == GlobalAPIProviderCollection.TestProvider.Name)
                cbAPIServer.SelectedIndex = 1;

            // Selects the default API server if none selected
            if (cbAPIServer.SelectedIndex == -1)
                cbAPIServer.SelectedIndex = 0;

            // Disable the drop down box if only one available
            cbAPIServer.Enabled = cbAPIServer.Items.Count > 1;
        }
        #endregion


        #region Validation

        /// <summary>
        /// Reminder value validation.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.ComponentModel.CancelEventArgs"/> instance containing the event data.</param>
        void tbReminder_Validating(object sender, CancelEventArgs e)
        {
            int value;
            if (!Int32.TryParse(tbReminder.Text, out value) || value <= 0)
            {
                e.Cancel = true;
                ShowErrorMessage("Reminder interval", "The reminder interval must be a strictly positive integer");
            }
        }

        /// <summary>
        /// Proxy port validation.
        /// Ensures the text represents a correct port number.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void proxyPortTextBox_Validating(object sender, CancelEventArgs e)
        {
            var text = ((TextBox)sender).Text;
            e.Cancel = !IsValidPort(text, "Proxy port");
        }

        /// <summary>
        /// IGB Port text box validation.
        /// Ensures the text represents a correct port number.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void igbPortTextBox_Validating(object sender, CancelEventArgs e)
        {
            var text = ((TextBox)sender).Text;
            e.Cancel = !IsValidPort(text, "IGB port");
        }

        /// <summary>
        /// IGB Port text box changes
        /// We update the text box displaying the url to use.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void igbPortTextBox_TextChanged(object sender, EventArgs e)
        {
            igbUrlTextBox.Text = String.Format("http://localhost:{0}/", igbPortTextBox.Text);
        }

        /// <summary>
        /// Checks a port is valid and displays a message box when it is not.
        /// </summary>
        /// <param name="str"></param>
        /// <param name="portName"></param>
        /// <returns></returns>
        private bool IsValidPort(string str, string portName)
        {
            int port = -1;
            Int32.TryParse(str, out port);

            if ((port < IPEndPoint.MinPort) || (port > IPEndPoint.MaxPort))
            {
                ShowErrorMessage("Invalid port",
                    String.Format(CultureConstants.DefaultCulture, "{0} value must be between {1} and {2}", portName, IPEndPoint.MinPort, IPEndPoint.MaxPort));

                return false;
            }
            return true;
        }

        /// <summary>
        /// Displays an error message.
        /// </summary>
        /// <param name="caption"></param>
        /// <param name="message"></param>
        private static void ShowErrorMessage(string caption, string message)
        {
            MessageBox.Show(message, caption, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
        #endregion


        #region Updates
        /// <summary>
        /// This handler occurs because some controls' values changed and requires to enable/disable other controls.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnMustEnableOrDisable(object sender, EventArgs e)
        {
            if (m_isLoading)
                return;

            UpdateDisables();
        }

        /// <summary>
        /// Enable or disable controls in reaction to other controls states.
        /// </summary>
        private void UpdateDisables()
        {
            g15Panel.Enabled = g15CheckBox.Checked;
            ACycleInterval.Enabled = cbG15ACycle.Checked;
            ACycleTimesInterval.Enabled = cbG15CycleTimes.Checked;
            igbFlowPanel.Enabled = igbCheckBox.Checked;
            trayIconPopupGroupBox.Enabled = !rbSystemTrayOptionsNever.Checked;
            mailNotificationPanel.Enabled = mailNotificationCheckBox.Checked;
            customProxyPanel.Enabled = customProxyCheckBox.Checked;
            overviewPanel.Enabled = cbShowOverViewTab.Checked;

            cbWindowsTitleList.Enabled = cbTitleToTime.Checked;
            cbSkillInTitle.Enabled = cbTitleToTime.Checked;
            btnEditAPIServer.Enabled = btnDeleteAPIServer.Enabled = cbAPIServer.SelectedIndex > 1;

            // Minimize to tray/task bar
            rbMinToTray.Enabled = !rbSystemTrayOptionsNever.Checked;
            if (rbSystemTrayOptionsNever.Checked && rbMinToTray.Checked)
            {
                rbMinToTaskBar.Checked = true;
            }

            // Calendar
            externalCalendarPanel.Enabled = externalCalendarCheckbox.Checked;
            gbGoogle.Enabled = rbGoogle.Checked;

            // Main window filters (show non-public skills and such)
            if (cbShowAllPublicSkills.Checked)
            {
                cbShowNonPublicSkills.Enabled = true;
                cbShowNonPublicSkills.Checked = m_settings.UI.MainWindow.ShowNonPublicSkills;
                cbShowPrereqMetSkills.Enabled = false;
                cbShowPrereqMetSkills.Checked = false;
            }
            else
            {
                cbShowNonPublicSkills.Enabled = false;
                cbShowNonPublicSkills.Checked = false;
                cbShowPrereqMetSkills.Enabled = true;
                cbShowPrereqMetSkills.Checked = m_settings.UI.MainWindow.ShowPrereqMetSkills;
            }
        }
        #endregion


        #region Buttons handlers
        /// <summary>
        /// Alerts > Email alerts > Send test email button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void emailTestButton_Click(object sender, EventArgs e)
        {
            NotificationSettings configuredValues = new NotificationSettings();
            PopulateNotificationsFromControls(out configuredValues);

            if (!Emailer.SendTestMail(configuredValues))
            {
                MessageBox.Show("The message failed to send.", "Mail Failure", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            else
            {
                MessageBox.Show("The message sent successfully. Please verify that the message was received.",
                                "Mail Success", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        /// <summary>
        /// Network > Proxy > Authentication button.
        /// Shows the proxy authentication configuration form.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void proxyAuthenticationButton_Click(object sender, EventArgs e)
        {
            var proxySettings = m_settings.Proxy.Clone();
            using (ProxyAuthenticationWindow window = new ProxyAuthenticationWindow(proxySettings))
            {
                var result = window.ShowDialog();
                if (result == DialogResult.OK)
                {
                    m_settings.Proxy = proxySettings;
                }
            }
        }

        /// <summary>
        /// Tray icon tooltip > Configure.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void trayTooltipButton_Click(object sender, EventArgs e)
        {
            var tooltipSettings = m_settings.UI.SystemTrayTooltip.Clone();
            using (TrayTooltipConfigForm f = new TrayTooltipConfigForm(tooltipSettings))
            {
                // Set current tooltip string
                f.ShowDialog();
                if (f.DialogResult == DialogResult.OK)
                {
                    // Save changes in local copy
                    m_settings.UI.SystemTrayTooltip = tooltipSettings;
                    trayTooltipRadio.Checked = true;
                }
            }
        }

        /// <summary>
        /// Tray icon popup > Configure.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void trayPopupButton_Click(object sender, EventArgs e)
        {
            var popupSettings = m_settings.UI.SystemTrayPopup.Clone();
            using (TrayPopupConfigForm f = new TrayPopupConfigForm(popupSettings))
            {
                // Edit a copy of the current settings
                f.ShowDialog();
                if (f.DialogResult == DialogResult.OK)
                {
                    m_settings.UI.SystemTrayPopup = popupSettings;
                    trayPopupRadio.Checked = true;
                }
            }
        }

        /// <summary>
        /// General > API Providers > Add.
        /// Displays the API provider configuration.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnAddAPIServer_Click(object sender, EventArgs e)
        {
            SerializableAPIProvider newProvider = new SerializableAPIProvider();
            using (APISettingsForm apiForm = new APISettingsForm(m_settings.APIProviders, newProvider))
            {
                DialogResult result = apiForm.ShowDialog();
                if (result == DialogResult.OK)
                {
                    m_settings.APIProviders.CustomProviders.Add(newProvider);
                    InitialiseAPIProvidersDropDown();
                    cbAPIServer.SelectedIndex = cbAPIServer.Items.Count - 1;
                }
            }
        }

        /// <summary>
        /// General > Network > API Providers > Edit.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnEditAPIServer_Click(object sender, EventArgs e)
        {
            // Search the provider with the selected name
            string name = (string)cbAPIServer.SelectedItem;
            foreach (var provider in m_settings.APIProviders.CustomProviders)
            {
                if (name == provider.Name)
                {
                    // Open the config form for this provider
                    using (APISettingsForm apiForm = new APISettingsForm(m_settings.APIProviders, provider))
                    {
                        apiForm.ShowDialog();
                    }
                    return;
                }
            }
        }

        /// <summary>
        /// General > Network > API Providers > Delete.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnDeleteAPIServer_Click(object sender, EventArgs e)
        {
            string name = (string)cbAPIServer.SelectedItem;
            var result = MessageBox.Show(String.Format(CultureConstants.DefaultCulture, "Delete API Server configuration \"{0}\"?", name),
                    "Delete API Server?", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2);

            if (result == DialogResult.Yes)
            {
                // Search the provider with the selected name
                SerializableAPIProvider providerToRemove = null;
                foreach (var provider in m_settings.APIProviders.CustomProviders)
                {
                    if (name == provider.Name)
                    {
                        providerToRemove = provider;
                        break;
                    }
                }

                // Remove it
                if (providerToRemove != null)
                {
                    m_settings.APIProviders.CustomProviders.Remove(providerToRemove);
                    InitialiseAPIProvidersDropDown();
                    cbAPIServer.SelectedIndex = 0;
                }
            }
        }

        /// <summary>
        /// Reset the priorities conflict custom message box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnPrioritiesReset_Click(object sender, EventArgs e)
        {
            Settings.UI.PlanWindow.PrioritiesMsgBox.ShowDialogBox = true;
            Settings.UI.PlanWindow.PrioritiesMsgBox.DialogResult = DialogResult.None;
        }

        /// <summary>
        /// Updates the timers of the query monitors.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void btnUpdateQueryTimers_Click(object sender, EventArgs e)
        {
            foreach (CCPCharacter character in EveClient.MonitoredCharacters.Where(x => x is CCPCharacter))
            {
                character.QueryMonitors.QueryEverything();
            }
        }
        #endregion


        #region Other handlers
        /// <summary>
        /// Skill Planner > Skill browser icon set > Icons set combo.
        /// Updates the sample below the combo box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void skillIconSetComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            ImageList def = new ImageList();
            def.ColorDepth = ColorDepth.Depth32Bit;
            string groupname = null;
            if (cbSkillIconSet.SelectedIndex >= 0 && cbSkillIconSet.SelectedIndex < IconSettings.Default.Properties.Count - 1)
                groupname = IconSettings.Default.Properties["Group" + (cbSkillIconSet.SelectedIndex + 1)].DefaultValue.ToString();

            if ((groupname != null
                && !System.IO.File.Exists(
                    String.Format(
                        "{1}Resources{0}Skill_Select{0}Group{2}{0}{3}.resources",
                        Path.DirectorySeparatorChar,
                        System.AppDomain.CurrentDomain.BaseDirectory,
                        (cbSkillIconSet.SelectedIndex + 1),
                        groupname)))
                || !System.IO.File.Exists(
                    String.Format(
                        "{1}Resources{0}Skill_Select{0}Group0{0}Default.resources",
                        Path.DirectorySeparatorChar,
                        System.AppDomain.CurrentDomain.BaseDirectory)))
            {
                groupname = null;
            }
            if (groupname != null)
            {
                System.Resources.IResourceReader basic = new System.Resources.ResourceReader(
                    String.Format(
                        "{1}Resources{0}Skill_Select{0}Group0{0}Default.resources",
                        Path.DirectorySeparatorChar,
                        System.AppDomain.CurrentDomain.BaseDirectory));
                System.Collections.IDictionaryEnumerator basicx = basic.GetEnumerator();
                while (basicx.MoveNext())
                {
                    def.Images.Add(basicx.Key.ToString(), (System.Drawing.Icon)basicx.Value);
                }
                basic.Close();
                basic = new System.Resources.ResourceReader(
                    String.Format(
                        "{1}Resources{0}Skill_Select{0}Group{2}{0}{3}.resources",
                        Path.DirectorySeparatorChar,
                        System.AppDomain.CurrentDomain.BaseDirectory,
                        (cbSkillIconSet.SelectedIndex + 1),
                        groupname));
                basicx = basic.GetEnumerator();
                while (basicx.MoveNext())
                {
                    if (def.Images.ContainsKey(basicx.Key.ToString()))
                        def.Images.RemoveByKey(basicx.Key.ToString());

                    def.Images.Add(basicx.Key.ToString(), (System.Drawing.Icon)basicx.Value);
                }
                basic.Close();
            }
            tvlist.Nodes.Clear();
            tvlist.ImageList = def;
            tvlist.ImageList.ColorDepth = ColorDepth.Depth32Bit;
            TreeNode gtn = new TreeNode("Book", tvlist.ImageList.Images.IndexOfKey("book"), tvlist.ImageList.Images.IndexOfKey("book"));
            gtn.Nodes.Add(new TreeNode("Pre-Reqs NOT met (Rank)", tvlist.ImageList.Images.IndexOfKey("PrereqsNOTMet"), tvlist.ImageList.Images.IndexOfKey("PrereqsNOTMet")));
            gtn.Nodes.Add(new TreeNode("Pre-Reqs met (Rank)", tvlist.ImageList.Images.IndexOfKey("PrereqsMet"), tvlist.ImageList.Images.IndexOfKey("PrereqsMet")));
            for (int i = 0; i < 6; i++)
            {
                gtn.Nodes.Add(new TreeNode("Level " + i + " (Rank)", tvlist.ImageList.Images.IndexOfKey("lvl" + i), tvlist.ImageList.Images.IndexOfKey("lvl" + i)));
            }
            gtn.Expand();
            tvlist.Nodes.Add(gtn);
        }

        /// <summary>
        /// Calendar > Scheduler entry colors > color controls.
        /// When clicked, displays a color picker.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void colorPanel_Click(object sender, EventArgs e)
        {
            Panel color = (Panel)sender;
            colorDialog.Color = color.BackColor;
            if (colorDialog.ShowDialog() == DialogResult.OK)
                color.BackColor = colorDialog.Color;
        }

        /// <summary>
        /// Selects the proper page.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void treeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            var text = e.Node.Tag as string;
            foreach (MultiPanelPage page in multiPanel.Controls)
            {
                if (page.Name == text)
                {
                    multiPanel.SelectedPage = page;
                    return;
                }
            }
        }

        /// <summary>
        /// When the size changes, stores the window rect.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnSizeChanged(EventArgs e)
        {
            base.OnSizeChanged(e);
        }

        /// <summary>
        /// Sets the character info max cycle time.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ACycleInterval_ValueChanged(object sender, EventArgs e)
        {
            if (ACycleInterval.Value == 1)
            {
                cbG15CycleTimes.Checked = false;
                panelCycleQueueInfo.Enabled = false;
            }
            else
            {
                ACycleTimesInterval.Maximum = Math.Max(ACycleInterval.Value / 2, 1);
                panelCycleQueueInfo.Enabled = true;
            }
        }

        private void BattleClinicLinkLabel_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Util.OpenURL(NetworkConstants.BattleClinicBase);
        }

        #endregion

    }
}