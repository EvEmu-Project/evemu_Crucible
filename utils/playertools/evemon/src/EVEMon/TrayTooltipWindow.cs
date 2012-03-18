using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Controls;

namespace EVEMon
{
    /// <summary>
    /// Displays a Windows-style tooltip
    /// </summary>
    public partial class TrayTooltipWindow : Form
    {
        private List<Character> m_characters = new List<Character>();
        private String m_tooltipFormat = String.Empty;
        private bool m_updatePending;

        /// <summary>
        /// Designer constructor
        /// </summary>
        public TrayTooltipWindow() 
        {
            InitializeComponent();
            this.Font = FontFactory.GetFont(SystemFonts.MessageBoxFont.Name, SystemFonts.MessageBoxFont.SizeInPoints, FontStyle.Regular, GraphicsUnit.Point);

            EveClient.MonitoredCharacterCollectionChanged += new EventHandler(EveClient_MonitoredCharacterCollectionChanged);
            EveClient.CharacterChanged += new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterChanged);
        }

        /// <summary>
        /// On close, stops the timer.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosed(EventArgs e)
        {
            EveClient.MonitoredCharacterCollectionChanged -= new EventHandler(EveClient_MonitoredCharacterCollectionChanged);
            EveClient.CharacterChanged -= new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterChanged);
            displayTimer.Stop();
            base.OnClosed(e);
        }

        /// <summary>
        /// On load, update controls
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            if (this.DesignMode) return;

            UpdateCharactersList();
        }

        /// <summary>
        /// When the window is shown, sets it as topmost without activation.
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
        /// On visible, checks whether an update is pending.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            if (this.Visible && m_updatePending)
                UpdateContent();

            base.OnVisibleChanged(e);
        }


        #region Content creation and refresh
        /// <summary>
        /// Updates the tooltip format we use as a base for update on every second, along with the characters list.
        /// </summary>
        private void UpdateCharactersList()
        {
            // Update characters list and selects display order
            m_characters.Clear();
            if (Settings.UI.SystemTrayTooltip.DisplayOrder)
            {    
                m_characters.AddRange(TrayPopUpWindow.GetCharacters());
            }
            else
            {
                m_characters.AddRange(EveClient.MonitoredCharacters.Where(x => x.IsTraining));
            } 

            // Assembles the tooltip format
            StringBuilder sb = new StringBuilder();
            if (String.IsNullOrEmpty(Settings.UI.SystemTrayTooltip.Format))
            {
                // Bad tooltip format
                displayTimer.Enabled = false;
                sb.Append("You can configure this tooltip in the options/general panel");
            }
            else if (m_characters.Count == 0)
            {
                // No character in training
                displayTimer.Enabled = false;
                sb.Append("No Characters in training!");
            }
            else
            {
                // Start the display timer
                displayTimer.Enabled = true;
                displayTimer.Start();

                // Assemble tooltip base format with character informations
                foreach (Character character in m_characters)
                {
                    if (sb.Length != 0)
                        sb.Append("\n");

                    sb.Append(FormatTooltipText(Settings.UI.SystemTrayTooltip.Format, character));
                }
            }
            m_tooltipFormat = sb.ToString();

            // Update the tooltip's content
            UpdateContent();
        }

        /// <summary>
        /// Updates the tooltip's content.
        /// </summary>
        private void UpdateContent()
        {
            if (!this.Visible)
            {
                m_updatePending = true;
                return;
            }
            m_updatePending = false;

            // Replaces the fragments like "%10546464r" (the number being the character ID) by the remaining time.
            string tooltip = m_tooltipFormat;
            foreach (var character in m_characters)
            {
                if (character.IsTraining)
                {
                    var trainingSkill = character.CurrentlyTrainingSkill;
                    TimeSpan remainingTime = trainingSkill.EndTime.Subtract(DateTime.UtcNow);

                    tooltip = Regex.Replace(tooltip,
                        '%' + character.CharacterID.ToString() + 'r',
                        remainingTime.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas),
                        RegexOptions.Compiled);
                }

                var ccpCharacter = character as CCPCharacter;
                if (ccpCharacter != null && ccpCharacter.SkillQueue.IsPaused)
                {                    
                    tooltip = Regex.Replace(tooltip,
                        '%' + character.CharacterID.ToString() + 'r', "(Paused)",
                        RegexOptions.Compiled);
                }
            }

            // Updates the tooltip and its location
            lblToolTip.Text = tooltip.ToString();
            TrayIcon.SetToolTipLocation(this);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="toolTipFormat"></param>
        /// <param name="character"></param>
        /// <returns></returns>
        private string FormatTooltipText(string toolTipFormat, Character character)
        {
            StringBuilder sb = new StringBuilder();

            sb.Append(Regex.Replace(toolTipFormat, "%([nbsdr]|[ct][ir])", new MatchEvaluator(delegate(Match m)
            {
                int level = -1;

                // First group
                switch (m.Groups[1].Value[0])
                {
                    case 'n':
                        return character.Name;
                    case 'b':
                        return character.Balance.ToString("N2");
                }

                var ccpCharacter = character as CCPCharacter;
                if (ccpCharacter != null && (ccpCharacter.IsTraining || ccpCharacter.SkillQueue.IsPaused))
                {
                    switch (m.Groups[1].Value[0])
                    {
                        case 'r':
                            return '%' + character.CharacterID.ToString() + 'r';
                        case 's':
                            return character.CurrentlyTrainingSkill.SkillName;
                        case 'd':
                            return character.CurrentlyTrainingSkill.EndTime.ToString("g");
                        case 'c':
                            level = character.CurrentlyTrainingSkill.Level - 1;
                            break;
                        case 't':
                            level = character.CurrentlyTrainingSkill.Level;
                            break;
                        default:
                            return String.Empty;
                    }

                    // Second group
                    if (level >= 0 && m.Groups[1].Value.Length > 1)
                    {
                        switch (m.Groups[1].Value[1])
                        {
                            case 'i':
                                return level.ToString();
                            case 'r':
                                return Skill.GetRomanForInt(level);
                            default:
                                return String.Empty;
                        }
                    }
                }

                return String.Empty;
            }), RegexOptions.Compiled));

            return sb.ToString();
        }
        #endregion


        #region Timer and global events
        /// <summary>
        /// When a character changes (skill completed, now data from CCP, etc), update the characters list
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            UpdateCharactersList();
        }

        /// <summary>
        /// Whenever the monitored characters collection changes, update the characters list.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_MonitoredCharacterCollectionChanged(object sender, EventArgs e)
        {
            UpdateCharactersList();
        }

        /// <summary>
        /// Every second, when characters are in training, we update the tooltip
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void displayTimer_Tick(object sender, EventArgs e)
        {
            UpdateContent();
        }

        #endregion


        #region Native Stuff
        private static class NativeMethods
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

