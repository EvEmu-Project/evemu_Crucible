//#define DEBUG_SINGLETHREAD
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Notifications;
using EVEMon.Common.Scheduling;
using EVEMon.Common.SettingsObjects;
using EVEMon.Controls;
using EVEMon.ExternalCalendar;

namespace EVEMon
{
    /// <summary>
    /// Implements the content of each of the character tabs.
    /// </summary>
    public partial class CharacterMonitor : UserControl
    {
        private readonly Character m_character;
        private readonly List<ToolStripButton> m_fullAPIKeyFeatures = new List<ToolStripButton>();
        private CredentialsLevel m_keyLevel;
        private bool m_pendingUpdate;


        #region Constructor

        /// <summary>
        /// Design-time constructor
        /// </summary>
        private CharacterMonitor()
        {
            InitializeComponent();
            SetStyle(ControlStyles.SupportsTransparentBackColor, true);

            multiPanel.SelectionChange += multiPanel_SelectionChange;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="CharacterMonitor"/> class.
        /// </summary>
        /// <param name="character">The character.</param>
        public CharacterMonitor(Character character)
            : this()
        {
            m_character = character;
            Header.Character = character;
            skillsList.Character = character;
            skillQueueList.Character = character;
            ordersList.Character = character;
            jobsList.Character = character;
            researchList.Character = character;
            mailMessagesList.Character = character;
            eveNotificationsList.Character = character;
            notificationList.Notifications = null;

            // Create a list of the full API Key features
            m_fullAPIKeyFeatures.Add(ordersIcon);
            m_fullAPIKeyFeatures.Add(jobsIcon);
            m_fullAPIKeyFeatures.Add(researchIcon);
            m_fullAPIKeyFeatures.Add(mailMessagesIcon);
            m_fullAPIKeyFeatures.Add(eveNotificationsIcon);

            // Hide all full api key related controls
            m_fullAPIKeyFeatures.ForEach(x => x.Visible = false);
            featuresMenu.Visible = false;
            toggleSkillsIcon.Visible = tsToggleSeparator.Visible = false;
            toolStripContextual.Visible = false;
            warningLabel.Visible = false;

            // Update all other controls
            EveClient_SettingsChanged(null, EventArgs.Empty);

            var ccpCharacter = character as CCPCharacter;

            if (ccpCharacter != null)
            {
                skillQueueControl.SkillQueue = ccpCharacter.SkillQueue;
            }
            else
            {
                pnlTraining.Visible = false;
                skillQueuePanel.Visible = false;
                skillQueueIcon.Visible = false;
            }

            // Subscribe events
            EveClient.TimerTick += EveClient_TimerTick;
            EveClient.SettingsChanged += EveClient_SettingsChanged;
            EveClient.SchedulerChanged += EveClient_SchedulerChanged;
            EveClient.CharacterChanged += EveClient_CharacterChanged;
            EveClient.CharacterMarketOrdersChanged += EveClient_CharacterMarketOrdersChanged;
            EveClient.CharacterIndustryJobsChanged += EveClient_CharacterIndustryJobsChanged;
            EveClient.CharacterResearchPointsChanged += EveClient_CharacterResearchPointsChanged;
            EveClient.CharacterEVEMailMessagesUpdated += EveClient_CharacterEVEMailMessagesUpdated;
            EveClient.CharacterEVENotificationsUpdated += EveClient_CharacterEVENotificationsUpdated;
            EveClient.NotificationSent += EveClient_NotificationSent;
            EveClient.NotificationInvalidated += EveClient_NotificationInvalidated;
            Disposed += OnDisposed;
        }

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.TimerTick -= EveClient_TimerTick;
            EveClient.SettingsChanged -= EveClient_SettingsChanged;
            EveClient.SchedulerChanged -= EveClient_SchedulerChanged;
            EveClient.CharacterChanged -= EveClient_CharacterChanged;
            EveClient.CharacterMarketOrdersChanged -= EveClient_CharacterMarketOrdersChanged;
            EveClient.CharacterIndustryJobsChanged -= EveClient_CharacterIndustryJobsChanged;
            EveClient.CharacterResearchPointsChanged -= EveClient_CharacterResearchPointsChanged;
            EveClient.CharacterEVEMailMessagesUpdated -= EveClient_CharacterEVEMailMessagesUpdated;
            EveClient.CharacterEVENotificationsUpdated -= EveClient_CharacterEVENotificationsUpdated;
            EveClient.NotificationSent -= EveClient_NotificationSent;
            EveClient.NotificationInvalidated -= EveClient_NotificationInvalidated;
            Disposed -= OnDisposed;
        }

        /// <summary>
        /// Gets the character associated with this monitor.
        /// </summary>
        /// <value>The character.</value>
        public Character Character
        {
            get { return m_character; }
        }

        #endregion


        #region Inherited events

        /// <summary>
        /// On load, we subscribe the events, start the timers, etc...
        /// </summary>
        /// <param name="e">An <see cref="T:System.EventArgs"/> that contains the event data.</param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            Font = FontFactory.GetFont("Tahoma", FontStyle.Regular);
            lblScheduleWarning.Font = FontFactory.GetFont("Tahoma", FontStyle.Bold);

            // Picks the last selected page
            multiPanel.SelectedPage = null;
            string tag = m_character.UISettings.SelectedPage;
            ToolStripItem item =
                toolStripFeatures.Items.Cast<ToolStripItem>().FirstOrDefault(x => tag == x.Tag as string);

            // If it's not a full key feature page make it visible
            if (item != null && !m_fullAPIKeyFeatures.Contains(item))
                item.Visible = true;

            // If it's a full key feature page reset to skills page
            if (item != null && m_fullAPIKeyFeatures.Contains(item))
                item = skillsIcon;

            toolbarIcon_Click((item ?? skillsIcon), EventArgs.Empty);

            // Updates the rest of the control
            UpdateContent();
        }

        /// <summary>
        /// On visibility, we may need to refresh the display.
        /// </summary>
        /// <param name="e">An <see cref="T:System.EventArgs"/> that contains the event data.</param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            if (m_pendingUpdate)
                UpdateContent();

            UpdateFeaturesTextVisibility();

            base.OnVisibleChanged(e);
        }

        #endregion


        #region Display update on character change

        /// <summary>
        /// Updates all the content
        /// </summary>
        private void UpdateContent()
        {
            if (!Visible)
            {
                m_pendingUpdate = true;
                return;
            }

            m_pendingUpdate = false;

            // Display the "no skills" label if there's no skills
            SuspendLayout();
            try
            {
                EveClient_TimerTick(null, EventArgs.Empty);
            }
            finally
            {
                ResumeLayout();
            }
        }

        /// <summary>
        /// Updates the informations for skill training
        /// </summary>
        private void UpdateTrainingInfo()
        {
            var ccpCharacter = m_character as CCPCharacter;

            // Is the character in training ?
            if (m_character.IsTraining)
            {
                QueuedSkill training = m_character.CurrentlyTrainingSkill;
                DateTime completionTime = training.EndTime.ToLocalTime();

                lblTrainingSkill.Text = training.ToString();
                lblSPPerHour.Text = (training.Skill == null
                                         ? "???"
                                         : String.Format(CultureConstants.DefaultCulture, "{0} SP/Hour",
                                                         training.Skill.SkillPointsPerHour));
                lblTrainingEst.Text = String.Format(CultureConstants.DefaultCulture, "{0} {1}",
                                                    completionTime.ToString("ddd"), completionTime.ToString("G"));

                string conflictMessage;
                if (Scheduler.SkillIsBlockedAt(training.EndTime.ToLocalTime(), out conflictMessage))
                {
                    lblScheduleWarning.Text = conflictMessage;
                    lblScheduleWarning.Visible = true;
                }
                else
                {
                    lblScheduleWarning.Visible = false;
                }

                if (ccpCharacter != null)
                {
                    DateTime queueCompletionTime = ccpCharacter.SkillQueue.EndTime.ToLocalTime();
                    lblQueueCompletionTime.Text = String.Format(CultureConstants.DefaultCulture,
                                                                "{0:ddd} {0:G}", queueCompletionTime);
                    if (skillQueueList.QueueHasChanged(ccpCharacter.SkillQueue.ToArray()))
                        skillQueueControl.Invalidate();
                    skillQueuePanel.Visible = true;
                    skillQueueTimePanel.Visible = ccpCharacter.SkillQueue.Count > 1 ||
                                                  Settings.UI.MainWindow.AlwaysShowSkillQueueTime;
                }

                pnlTraining.Visible = true;
                lblPaused.Visible = false;
                return;
            }

            // Not in training, check for paused skill queue
            if (ccpCharacter != null && ccpCharacter.SkillQueue.IsPaused)
            {
                QueuedSkill training = ccpCharacter.SkillQueue.CurrentlyTraining;
                lblTrainingSkill.Text = training.ToString();
                lblSPPerHour.Text = (training.Skill == null
                                         ? "???"
                                         : String.Format(CultureConstants.DefaultCulture, "{0} SP/Hour",
                                                         training.Skill.SkillPointsPerHour));

                lblTrainingRemain.Text = "Paused";
                lblTrainingEst.Text = String.Empty;
                lblScheduleWarning.Visible = false;
                skillQueueTimePanel.Visible = false;
                skillQueuePanel.Visible = true;
                lblPaused.Visible = true;
                pnlTraining.Visible = true;
                return;
            }

            // Not training, no skill queue
            skillQueuePanel.Visible = false;
            pnlTraining.Visible = false;
            lblPaused.Visible = false;
        }

        /// <summary>
        /// Hides or shows the warning about the insufficient key level.
        /// </summary>
        private void UpdateWarningLabel()
        {
            if (m_character.Identity.Account == null)
            {
                warningLabel.Text = "This character has no associated account, data won't be updated.";
                warningLabel.Visible = true;
                return;
            }

            warningLabel.Visible = false;
        }

        /// <summary>
        /// Hides or shows the feature menu that gets enabled by a full api key.
        /// </summary>
        private void UpdateFeaturesMenu()
        {
            Account account = m_character.Identity.Account;
            if (account == null || m_keyLevel == account.KeyLevel)
                return;

            m_keyLevel = account.KeyLevel;

            if (m_character is CCPCharacter)
            {
                if (m_keyLevel == CredentialsLevel.Full)
                {
                    m_fullAPIKeyFeatures.ForEach(x => x.Visible = CheckEnabledFeatures(x.Text));
                    featuresMenu.Visible = true;
                    tsToggleSeparator.Visible = featuresMenu.Visible && toggleSkillsIcon.Visible;

                    ToggleFullAPIKeyFeaturesMonitoring();

                    return;
                }

                m_fullAPIKeyFeatures.ForEach(x => x.Visible = false);
                featuresMenu.Visible = tsToggleSeparator.Visible = false;
                UpdateFullAPIKeyPagesSettings();
            }
        }

        /// <summary>
        /// Hides or shows the pages controls.
        /// </summary>
        private void UpdatePageControls()
        {
            // Enables/Disables the skill page controls
            toggleSkillsIcon.Enabled = !m_character.Skills.IsEmpty();

            // Exit if it's a non-CCPCharacter
            var ccpCharacter = m_character as CCPCharacter;
            if (ccpCharacter == null)
                return;

            // Ensures the visibility of the toolstrip items
            foreach (ToolStripItem item in toolStripContextual.Items)
            {
                item.Visible = true;
            }
            
            // Enables/Disables the market orders page related controls
            if (multiPanel.SelectedPage == ordersPage)
                toolStripContextual.Enabled = !ccpCharacter.MarketOrders.IsEmpty();

            // Enables/Disables the industry jobs page related controls
            if (multiPanel.SelectedPage == jobsPage)
                toolStripContextual.Enabled = !ccpCharacter.IndustryJobs.IsEmpty();

            // Enables/Disables the research points page related controls
            if (multiPanel.SelectedPage == researchPage)
            {
                toolStripContextual.Enabled = !ccpCharacter.ResearchPoints.IsEmpty();
                groupMenu.Visible = false;
            }

            // Enables/Disables the EVE mail messages page related controls
            if (multiPanel.SelectedPage == mailMessagesPage)
                toolStripContextual.Enabled = !ccpCharacter.EVEMailMessages.IsEmpty();

            // Enables/Disables the EVE notifications page related controls
            if (multiPanel.SelectedPage == eveNotificationsPage)
                toolStripContextual.Enabled = !ccpCharacter.EVENotifications.IsEmpty();

        }

        /// <summary>
        /// Toggles the full API key features monitoring.
        /// </summary>
        private void ToggleFullAPIKeyFeaturesMonitoring()
        {
            // Exit if it's a non-CCPCharacter
            var ccpCharacter = m_character as CCPCharacter;
            if (ccpCharacter == null)
                return;

            foreach (IQueryMonitor monitor in ccpCharacter.QueryMonitors.Where(x => x.IsFullKeyNeeded))
            {
                foreach (ToolStripButton fullAPIKeyFeature in m_fullAPIKeyFeatures
                                                        .Where(x => monitor.Method.ToString().Contains(x.Text)))
                {
                    if (monitor.Method.ToString().Contains("Corporation") && ccpCharacter.IsInNPCCorporation)
                        continue;

                    monitor.Enabled = CheckEnabledFeatures(fullAPIKeyFeature.Text);
                }

                if (monitor.Method == APIMethods.MailMessages && monitor.Enabled && monitor.LastResult == null)
                    ccpCharacter.ForceUpdate(monitor);

                if (monitor.Method == APIMethods.Notifications && monitor.Enabled && monitor.LastResult == null)
                    ccpCharacter.ForceUpdate(monitor);
            }
        }

        /// <summary>
        /// Updates full api key pages selection and settings.
        /// </summary>
        private void UpdateFullAPIKeyPagesSettings()
        {
            if ((multiPanel.SelectedPage == ordersPage && !ordersIcon.Visible)
                || (multiPanel.SelectedPage == jobsPage && !jobsIcon.Visible)
                || (multiPanel.SelectedPage == researchPage && !researchIcon.Visible)
                || (multiPanel.SelectedPage == mailMessagesPage && !mailMessagesIcon.Visible)
                || (multiPanel.SelectedPage == eveNotificationsPage && !eveNotificationsIcon.Visible))
                toolbarIcon_Click(skillsIcon, EventArgs.Empty);

            var enabledFullAPIKeyPages = new List<string>();
            foreach (ToolStripMenuItem menuItem in featuresMenu.DropDownItems)
            {
                if (menuItem.Checked)
                    enabledFullAPIKeyPages.Add(menuItem.Text);
            }

            m_character.UISettings.FullAPIKeyEnabledPages.Clear();
            m_character.UISettings.FullAPIKeyEnabledPages.AddRange(enabledFullAPIKeyPages);
        }

        /// <summary>
        /// Check the enabled features from settings.
        /// </summary>
        /// <param name="page"></param>
        /// <returns></returns>
        private bool CheckEnabledFeatures(string page)
        {
            return m_character.UISettings.FullAPIKeyEnabledPages.Any(x => x == page);
        }

        /// <summary>
        /// Update the toolstrip text visibility.
        /// </summary>
        private void UpdateFeaturesTextVisibility()
        {
            if (!Settings.UI.SafeForWork)
            {
                foreach (ToolStripItem item in toolStripFeatures.Items)
                {
                    if (item is ToolStripButton)
                        item.DisplayStyle = (Settings.UI.ShowTextInToolStrip
                                                 ? ToolStripItemDisplayStyle.ImageAndText
                                                 : ToolStripItemDisplayStyle.Image);
                }

                featuresMenu.DisplayStyle = ToolStripItemDisplayStyle.Image;
            }
        }

        #endregion


        #region Updates on global events

        /// <summary>
        /// Updates the page controls on market orders change.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterMarketOrdersChanged(object sender, CharacterChangedEventArgs e)
        {
            if (e.Character != m_character)
                return;

            UpdatePageControls();
        }

        /// <summary>
        /// Updates the page controls on industry jobs change.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterIndustryJobsChanged(object sender, CharacterChangedEventArgs e)
        {
            if (e.Character != m_character)
                return;

            UpdatePageControls();
        }

        /// <summary>
        /// Updates the page controls on research points change.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterResearchPointsChanged(object sender, CharacterChangedEventArgs e)
        {
            if (e.Character != m_character)
                return;

            UpdatePageControls();
        }

        /// <summary>
        /// Updates the page controls on EVE mail messages change.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterEVEMailMessagesUpdated(object sender, CharacterChangedEventArgs e)
        {
            if (e.Character != m_character)
                return;

            UpdatePageControls();
        }

        /// <summary>
        /// Updates the page controls on EVE notifications change.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterEVENotificationsUpdated(object sender, CharacterChangedEventArgs e)
        {
            if (e.Character != m_character)
                return;

            UpdatePageControls();
        }

        /// <summary>
        /// Occur when the character changed. We update all the controls' content.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            if (e.Character != m_character)
                return;

            UpdateContent();
            ToggleFullAPIKeyFeaturesMonitoring();
        }

        /// <summary>
        /// Updates the controls on settings change.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            // Read the settings
            if (!Settings.UI.SafeForWork)
            {

                UpdateFeaturesTextVisibility();

                groupMenu.DisplayStyle = ToolStripItemDisplayStyle.ImageAndText;
                preferencesMenu.DisplayStyle = ToolStripItemDisplayStyle.Image;

                toolStripFeatures.ContextMenuStrip = toolstripContextMenu;
            }
            else
            {
                foreach (ToolStripItem item in toolStripFeatures.Items)
                {
                    if (item is ToolStripButton || item is ToolStripDropDownButton)
                        item.DisplayStyle = ToolStripItemDisplayStyle.Text;
                }

                foreach (ToolStripItem item in toolStripContextual.Items)
                {
                    item.DisplayStyle = ToolStripItemDisplayStyle.Text;
                }

                toolStripFeatures.ContextMenuStrip = null;
            }

            // "Update Calendar" button
            btnAddToCalendar.Visible = Settings.Calendar.Enabled;

            var ccpCharacter = m_character as CCPCharacter;
            if (ccpCharacter == null)
                return;

            // Skill queue time
            if (ccpCharacter.SkillQueue.Count == 1)
                skillQueueTimePanel.Visible = ccpCharacter.IsTraining && Settings.UI.MainWindow.AlwaysShowSkillQueueTime;
        }

        /// <summary>
        /// Occur on every second. We update the total SP, remaining time and the matching item in skill list
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void EveClient_TimerTick(object sender, EventArgs e)
        {
            // No need to do anything when the control is not visible
            if (!Visible)
                return;

            // Update the training info
            UpdateTrainingInfo();

            // Update the warning label
            UpdateWarningLabel();

            // Update the full api key enabled pages
            UpdateFeaturesMenu();

            var ccpCharacter = m_character as CCPCharacter;
            if (ccpCharacter == null)
                return;

            // Exit if the character is not in training
            if (!ccpCharacter.IsTraining)
                return;

            // Remaining training time label
            QueuedSkill training = m_character.CurrentlyTrainingSkill;
            lblTrainingRemain.Text = training.EndTime.ToRemainingTimeDescription(DateTimeKind.Utc);

            // Remaining queue time label
            DateTime queueEndTime = ccpCharacter.SkillQueue.EndTime;
            lblQueueRemaining.Text = queueEndTime.ToRemainingTimeDescription(DateTimeKind.Utc);

        }

        /// <summary>
        /// When the scheduler changed, we need to check the conflicts
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void EveClient_SchedulerChanged(object sender, EventArgs e)
        {
            UpdateTrainingInfo();
        }

        /// <summary>
        /// Update the notifications list.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.Notifications.NotificationInvalidationEventArgs"/> instance containing the event data.</param>
        private void EveClient_NotificationInvalidated(object sender, NotificationInvalidationEventArgs e)
        {
            UpdateNotifications();
        }

        /// <summary>
        /// Update the notifications list.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The e.</param>
        private void EveClient_NotificationSent(object sender, Notification e)
        {
            UpdateNotifications();
        }

        /// <summary>
        /// Update the notifications list.
        /// </summary>
        private void UpdateNotifications()
        {
            notificationList.Notifications = EveClient.Notifications.Where(x => x.Sender == m_character);
        }

        #endregion


        #region Control/Component Event Handlers

        /// <summary>
        /// Handles the Click event of the toolbarIcon control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void toolbarIcon_Click(object sender, EventArgs e)
        {
            foreach (ToolStripItem item in toolStripFeatures.Items)
            {
                // Skip tags without tag, those ones do not represent "pages switches"
                if (item.Tag == null)
                    continue;

                // Is it the item we clicked ?
                var button = item as ToolStripButton;
                if (item == sender)
                {
                    // Selects the proper page
                    multiPanel.SelectedPage =
                        multiPanel.Controls.Cast<MultiPanelPage>().First(x => x.Name == (string) item.Tag);

                    // Checks it
                    button.Checked = true;
                }
                // Or another one representing another page ?
                else if (button != null)
                {
                    // Unchecks it
                    button.Checked = false;
                }
            }
        }

        /// <summary>
        /// When the selected page changes, we may have to update the related controls visibility.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Controls.MultiPanelSelectionChangeEventArgs"/> instance containing the event data.</param>
        private void multiPanel_SelectionChange(object sender, MultiPanelSelectionChangeEventArgs e)
        {
            if (e.NewPage == null)
                return;

            // Stores the setting
            m_character.UISettings.SelectedPage = e.NewPage.Text;

            // Update the buttons visibility
            toggleSkillsIcon.Visible = (e.NewPage == skillsPage);
            tsToggleSeparator.Visible = featuresMenu.Visible && toggleSkillsIcon.Visible;
            toolStripContextual.Visible = (e.NewPage != skillsPage && e.NewPage != skillQueuePage);

            // Update the page controls
            UpdatePageControls();
        }

        /// <summary>
        /// Toggles all the skill groups to collapse or open.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void toggleSkillsIcon_Click(object sender, EventArgs e)
        {
            skillsList.ToggleAll();
        }

        /// <summary>
        /// Occurs when the user click the "add to calendar" button.
        /// We open the unique external calendar window.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void btnAddToCalendar_Click(object sender, EventArgs e)
        {
            // Ensure that we are trying to use the external calendar.
            if (!Settings.Calendar.Enabled)
            {
                btnAddToCalendar.Visible = false;
                return;
            }

            WindowsFactory<ExternalCalendarWindow>.ShowByTag(m_character);
        }

        /// <summary>
        /// Notification list was resized, this may affect the skills list.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void notificationList_Resize(object sender, EventArgs e)
        {
            UpdateNotifications();
            skillsList.Invalidate();
        }

        #endregion


        # region Screenshot Method

        /// <summary>
        /// Takes a screeenshot of this character's monitor and returns it (used for PNG exportation)
        /// </summary>
        /// <returns>Screenshot of a character.</returns>
        internal Bitmap GetCharacterScreenshot()
        {
            int cachedHeight = skillsList.Height;
            int preferredHeight = skillsList.PreferredSize.Height;

            skillsList.Dock = DockStyle.None;
            skillsList.Height = preferredHeight;
            skillsList.Update();

            var bitmap = new Bitmap(skillsList.Width, preferredHeight);
            skillsList.DrawToBitmap(bitmap, new Rectangle(0, 0, skillsList.Width, preferredHeight));

            skillsList.Dock = DockStyle.Fill;
            skillsList.Height = cachedHeight;
            skillsList.Update();

            Invalidate();
            return bitmap;
        }

        # endregion


        # region Multi Panel Control/Component Event Handlers

        /// <summary>
        /// On menu opening we create the menu items and update their checked state.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void featureMenu_DropDownOpening(object sender, EventArgs e)
        {
            featuresMenu.DropDownItems.Clear();

            // Create the menu items
            foreach (FullAPIKeyFeatures feature in EnumExtensions.GetValues<FullAPIKeyFeatures>())
            {
                var item = new ToolStripMenuItem(feature.GetHeader());
                item.Checked = CheckEnabledFeatures(feature.GetHeader());

                featuresMenu.DropDownItems.Add(item);
            }
        }

        /// <summary>
        /// Occurs when the user click an item in the features menu.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void featuresMenu_DropDownItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            var item = e.ClickedItem as ToolStripMenuItem;
            item.Checked = !item.Checked;

            ordersIcon.Visible = (item.Text == ordersIcon.Text ? item.Checked : ordersIcon.Visible);
            jobsIcon.Visible = (item.Text == jobsIcon.Text ? item.Checked : jobsIcon.Visible);
            researchIcon.Visible = (item.Text == researchIcon.Text ? item.Checked : researchIcon.Visible);
            mailMessagesIcon.Visible = (item.Text == mailMessagesIcon.Text ? item.Checked : mailMessagesIcon.Visible);
            eveNotificationsIcon.Visible = (item.Text == eveNotificationsIcon.Text ? item.Checked : eveNotificationsIcon.Visible);

            UpdateFullAPIKeyPagesSettings();
            ToggleFullAPIKeyFeaturesMonitoring();
       }

        /// <summary>
        /// On opening we create the menu items for "Group By..." in panel.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void groupMenu_DropDownOpening(object sender, EventArgs e)
        {
            groupMenu.DropDownItems.Clear();

            if (multiPanel.SelectedPage == ordersPage)
                CreateGroupMenuList<MarketOrderGrouping, Enum>(ordersList);
            
            if (multiPanel.SelectedPage == jobsPage)
                CreateGroupMenuList<IndustryJobGrouping, Enum>(jobsList);

            if (multiPanel.SelectedPage == mailMessagesPage)
                CreateGroupMenuList<EVEMailMessagesGrouping, Enum>(mailMessagesList);

            if (multiPanel.SelectedPage == eveNotificationsPage)
                CreateGroupMenuList<EVENotificationsGrouping, Enum>(eveNotificationsList);
        }

        /// <summary>
        /// Occurs when the user click an item in the "Group By..." menu.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.ToolStripItemClickedEventArgs"/> instance containing the event data.</param>
        private void groupMenu_DropDownItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            ToolStripItem item = e.ClickedItem;
            if (multiPanel.SelectedPage == ordersPage)
                GroupMenuSetting<MarketOrderGrouping, Enum>(item, ordersList);
            
            if (multiPanel.SelectedPage == jobsPage)
                GroupMenuSetting<IndustryJobGrouping, Enum>(item, jobsList);

            if (multiPanel.SelectedPage == mailMessagesPage)
                GroupMenuSetting<EVEMailMessagesGrouping, Enum>(item, mailMessagesList);

            if (multiPanel.SelectedPage == eveNotificationsPage)
                GroupMenuSetting<EVENotificationsGrouping, Enum>(item, eveNotificationsList);
        }

        /// <summary>
        /// Occurs when the search text changes.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void searchTextBox_TextChanged(object sender, EventArgs e)
        {
            if (multiPanel.SelectedPage == ordersPage)
                ordersList.TextFilter = searchTextBox.Text;
            
            if (multiPanel.SelectedPage == jobsPage)
                jobsList.TextFilter = searchTextBox.Text;

            if (multiPanel.SelectedPage == researchPage)
                researchList.TextFilter = searchTextBox.Text;

            if (multiPanel.SelectedPage == mailMessagesPage)
                mailMessagesList.TextFilter = searchTextBox.Text;

            if (multiPanel.SelectedPage == eveNotificationsPage)
                eveNotificationsList.TextFilter = searchTextBox.Text;
        }

        /// <summary>
        /// Display the window to select columns.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void columnSettingsMenuItem_Click(object sender, EventArgs e)
        {
            if (multiPanel.SelectedPage == ordersPage)
            {
                using (var f = new MarketOrdersColumnsSelectWindow(ordersList.Columns.Select(x => x.Clone())))
                {
                    DialogResult dr = f.ShowDialog();
                    if (dr == DialogResult.OK)
                    {
                        ordersList.Columns = f.Columns.Cast<MarketOrderColumnSettings>();
                        Settings.UI.MainWindow.MarketOrders.Columns =
                            ordersList.Columns.Select(x => x.Clone()).ToArray();
                    }
                }
            }
            
            if (multiPanel.SelectedPage == jobsPage)
            {
                using (var f = new IndustryJobsColumnsSelectWindow(jobsList.Columns.Select(x => x.Clone())))
                {
                    DialogResult dr = f.ShowDialog();
                    if (dr == DialogResult.OK)
                    {
                        jobsList.Columns = f.Columns.Cast<IndustryJobColumnSettings>();
                        Settings.UI.MainWindow.IndustryJobs.Columns =
                            jobsList.Columns.Select(x => x.Clone()).ToArray();
                    }
                }
            }

            if (multiPanel.SelectedPage == researchPage)
            {
                using (var f = new ResearchColumnsSelectWindow(researchList.Columns.Select(x => x.Clone())))
                {
                    DialogResult dr = f.ShowDialog();
                    if (dr == DialogResult.OK)
                    {
                        researchList.Columns = f.Columns.Cast<ResearchColumnSettings>();
                        Settings.UI.MainWindow.Research.Columns =
                            researchList.Columns.Select(x => x.Clone()).ToArray();
                    }
                }
            }

            if (multiPanel.SelectedPage == mailMessagesPage)
            {
                using (var f = new EveMailMessagesColumnsSelectWindow(mailMessagesList.Columns.Select(x => x.Clone())))
                {
                    DialogResult dr = f.ShowDialog();
                    if (dr == DialogResult.OK)
                    {
                        mailMessagesList.Columns = f.Columns.Cast<EveMailMessagesColumnSettings>();
                        Settings.UI.MainWindow.EVEMailMessages.Columns =
                            mailMessagesList.Columns.Select(x => x.Clone()).ToArray();
                    }
                }
            }

            if (multiPanel.SelectedPage == eveNotificationsPage)
            {
                using (var f = new EveNotificationsColumnsSelectWindow(eveNotificationsList.Columns.Select(x => x.Clone())))
                {
                    DialogResult dr = f.ShowDialog();
                    if (dr == DialogResult.OK)
                    {
                        eveNotificationsList.Columns = f.Columns.Cast<EveNotificationsColumnSettings>();
                        Settings.UI.MainWindow.EVENotifications.Columns =
                            eveNotificationsList.Columns.Select(x => x.Clone()).ToArray();
                    }
                }
            }
        }

        /// <summary>
        /// On menu opening we update the menu items.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void preferencesMenu_DropDownOpening(object sender, EventArgs e)
        {
            bool hideInactive = true;
            bool numberFormat = false;

            if (multiPanel.SelectedPage == ordersPage)
            {
                hideInactive = Settings.UI.MainWindow.MarketOrders.HideInactiveOrders;
                numberFormat = Settings.UI.MainWindow.MarketOrders.NumberAbsFormat;
                preferencesMenu.DropDownItems.Insert(3, numberAbsFormatMenuItem);
                preferencesMenu.DropDownItems.Remove(tsReadingPaneSeparator);
                preferencesMenu.DropDownItems.Remove(readingPaneMenuItem);
                numberAbsFormatMenuItem.Text = (numberFormat ? "Number Full Format" : "Number Abbreviating Format");
                showOnlyCharMenuItem.Checked = ordersList.ShowIssuedFor == IssuedFor.Character;
                showOnlyCorpMenuItem.Checked = ordersList.ShowIssuedFor == IssuedFor.Corporation;
            }
            
            if (multiPanel.SelectedPage == jobsPage)
            {
                hideInactive = Settings.UI.MainWindow.IndustryJobs.HideInactiveJobs;
                preferencesMenu.DropDownItems.Remove(numberAbsFormatMenuItem);
                showOnlyCharMenuItem.Checked = jobsList.ShowIssuedFor == IssuedFor.Character;
                showOnlyCorpMenuItem.Checked = jobsList.ShowIssuedFor == IssuedFor.Corporation;
                preferencesMenu.DropDownItems.Remove(tsReadingPaneSeparator);
                preferencesMenu.DropDownItems.Remove(readingPaneMenuItem);
            }

            if (multiPanel.SelectedPage == researchPage)
            {
                preferencesMenu.DropDownItems.Clear();
                preferencesMenu.DropDownItems.Add(columnSettingsMenuItem);
                preferencesMenu.DropDownItems.Remove(tsReadingPaneSeparator);
                preferencesMenu.DropDownItems.Remove(readingPaneMenuItem);
            }

            if (multiPanel.SelectedPage == mailMessagesPage || multiPanel.SelectedPage == eveNotificationsPage)
            {
                preferencesMenu.DropDownItems.Clear();
                preferencesMenu.DropDownItems.Add(columnSettingsMenuItem);
                preferencesMenu.DropDownItems.Add(tsReadingPaneSeparator);
                preferencesMenu.DropDownItems.Add(readingPaneMenuItem);
            }

            hideInactiveMenuItem.Text = (hideInactive ? "Unhide Inactive" : "Hide Inactive");
        }

        /// <summary>
        /// Hide/Show the inactive entries.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void hideInactiveMenuItem_Click(object sender, EventArgs e)
        {
            bool hideInactive = true;
            if (multiPanel.SelectedPage == ordersPage)
            {
                hideInactive = Settings.UI.MainWindow.MarketOrders.HideInactiveOrders;
                Settings.UI.MainWindow.MarketOrders.HideInactiveOrders = !hideInactive;
                ordersList.UpdateColumns();
            }
            
            if (multiPanel.SelectedPage == jobsPage)
            {
                hideInactive = Settings.UI.MainWindow.IndustryJobs.HideInactiveJobs;
                Settings.UI.MainWindow.IndustryJobs.HideInactiveJobs = !hideInactive;
                jobsList.UpdateColumns();
            }
            hideInactiveMenuItem.Text = (!hideInactive ? "Unhide Inactive" : "Hide Inactive");
        }

        /// <summary>
        /// Switches between Abbreviating/Full number format.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void numberAbsFormatMenuItem_Click(object sender, EventArgs e)
        {
            bool numberFormat = Settings.UI.MainWindow.MarketOrders.NumberAbsFormat;
            numberAbsFormatMenuItem.Text = (!numberFormat ? "Number Full Format" : "Number Abbreviating Format");
            Settings.UI.MainWindow.MarketOrders.NumberAbsFormat = !numberFormat;
            ordersList.UpdateColumns();
        }

        /// <summary>
        /// Displays only the entries issued for character.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void showOnlyCharMenuItem_Click(object sender, EventArgs e)
        {
            if (multiPanel.SelectedPage == ordersPage)
            {
                ordersList.ShowIssuedFor = (showOnlyCharMenuItem.Checked ? IssuedFor.Character : IssuedFor.All);
                showOnlyCorpMenuItem.Checked = (ordersList.ShowIssuedFor == IssuedFor.Corporation);
            }
            
            if (multiPanel.SelectedPage == jobsPage)
            {
                jobsList.ShowIssuedFor = (showOnlyCharMenuItem.Checked ? IssuedFor.Character : IssuedFor.All);
                showOnlyCorpMenuItem.Checked = (jobsList.ShowIssuedFor == IssuedFor.Corporation);
            }
        }

        /// <summary>
        /// Displays only the entries issued for corporation.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void showOnlyCorpMenuItem_Click(object sender, EventArgs e)
        {
            if (multiPanel.SelectedPage == ordersPage)
            {
                ordersList.ShowIssuedFor = (showOnlyCorpMenuItem.Checked ? IssuedFor.Corporation : IssuedFor.All);
                showOnlyCharMenuItem.Checked = (ordersList.ShowIssuedFor == IssuedFor.Character);
            }
            
            if (multiPanel.SelectedPage == jobsPage)
            {
                jobsList.ShowIssuedFor = (showOnlyCorpMenuItem.Checked ? IssuedFor.Corporation : IssuedFor.All);
                showOnlyCharMenuItem.Checked = (jobsList.ShowIssuedFor == IssuedFor.Character);
            }
        }

        /// <summary>
        /// Handles the DropDownOpening event of the readingPaneMenuItem control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void readingPaneMenuItem_DropDownOpening(object sender, EventArgs e)
        {
            string paneSetting = ReadingPanePositioning.Off.ToString();

            if (multiPanel.SelectedPage == mailMessagesPage)
                paneSetting = Settings.UI.MainWindow.EVEMailMessages.ReadingPanePosition.ToString();

            if (multiPanel.SelectedPage == eveNotificationsPage)
                paneSetting = Settings.UI.MainWindow.EVENotifications.ReadingPanePosition.ToString();

            foreach (ToolStripMenuItem menuItem in readingPaneMenuItem.DropDownItems)
            {
                menuItem.Checked = ((string)menuItem.Tag == paneSetting);
            }
        }

        /// <summary>
        /// Handles the Click event of the paneRightMenuItem control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void paneRightMenuItem_Click(object sender, EventArgs e)
        {
            foreach (ToolStripMenuItem menuItem in readingPaneMenuItem.DropDownItems)
            {
                menuItem.Checked = false;
            }

            paneRightMenuItem.Checked = true;

            if (multiPanel.SelectedPage == mailMessagesPage)
            {
                mailMessagesList.PanePosition = ReadingPanePositioning.Right;
                Settings.UI.MainWindow.EVEMailMessages.ReadingPanePosition = mailMessagesList.PanePosition;
            }

            if (multiPanel.SelectedPage == eveNotificationsPage)
            {
                eveNotificationsList.PanePosition = ReadingPanePositioning.Right;
                Settings.UI.MainWindow.EVENotifications.ReadingPanePosition = eveNotificationsList.PanePosition;
            }
        }

        /// <summary>
        /// Handles the Click event of the paneBottomMenuItem control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void paneBottomMenuItem_Click(object sender, EventArgs e)
        {
            foreach (ToolStripMenuItem menuItem in readingPaneMenuItem.DropDownItems)
            {
                menuItem.Checked = false;
            }

            paneBottomMenuItem.Checked = true;

            if (multiPanel.SelectedPage == mailMessagesPage)
            {
                mailMessagesList.PanePosition = ReadingPanePositioning.Bottom;
                Settings.UI.MainWindow.EVEMailMessages.ReadingPanePosition = mailMessagesList.PanePosition;
            }

            if (multiPanel.SelectedPage == eveNotificationsPage)
            {
                eveNotificationsList.PanePosition = ReadingPanePositioning.Bottom;
                Settings.UI.MainWindow.EVENotifications.ReadingPanePosition = eveNotificationsList.PanePosition;
            }
        }

        /// <summary>
        /// Handles the Click event of the paneOffMenuItem control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void paneOffMenuItem_Click(object sender, EventArgs e)
        {
            foreach (ToolStripMenuItem menuItem in readingPaneMenuItem.DropDownItems)
            {
                menuItem.Checked = false;
            }

            paneOffMenuItem.Checked = true;

            if (multiPanel.SelectedPage == mailMessagesPage)
            {
                mailMessagesList.PanePosition = ReadingPanePositioning.Off;
                Settings.UI.MainWindow.EVEMailMessages.ReadingPanePosition = mailMessagesList.PanePosition;
            }

            if (multiPanel.SelectedPage == eveNotificationsPage)
            {
                eveNotificationsList.PanePosition = ReadingPanePositioning.Off;
                Settings.UI.MainWindow.EVENotifications.ReadingPanePosition = eveNotificationsList.PanePosition;
            }
        }

        /// <summary>
        /// On menu opening we update the menu items.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void toolstripContextMenu_Opening(object sender, CancelEventArgs e)
        {
            showTextMenuItem.Checked = Settings.UI.ShowTextInToolStrip;
        }

        /// <summary>
        /// Shows or hides the text of the toolstrip items.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void showTextMenuItem_Click(object sender, EventArgs e)
        {
            Settings.UI.ShowTextInToolStrip = showTextMenuItem.Checked = !showTextMenuItem.Checked;
            EveClient_SettingsChanged(null, EventArgs.Empty);
        }

        # endregion


        #region Generic Helper Methods

        /// <summary>
        /// Creates the group menu list.
        /// </summary>
        /// <typeparam name="T">The grouping type.</typeparam>
        /// <typeparam name="V">The grouping base type.</typeparam>
        /// <param name="list">The list.</param>
        private void CreateGroupMenuList<T, V>(IGroupingListView list)
            where T : V
        {
            foreach (T grouping in EnumExtensions.GetValues<T>())
            {
                Enum group = grouping as Enum;
                if (group == null)
                    continue;

                var menu = new ToolStripButton(group.GetHeader());
                menu.Checked = (list.Grouping.CompareTo(group) == 0);
                menu.Tag = (T)grouping;

                groupMenu.DropDownItems.Add(menu);
            }
        }

        /// <summary>
        /// Sets and stores in settings the GroupBy selection.
        /// </summary>
        /// <typeparam name="T">The grouping type.</typeparam>
        /// <typeparam name="V">The grouping base type.</typeparam>
        /// <param name="item">The item.</param>
        /// <param name="list">The list.</param>
        private void GroupMenuSetting<T, V>(ToolStripItem item, IGroupingListView list)
            where T : V
        {
            Enum grouping = item.Tag as Enum;
            if (grouping == null)
                return;

            list.Grouping = grouping;
            T obj = default(T);

            if (obj is MarketOrderGrouping)
                m_character.UISettings.OrdersGroupBy = (MarketOrderGrouping)grouping;

            if (obj is IndustryJobGrouping)
                m_character.UISettings.JobsGroupBy = (IndustryJobGrouping)grouping;

            if (obj is EVEMailMessagesGrouping)
                m_character.UISettings.EVEMailMessagesGroupBy = (EVEMailMessagesGrouping)grouping;

            if (obj is EVENotificationsGrouping)
                m_character.UISettings.EVENotificationsGroupBy = (EVENotificationsGrouping)grouping;
        }
        
        #endregion
        
        
        #region Testing Function

        /// <summary>
        /// Tests character's notification display in the Character Monitor.
        /// </summary>
        internal void TestCharacterNotification()
        {
            var notification = new Notification(NotificationCategory.TestNofitication, m_character)
                                   {
                                       Priority = NotificationPriority.Warning,
                                       Behaviour = NotificationBehaviour.Overwrite,
                                       Description = "Test Character Notification."
                                   };
            EveClient.Notifications.Notify(notification);
        }

        #endregion
    }
}