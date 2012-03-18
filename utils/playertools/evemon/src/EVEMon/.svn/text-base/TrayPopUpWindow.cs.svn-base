using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.SettingsObjects;
using EVEMon.Controls;

namespace EVEMon
{
    /// <summary>
    /// Popup form displayed when the user hovers over the tray icon
    /// </summary>
    /// <remarks>
    /// Display contents are governed by Settings.TrayPopupConfig<br/>
    /// Popup location is determined using mouse location, screen and screen bounds (see SetPosition()).<br/>
    /// </remarks>
    public partial class TrayPopUpWindow : Form
    {
        private bool m_updatePending;

        private Label m_eveTimeLabel;
        private Label m_serverStatusLabel;
        private int[] m_portraitSize = { 16, 24, 32, 40, 48, 56, 64 };

        /// <summary>
        /// Default constructor
        /// </summary>
        public TrayPopUpWindow()
        {
            InitializeComponent();
        }


        #region Control's lifecycle management and painting
        /// <summary>
        /// Adds the character panes to the form, gets the TQ status message and sets the popup position
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            // Look'n feel
            this.Font = FontFactory.GetFont(SystemFonts.MessageBoxFont.Name, SystemFonts.MessageBoxFont.SizeInPoints, FontStyle.Regular, GraphicsUnit.Point);
            mainPanel.BackColor = SystemColors.ControlLightLight;

            // Client events
            EveClient.MonitoredCharacterCollectionChanged += new EventHandler(EveClient_MonitoredCharacterCollectionChanged);
            EveClient.QueuedSkillsCompleted += new EventHandler<QueuedSkillsEventArgs>(EveClient_QueuedSkillsCompleted);
            EveClient.ServerStatusUpdated += new EventHandler<EveServerEventArgs>(EveClient_ServerStatusUpdated);
            EveClient.SettingsChanged += new EventHandler(EveClient_SettingsChanged);
            EveClient.TimerTick += new EventHandler(EveClient_TimerTick);

            // Character Details
            UpdateContent();
        }

        /// <summary>
        /// Unregister events
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);
            EveClient.MonitoredCharacterCollectionChanged -= new EventHandler(EveClient_MonitoredCharacterCollectionChanged);
            EveClient.QueuedSkillsCompleted -= new EventHandler<QueuedSkillsEventArgs>(EveClient_QueuedSkillsCompleted);
            EveClient.ServerStatusUpdated -= new EventHandler<EveServerEventArgs>(EveClient_ServerStatusUpdated);
            EveClient.SettingsChanged -= new EventHandler(EveClient_SettingsChanged);
            EveClient.TimerTick -= new EventHandler(EveClient_TimerTick);
        }

        /// <summary>
        /// When the window becomes visible again, checks whether an update is pending.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            if (this.Visible && m_updatePending)
            {
                UpdateContent();
            }
            base.OnVisibleChanged(e);
        }

        /// <summary>
        /// Draws the rounded rectangle border and background
        /// </summary>
        /// <param name="e"></param>
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            // Draw the border and background
            DrawBorder(e);
        }

        /// <summary>
        /// Sets this window as topmost without activiting it
        /// </summary>
        /// <param name="e"></param>
        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            // Equivalent to setting TopMost = true, except don't activate the window.
            NativeMethods.SetWindowPos(this.Handle, NativeMethods.HWND_TOPMOST, 0, 0, 0, 0,
                NativeMethods.SWP_NOACTIVATE | NativeMethods.SWP_NOMOVE | NativeMethods.SWP_NOSIZE);
            // Show the window without activating it.
            NativeMethods.ShowWindow(this.Handle, NativeMethods.SW_SHOWNOACTIVATE);
        }

        /// <summary>
        /// Draws the rounded rectangle border
        /// </summary>
        /// <param name="e"></param>
        private void DrawBorder(PaintEventArgs e)
        {
            // Create graphics object to work with
            Graphics g = e.Graphics;
            g.SmoothingMode = SmoothingMode.HighQuality;
            // Define the size of the rectangle used for each of the 4 corner arcs.
            int radius = 4;
            Size cornerSize = new Size(radius * 2, radius * 2);
            // Construct a GraphicsPath for the outline
            GraphicsPath path = new GraphicsPath();
            path.StartFigure();
            // Top left
            path.AddArc(new Rectangle(0, 0, cornerSize.Width, cornerSize.Height), 180, 90);
            // Top Right
            path.AddArc(new Rectangle(e.ClipRectangle.Width - 1 - cornerSize.Width, 0, cornerSize.Width, cornerSize.Height), 270, 90);
            // Bottom right
            path.AddArc(new Rectangle(e.ClipRectangle.Width - 1 - cornerSize.Width, e.ClipRectangle.Height - 1 - cornerSize.Height, cornerSize.Width, cornerSize.Height), 0, 90);
            // Bottom Left
            path.AddArc(new Rectangle(0, e.ClipRectangle.Height - 1 - cornerSize.Height, cornerSize.Width, cornerSize.Height), 90, 90);
            path.CloseFigure();
            // Draw the background
            Brush fillBrush = new SolidBrush(SystemColors.ControlLightLight);
            g.FillPath(fillBrush, path);
            // Now the border
            Pen borderPen = SystemPens.WindowFrame;
            g.DrawPath(borderPen, path);
        }
        #endregion


        #region Eve client events
        /// <summary>
        /// Occurs when the monitored characters collection changed
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_MonitoredCharacterCollectionChanged(object sender, EventArgs e)
        {
            UpdateContent();
        }

        /// <summary>
        /// Updates the TQ status message
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_ServerStatusUpdated(object sender, EveServerEventArgs e)
        {
            UpdateServerStatusLabel();
        }

        /// <summary>
        /// Once per second, we update the eve time
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_TimerTick(object sender, EventArgs e)
        {
            UpdateEveTimeLabel();
        }

        /// <summary>
        /// Occur when characters completed skills. We refresh the controls.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_QueuedSkillsCompleted(object sender, QueuedSkillsEventArgs e)
        {
            UpdateContent();
        }

        /// <summary>
        /// When the settings changed, some may affect this popup, so we recreate content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            UpdateContent();
        }
        #endregion


        #region Content management : add the controls to the panel, update them, etc
        /// <summary>
        /// Recreates the controls for character and warning
        /// </summary>
        private void UpdateContent()
        {
            if (!this.Visible)
            {
                m_updatePending = true;
                return;
            }
            m_updatePending = false;

            IEnumerable<Character> characters = GetCharacters();

            // Remove controls and dispose them
            var oldControls = mainPanel.Controls.Cast<Control>().ToArray();
            mainPanel.Controls.Clear();
            foreach (var ctl in oldControls)
            {
                ctl.Dispose();
            }

            // Add controls for characters
			if (Settings.UI.SystemTrayPopup.GroupBy == TrayPopupGrouping.Account && Settings.UI.SystemTrayPopup.IndentGroupedAccounts)
			{
				long PrevUserID = 0;
				foreach (Character character in characters)
				{
					if (character.Identity.Account.UserID != PrevUserID)
					{
						mainPanel.Controls.Add(new OverviewItem(character, Settings.UI.SystemTrayPopup));
						PrevUserID = character.Identity.Account.UserID;
					}
					else
					{
						FlowLayoutPanel AccountGroupPanel = new FlowLayoutPanel();
						AccountGroupPanel.AutoSize = true;
						AccountGroupPanel.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
						AccountGroupPanel.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
						AccountGroupPanel.Padding = new System.Windows.Forms.Padding(10, 0, 0, 0);
						OverviewItem charpanel = new OverviewItem(character, Settings.UI.SystemTrayPopup);
						charpanel.Padding = new System.Windows.Forms.Padding(0, 0, 0, 0);
						AccountGroupPanel.Controls.Add(charpanel);
						mainPanel.Controls.Add(AccountGroupPanel);
						PrevUserID = character.Identity.Account.UserID;
					}
				}
			}
			else
			{
				mainPanel.Controls.AddRange(characters.Select(x => new OverviewItem(x, Settings.UI.SystemTrayPopup)).ToArray());
			}

            // Return if the user do not want to be warned about accounts not in training
            if (Settings.UI.SystemTrayPopup.ShowWarning)
            {
                // Creates the warning for accounts not in training
                string warningMessage;
                if (EveClient.Accounts.HasAccountsNotTraining(out warningMessage))
                {
                    FlowLayoutPanel warningPanel = CreateAccountsNotTrainingPanel(warningMessage);
                    mainPanel.Controls.Add(warningPanel);
                }
            }

            // TQ Server Status
            if (Settings.UI.SystemTrayPopup.ShowServerStatus)
            {
                m_serverStatusLabel = new Label();
                m_serverStatusLabel.AutoSize = true;
                mainPanel.Controls.Add(m_serverStatusLabel);
                UpdateServerStatusLabel();
            }

            // EVE Time
            if (Settings.UI.SystemTrayPopup.ShowEveTime)
            {
                m_eveTimeLabel = new Label();
                m_eveTimeLabel.AutoSize = true;
                mainPanel.Controls.Add(m_eveTimeLabel);
                UpdateEveTimeLabel();
            }

            // Updates the tooltip width
            CompleteLayout();
        }

        /// <summary>
        /// Gets the characters list, sorted, grouped and filter according to the user settings.
        /// </summary>
        /// <returns></returns>
        public static IEnumerable<Character> GetCharacters()
        {
            IEnumerable<Character> characters = EveClient.MonitoredCharacters;

            // Filter characters not in training ?
            if (!Settings.UI.SystemTrayPopup.ShowCharNotTraining)
            {
                characters = characters.Where(x => x.IsTraining);
            }

            // Sort
            var charactersList = characters.ToList();
            charactersList.StableSort(new CharacterComparer(Settings.UI.SystemTrayPopup.SecondarySortOrder));
            charactersList.StableSort(new CharacterComparer(Settings.UI.SystemTrayPopup.PrimarySortOrder));

            // Grouping
            List<Character> newCharacters = new List<Character>();
            switch (Settings.UI.SystemTrayPopup.GroupBy)
            {
                case TrayPopupGrouping.None:
                    newCharacters.AddRange(charactersList);
                    return newCharacters;
                
                case TrayPopupGrouping.Account:
					newCharacters.AddRange(charactersList.Where(x => x.Identity.Account != null));
					return newCharacters.GroupBy(x => x.Identity.Account).SelectMany(y => y);

                case TrayPopupGrouping.TrainingAtTop:
                    newCharacters.AddRange(charactersList.Where(x => x.IsTraining));
                    newCharacters.AddRange(charactersList.Where(x => !x.IsTraining));
                    return newCharacters;

                case TrayPopupGrouping.TrainingAtBottom:
                    newCharacters.AddRange(charactersList.Where(x => !x.IsTraining));
                    newCharacters.AddRange(charactersList.Where(x => x.IsTraining));
                    return newCharacters;

                default:
                    return characters;
            }
        }

        /// <summary>
        /// Creates a panel contains the warning message for accounts not in training
        /// </summary>
        /// <param name="warningMessage"></param>
        /// <returns></returns>
        private FlowLayoutPanel CreateAccountsNotTrainingPanel(string warningMessage)
        {
            // Create a flowlayout to hold the content
            FlowLayoutPanel warningPanel = new FlowLayoutPanel();
            warningPanel.AutoSize = true;
            warningPanel.AutoSizeMode = AutoSizeMode.GrowAndShrink;
            warningPanel.Margin = new Padding(0, 0, 0, 2);

            // Add a picture on the left with a warning icon
            if (!Settings.UI.SafeForWork)
            {
                PictureBox pbWarning = new PictureBox();
                pbWarning.Image = SystemIcons.Warning.ToBitmap();
                pbWarning.SizeMode = PictureBoxSizeMode.StretchImage;
                pbWarning.Size = new Size(m_portraitSize[(int)Settings.UI.SystemTrayPopup.PortraitSize], m_portraitSize[(int)Settings.UI.SystemTrayPopup.PortraitSize]);
                pbWarning.Margin = new Padding(2);
                warningPanel.Controls.Add(pbWarning);
            }

            // Adds a label to hold the message
            Label lblMessage = new Label();
            lblMessage.AutoSize = true;
            lblMessage.Text = warningMessage;
            warningPanel.Controls.Add(lblMessage);
            return warningPanel;
        }

        /// <summary>
        /// Completes the layout after new controls have been added.
        /// </summary>
        private void CompleteLayout()
        {
            // Fix the panel widths to the largest.
            // We let the framework determine the appropriate widths, then fix them so that
            // updates to training time remaining don't cause the form to resize.
            int pnlWidth = 0;
            foreach (Control control in mainPanel.Controls)
            {
                if (control.Width > pnlWidth)
                {
                    pnlWidth = control.Width;
                }
            }

            foreach (Control control in mainPanel.Controls)
            {
                if (control is FlowLayoutPanel)
                {
                    FlowLayoutPanel flowPanel = control as FlowLayoutPanel;
                    int pnlHeight = flowPanel.Height;
                    flowPanel.AutoSize = false;
                    flowPanel.Width = pnlWidth;
                    flowPanel.Height = pnlHeight;
                }
            }
            // Position Popup
            TrayIcon.SetToolTipLocation(this);
        }

        /// <summary>
        /// Updates the eve time label
        /// </summary>
        private void UpdateEveTimeLabel()
        {
            if (Settings.UI.SystemTrayPopup.ShowEveTime)
                m_eveTimeLabel.Text = String.Format(CultureConstants.DefaultCulture, "EVE Time: {0:HH:mm}", DateTime.UtcNow);
        }

        /// <summary>
        /// Updates the server status label
        /// </summary>
        private void UpdateServerStatusLabel()
        {
            if (m_serverStatusLabel == null)
                return;

            if (Settings.UI.SystemTrayPopup.ShowServerStatus)
                m_serverStatusLabel.Text = EveClient.EVEServer.StatusText;
        }
        #endregion


        #region Native Stuff
        internal class NativeMethods
        {
            public const Int32 HWND_TOPMOST = -1;
            public const Int32 SWP_NOACTIVATE = 0x0010;
            public const Int32 SWP_NOSIZE = 0x0001;
            public const Int32 SWP_NOMOVE = 0x0002;
            public const Int32 SW_SHOWNOACTIVATE = 4;

            [DllImport("user32.dll")]
            public static extern bool ShowWindow(IntPtr hWnd, Int32 flags);
            [DllImport("user32.dll")]
            public static extern bool SetWindowPos(IntPtr hWnd,
                Int32 hWndInsertAfter, Int32 X, Int32 Y, Int32 cx, Int32 cy, uint uFlags);

        }
        #endregion
    }
}
