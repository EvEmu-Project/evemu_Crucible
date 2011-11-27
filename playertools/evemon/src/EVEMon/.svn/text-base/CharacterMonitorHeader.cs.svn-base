using System;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using EVEMon.Accounting;
using EVEMon.Common;
using EVEMon.Common.Net;
using EVEMon.Controls;

namespace EVEMon
{
    /// <summary>
    /// Implements the header component of the main character monitor user interface.
    /// </summary>
    public partial class CharacterMonitorHeader : UserControl
    {
        private Character m_character;
        private int m_spAtLastRedraw;

        #region Constructor

        /// <summary>
        /// Initializes a new instance of the <see cref="CharacterMonitorHeader"/> class.
        /// </summary>
        public CharacterMonitorHeader()
        {
            InitializeComponent();

            // Fonts
            Font = FontFactory.GetFont("Tahoma", FontStyle.Regular);
            CharacterNameLabel.Font = FontFactory.GetFont("Tahoma", 11.25F, FontStyle.Bold);

            // Subscribe to events
            EveClient.TimerTick += EveClient_TimerTick;
            EveClient.SettingsChanged += EveClient_SettingsChanged;
            EveClient.CharacterChanged += EveClient_CharacterChanged;
            EveClient.CharacterMarketOrdersChanged += EveClient_MarketOrdersChanged;
            Disposed += OnDisposed;
        }
        #endregion


        #region Public Properties

        /// <summary>
        /// Gets or sets the character.
        /// </summary>
        /// <value>The character.</value>
        public Character Character
        {
            get { return m_character; }
            set
            {
                if (m_character == value)
                    return;

                m_character = value;

                UpdateFrequentControls();
                UpdateInfrequentControls();
            }
        }

        #endregion


        #region Helper Functions

        /// <summary>
        /// Updates the controls whos content changes frequently.
        /// </summary>
        private void UpdateFrequentControls()
        {
            if (m_character == null)
                return;

            try
            {
                SuspendLayout();
                RefreshThrobber();

                // Only update the skill summary when the skill points change
                if (m_spAtLastRedraw != m_character.SkillPoints)
                    SkillSummaryLabel.Text = FormatSkillSummary(m_character);

                m_spAtLastRedraw = m_character.SkillPoints;
            }
            finally
            {
                ResumeLayout();
            }
        }

        /// <summary>
        /// Updates the controls whos content changes infrequently.
        /// </summary>
        private void UpdateInfrequentControls()
        {
            if (m_character == null)
                return;

            try
            {
                SuspendLayout();

                // Safe for work implementation
                MainTableLayoutPanel.ColumnStyles[0].SizeType = Settings.UI.SafeForWork ? SizeType.Absolute : SizeType.AutoSize;
                MainTableLayoutPanel.ColumnStyles[0].Width = 0;
                CharacterPortrait.Visible = !Settings.UI.SafeForWork;

                CharacterPortrait.Character = m_character;
                CharacterNameLabel.Text = m_character.AdornedName;
                BioInfoLabel.Text = String.Format(CultureConstants.DefaultCulture,
                    "{0} - {1} - {2} - {3}", m_character.Gender, m_character.Race, m_character.Bloodline, m_character.Ancestry);
                BirthdayLabel.Text = String.Format(CultureConstants.DefaultCulture,
                    "Birthday: {0}", m_character.Birthday.ToLocalTime());
                CorporationNameLabel.Text = String.Format(CultureConstants.DefaultCulture,
                    "Corporation: {0}", m_character.CorporationName);

                FormatBalance();

                FormatAttributes();
            }
            finally
            {
                ResumeLayout();
            }
        }

        /// <summary>
        /// Formats the balance.
        /// </summary>
        private void FormatBalance()
        {
            if (m_character == null)
                return;

            BalanceLabel.Text = String.Format(CultureConstants.DefaultCulture,
                "Balance: {0:N} ISK", m_character.Balance);

            var ccpCharacter = m_character as CCPCharacter;

            if (ccpCharacter == null)
                return;

            if (!Settings.UI.SafeForWork && !ccpCharacter.HasSufficientBalance)
            {
                BalanceLabel.ForeColor = Color.Orange;
                BalanceLabel.Font = new Font(Font, FontStyle.Bold);
                return;
            }

            BalanceLabel.ForeColor = SystemColors.ControlText;
            BalanceLabel.Font = new Font(Font, FontStyle.Regular);
        }

        /// <summary>
        /// Refreshes the throbber.
        /// </summary>
        private void RefreshThrobber()
        {
            var ccpCharacter = m_character as CCPCharacter;

            if (ccpCharacter == null)
            {
                HideThrobber();
                return;
            }

            if (ccpCharacter.QueryMonitors.AnyUpdating)
            {
                SetThrobberUpdating();
                return;
            }

            if (!NetworkMonitor.IsNetworkAvailable)
            {
                SetThrobberStrobing("Network Unavailable");
                return;
            }

            SetThrobberStopped();
            UpdateCountdown();
        }

        /// <summary>
        /// Updates the countdown.
        /// </summary>
        private void UpdateCountdown()
        {
            var ccpCharacter = m_character as CCPCharacter;
            var nextMonitor = ccpCharacter.QueryMonitors.NextUpdate;

            if (nextMonitor == null)
            {
                UpdateLabel.Visible = false;
                return;
            }

            UpdateLabel.Visible = true;

            TimeSpan timeLeft = nextMonitor.NextUpdate.Subtract(DateTime.UtcNow);

            if (timeLeft < TimeSpan.Zero)
            {
                UpdateLabel.Text = "Pending...";
                return;
            }

            UpdateLabel.Text = GetCountdownFormat(timeLeft);
        }

        /// <summary>
        /// Gets the countdown format.
        /// </summary>
        /// <param name="timeLeft">The time left.</param>
        /// <returns>String formatted as a countdown timer.</returns>
        /// <remarks>Hours are formatted accumulatively when "Week" is selected</remarks>
        private static string GetCountdownFormat(TimeSpan timeLeft)
        {
            var hours = Math.Floor(timeLeft.TotalHours);
            var minutes = timeLeft.Minutes;
            var seconds = timeLeft.Seconds;
            return String.Format(CultureConstants.DefaultCulture, "{0:#00}:{1:d2}:{2:d2}", hours, minutes, seconds);
        }

        /// <summary>
        /// Sets the throbber stopped.
        /// </summary>
        private void SetThrobberStopped()
        {
            UpdateThrobber.State = ThrobberState.Stopped;

            var ccpCharacter = m_character as CCPCharacter;

            if (ccpCharacter == null)
                return;

            if (ccpCharacter.QueryMonitors.Any(x => x.ForceUpdateWillCauseError) || ccpCharacter.Identity.Account == null)
            {
                ToolTip.SetToolTip(UpdateThrobber, String.Empty);
                return;
            }

            ToolTip.SetToolTip(UpdateThrobber, "Click to update now");
        }

        /// <summary>
        /// Sets the throbber strobing.
        /// </summary>
        /// <param name="status">The status.</param>
        private void SetThrobberStrobing(string status)
        {
            UpdateThrobber.Visible = true;
            UpdateThrobber.State = ThrobberState.Strobing;
            ToolTip.SetToolTip(UpdateThrobber, status);
            UpdateLabel.Visible = false;
        }

        /// <summary>
        /// Sets the throbber updating.
        /// </summary>
        private void SetThrobberUpdating()
        {
            UpdateThrobber.Visible = true;
            UpdateThrobber.State = ThrobberState.Rotating;
            ToolTip.SetToolTip(UpdateThrobber, "Retrieving data from EVE Online...");
            UpdateLabel.Visible = false;
        }

        /// <summary>
        /// Hides the throbber.
        /// </summary>
        private void HideThrobber()
        {
            UpdateThrobber.State = ThrobberState.Stopped;
            UpdateThrobber.Visible = false;
            UpdateLabel.Visible = false;
        }

        /// <summary>
        /// Populates the attribute text for the attribute labels.
        /// </summary>
        private void FormatAttributes()
        {
            SetAttributeLabel(lblINTAttribute, m_character, EveAttribute.Intelligence);
            SetAttributeLabel(lblPERAttribute, m_character, EveAttribute.Perception);
            SetAttributeLabel(lblCHAAttribute, m_character, EveAttribute.Charisma);
            SetAttributeLabel(lblWILAttribute, m_character, EveAttribute.Willpower);
            SetAttributeLabel(lblMEMAttribute, m_character, EveAttribute.Memory);
        }

        /// <summary>
        /// Gets the update status.
        /// </summary>
        /// <returns>Status text to display in the tool tip.</returns>
        private string GetUpdateStatus()
        {
            var ccpCharacter = m_character as CCPCharacter;

            if (ccpCharacter == null)
                return String.Empty;

            var output = new StringBuilder();

            foreach (var monitor in ccpCharacter.QueryMonitors.OrderedByUpdateTime)
            {
                // Skip character's corporation market orders and
                // industry jobs monitor, cause there is no need to
                // show them as they are bind with the character's 
                // personal monitor
                if (monitor.Method == APIMethods.CorporationMarketOrders ||
                    monitor.Method == APIMethods.CorporationIndustryJobs)
                    continue;

                output.AppendLine(GetStatusForMonitor(monitor));
            }

            return output.ToString();
        }

        /// <summary>
        /// Creates the new tool strip separator.
        /// </summary>
        private void CreateNewToolStripSeparator()
        {
            if (ThrobberContextMenu.Items.Contains(ThrobberSeparator))
                return;

            // Add new separator before monitor items
            ThrobberSeparator = new ToolStripSeparator();
            ThrobberSeparator.Name = "throbberSeparator";
            ThrobberContextMenu.Items.Add(ThrobberSeparator);
        }

        /// <summary>
        /// Removes the monitor menu items and separator.
        /// </summary>
        /// <param name="contextMenu">The context menu.</param>
        private void RemoveMonitorMenuItems(ContextMenuStrip contextMenu)
        {
            // Remove all the items after the separator including the separator
            int separatorIndex = contextMenu.Items.IndexOf(ThrobberSeparator);
            while (separatorIndex > -1 && separatorIndex < contextMenu.Items.Count)
            {
                contextMenu.Items.RemoveAt(separatorIndex);
            }
        }

        #endregion


        #region Static Helper Functions

        /// <summary>
        /// Generates text representing the time to next update.
        /// </summary>
        /// <param name="monitor">The monitor.</param>
        /// <returns>String describing the time until the next update.</returns>
        private static string GenerateTimeToNextUpdateText(IQueryMonitor monitor)
        {
            TimeSpan timeToNextUpdate = monitor.NextUpdate.Subtract(DateTime.UtcNow);

            if (timeToNextUpdate <= TimeSpan.Zero)
                return "(Pending)";

            if (monitor.NextUpdate == DateTime.MaxValue)
                return "(Never)";

            if (timeToNextUpdate.TotalMinutes >= 60)
                return String.Format(CultureConstants.DefaultCulture, "({0}h)", Math.Floor(timeToNextUpdate.TotalHours));

            return String.Format(CultureConstants.DefaultCulture, "({0}m)", Math.Floor(timeToNextUpdate.TotalMinutes));
        }


        /// <summary>
        /// Gets the update status for a monitor.
        /// </summary>
        /// <param name="monitor">The monitor.</param>
        /// <returns>Status text for the monitor.</returns>
        private static string GetStatusForMonitor(IQueryMonitor monitor)
        {
            var output = new StringBuilder();

            output.AppendFormat(CultureConstants.DefaultCulture, "{0}: ", monitor);

            if (monitor.Status == QueryStatus.Pending)
            {
                output.Append(GetDetailedStatusForMonitor(monitor));
                return output.ToString();
            }

            output.Append(monitor.Status.GetDescription());
            return output.ToString();
        }

        /// <summary>
        /// Gets the detailed status for monitor.
        /// </summary>
        /// <param name="monitor">The monitor.</param>
        /// <returns>Detailed status text for the monitor.</returns>
        private static string GetDetailedStatusForMonitor(IQueryMonitor monitor)
        {
            if (monitor.NextUpdate == DateTime.MaxValue)
                return "Never";

            var remainingTime = monitor.NextUpdate.Subtract(DateTime.UtcNow);
            if (remainingTime.Minutes > 0)
            {
                return remainingTime.ToDescriptiveText(
                   DescriptiveTextOptions.FullText |
                   DescriptiveTextOptions.SpaceText |
                   DescriptiveTextOptions.SpaceBetween, false);
            }
            else
            {
                return "Less than a minute";
            }
        }

        /// <summary>
        /// Creates the new monitor tool strip menu item.
        /// </summary>
        /// <param name="monitor">The monitor.</param>
        /// <returns>New menu item for a monitor.</returns>
        private static ToolStripMenuItem CreateNewMonitorToolStripMenuItem(IQueryMonitor monitor)
        {
            string menuText = String.Format(CultureConstants.DefaultCulture,
                "Update {0} {1}", monitor.ToString(), GenerateTimeToNextUpdateText(monitor));

            var menu = new ToolStripMenuItem(menuText)
            {
                Tag = (object)monitor.Method,
                Enabled = !monitor.ForceUpdateWillCauseError
            };

            return menu;
        }

        /// <summary>
        /// Gets the attribute text for a character.
        /// </summary>
        /// <param name="character">The character.</param>
        /// <param name="eveAttribute">The eve attribute.</param>
        /// <returns>Formatted string describing the attribute and its value.</returns>
        private static void SetAttributeLabel(Label label, Character character, EveAttribute eveAttribute)
        {
            label.Text = character[eveAttribute].EffectiveValue.ToString(CultureConstants.DefaultCulture);

            label.Tag = eveAttribute;
        }

        /// <summary>
        /// Formats the characters skill summary as a multi-line string.
        /// </summary>
        /// <param name="character">The character.</param>
        /// <returns>Formatted list of information about a characters skills.</returns>
        private static string FormatSkillSummary(Character character)
        {
            StringBuilder output = new StringBuilder();

            // Using the .AppendLine() method of creating a multi line
            // string in this case as it is an interface requirement,
            // rather than a cultural preference.
            output.AppendFormat(CultureConstants.DefaultCulture, "Known Skills: {0}", character.KnownSkillCount).AppendLine();
            output.AppendFormat(CultureConstants.DefaultCulture, "Skills at Level V: {0}", character.GetSkillCountAtLevel(5)).AppendLine();
            output.AppendFormat(CultureConstants.DefaultCulture, "Total SP: {0:#,##0}", character.SkillPoints).AppendLine();
            output.AppendFormat(CultureConstants.DefaultCulture, "Clone Limit: {0:#,##0}", character.CloneSkillPoints).AppendLine();
            output.Append(character.CloneName);

            return output.ToString();
        }

        #endregion
        

        #region Event Handlers

        /// <summary>
        /// Occurs when visibility changes.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            base.OnVisibleChanged(e);

            if (!Visible)
                return;

            UpdateInfrequentControls();
        }

        /// <summary>
        /// Called when the control is disposed.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.TimerTick -= EveClient_TimerTick;
            EveClient.SettingsChanged -= EveClient_SettingsChanged;
            EveClient.CharacterChanged -= EveClient_CharacterChanged;
            EveClient.CharacterMarketOrdersChanged -= EveClient_MarketOrdersChanged;
            Disposed -= OnDisposed;
        }

        /// <summary>
        /// Handles the TimerTick event of the EveClient control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void EveClient_TimerTick(object sender, EventArgs e)
        {
            // No need to do this if control is not visible
            if (!Visible)
                return;

            UpdateFrequentControls();
        }

        /// <summary>
        /// Handles the SettingsChanged event of the EveClient control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            // No need to do this if control is not visible
            if (!Visible)
                return;

            UpdateInfrequentControls();
        }

        /// <summary>
        /// Handles the CharacterChanged event of the EveClient control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            // No need to do this if control is not visible
            if (!Visible)
                return;

            UpdateInfrequentControls();
        }

        /// <summary>
        /// Handles the MarketOrdersChanged event of the EveClient control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_MarketOrdersChanged(object sender, CharacterChangedEventArgs e)
        {
            // No need to do this if control is not visible
            if (!Visible)
                return;

            FormatBalance();
        }

        /// <summary>
        /// Occurs when the user click the throbber.
        /// Query the API for or a full update when possible, or show the throbber's context menu.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void UpdateThrobber_Click(object sender, EventArgs e)
        {
            var ccpCharacter = m_character as CCPCharacter;

            // This is not a CCP account, it can't be updated
            if (ccpCharacter == null)
                return;
            
            // There has been an error in the past (Authorization, Server Error, etc.)
            if (UpdateThrobber.State == ThrobberState.Strobing)
            {
                ThrobberContextMenu.Show(MousePosition);
                return;
            }

            // Updating now will return an API error because the cache has not expired.
            if (ccpCharacter.QueryMonitors.Any(x => x.ForceUpdateWillCauseError))
            {
                ThrobberContextMenu.Show(MousePosition);
                return;
            }

            // All checks out query everything
            ccpCharacter.QueryMonitors.QueryEverything();
        }

        /// <summary>
        /// Handles the MouseHover event of the UpdateLabel control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void UpdateLabel_MouseHover(object sender, EventArgs e)
        {
            ToolTip.SetToolTip(UpdateLabel, GetUpdateStatus());
        }

        /// <summary>
        /// Handles the Opening event of the ThrobberContextMenu control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.ComponentModel.CancelEventArgs"/> instance containing the event data.</param>
        private void ThrobberContextMenu_Opening(object sender, CancelEventArgs e)
        {
            var contextMenu = sender as ContextMenuStrip;

            RemoveMonitorMenuItems(contextMenu);

            var ccpCharacter = m_character as CCPCharacter;

            // Exit for non-CCP characters or no associated account
            if (ccpCharacter == null || ccpCharacter.Identity.Account == null)
            {
                QueryEverythingMenuItem.Enabled = false;
                return;
            }

            // Enables/disables the "query everything" menu item
            QueryEverythingMenuItem.Enabled = !ccpCharacter.QueryMonitors.Any(x => x.ForceUpdateWillCauseError);

            CreateNewToolStripSeparator();

            // Add monitor items
            foreach (var monitor in ccpCharacter.QueryMonitors)
            {
                // Skip full api key related monitor, if api key is a limited one
                if (monitor.IsFullKeyNeeded && ccpCharacter.Identity.Account.KeyLevel != CredentialsLevel.Full)
                    continue;

                // Skip character's corporation market orders and
                // industry jobs monitor, cause there is no need to
                // show them as they are bind with the character's
                // personal monitor
                if (monitor.Method == APIMethods.CorporationMarketOrders ||
                    monitor.Method == APIMethods.CorporationIndustryJobs)
                    continue;

                var menu = CreateNewMonitorToolStripMenuItem(monitor);
                ThrobberContextMenu.Items.Add(menu);
            }
        }

        /// <summary>
        /// Handles the ItemClicked event of the ThrobberContextMenu control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.ToolStripItemClickedEventArgs"/> instance containing the event data.</param>
        private void ThrobberContextMenu_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            var ccpCharacter = m_character as CCPCharacter;

            if (ccpCharacter == null)
                return;

            if (e.ClickedItem == QueryEverythingMenuItem)
                ccpCharacter.QueryMonitors.QueryEverything();

            if (!(e.ClickedItem.Tag is APIMethods))
                return;

            var method = (APIMethods)e.ClickedItem.Tag;

            SetThrobberUpdating();

            ccpCharacter.QueryMonitors.Query(method);

            // Additionaly trigger the update of corporaiton equivalants
            if (method == APIMethods.MarketOrders)
                ccpCharacter.QueryMonitors.Query(APIMethods.CorporationMarketOrders);

            if (method == APIMethods.IndustryJobs)
                ccpCharacter.QueryMonitors.Query(APIMethods.CorporationIndustryJobs);
        }

        /// <summary>
        /// Handles the MouseHover event of the SkillSummaryLabel control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void SkillSummaryLabel_MouseHover(object sender, EventArgs e)
        {
            StringBuilder sb = new StringBuilder();
            for (int skillLevel = 0; skillLevel <= 5; skillLevel++)
            {
                int count = m_character.GetSkillCountAtLevel(skillLevel);

                if (skillLevel > 0)
                    sb.AppendLine();

                sb.AppendFormat(CultureConstants.DefaultCulture, "Skills at Level {0}: {1}", skillLevel, count.ToString().PadLeft(5));
            }

            ToolTip.SetToolTip(sender as Label, sb.ToString());
        }

        /// <summary>
        /// When the user hovers over one of the attribute label, we display a tooltip such as :
        /// 19.8 (7 base + 7 remap points + 4 implants)
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void AttributeLabel_MouseHover(object sender, EventArgs e)
        {
            // Retrieve the attribute from the sender
            Label attributeLabel = sender as Label;
            EveAttribute eveAttribute = (EveAttribute)attributeLabel.Tag;

            // Format the values for the tooltip
            ICharacterAttribute attribute = m_character[eveAttribute];
            string toolTip = attribute.ToString("%e (%B base + %r remap points + %i implants)");
            ToolTip.SetToolTip(attributeLabel, toolTip);
        }

        /// <summary>
        /// Handles the MouseHover event of the CorporationNameLabel control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void CorporationNameLabel_MouseHover(object sender, EventArgs e)
        {
            if (m_character.AllianceID == 0)
                return;

            string tooltipText = String.Format(CultureConstants.DefaultCulture, "Alliance member of: {0}", m_character.AllianceName);
            ToolTip.SetToolTip(sender as Label, tooltipText);
        }

        /// <summary>
        /// Handles the Resize event of the CharacterMonitorHeader control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void CharacterMonitorHeader_Resize(object sender, EventArgs e)
        {
            Height = MainTableLayoutPanel.Height;
            MainTableLayoutPanel.Width = Width;
        }

        /// <summary>
        /// Handles the Click event of the ChangeInfoMenuItem control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void ChangeInfoMenuItem_Click(object sender, EventArgs e)
        {
            // This menu should be enabled only for CCP characters with non-null accounts.
            using (AccountUpdateOrAdditionWindow f = new AccountUpdateOrAdditionWindow(m_character.Identity.Account))
            {
                f.ShowDialog();
            }
        }

        #endregion
    }
}
