using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.SkillPlanner
{
    public partial class SkillBrowser : UserControl
    {
        private const DescriptiveTextOptions TimeOptions = DescriptiveTextOptions.IncludeCommas | DescriptiveTextOptions.UppercaseText;

        private Skill m_selectedSkill;
        private Plan m_plan;


        /// <summary>
        /// Constructor
        /// </summary>
        public SkillBrowser()
        {
            InitializeComponent();
            verticalSplitContainer.RememberDistanceKey = "SkillBrowser_Vertical";
        }

        /// <summary>
        /// On load.
        /// </summary>
        /// <param name="e">An <see cref="T:System.EventArgs"/> that contains the event data.</param>
        protected override void OnLoad(EventArgs e)
        {
            lblSkillName.Font = FontFactory.GetFont("Tahoma", 8.25F, FontStyle.Bold, GraphicsUnit.Point);
            
            // Reposition the help text along side the treeview
            Control[] result = skillSelectControl.Controls.Find("pnlResults", true);
            if (result.Length > 0)
                lblHelp.Location = new Point(lblHelp.Location.X, result[0].Location.Y);

            skillTreeDisplay.SkillClicked += skillTreeDisplay_SkillClicked;
            
            EveClient.SettingsChanged += EveClient_SettingsChanged;
            EveClient.PlanChanged += EveClient_PlanChanged;
            Disposed += OnDisposed;

            //Update the controls
            EveClient_SettingsChanged(null, EventArgs.Empty);

            base.OnLoad(e);
        }

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            skillTreeDisplay.SkillClicked -= skillTreeDisplay_SkillClicked;
            EveClient.PlanChanged -= EveClient_PlanChanged;
            EveClient.SettingsChanged -= EveClient_SettingsChanged;
            Disposed -= OnDisposed;
        }


        /// <summary>
        /// Gets or sets the plan this control is bound to.
        /// </summary>
        public Plan Plan
        {
            get { return m_plan; }
            set
            {
                m_plan = value;
                skillTreeDisplay.Plan = value;
                skillSelectControl.Plan = value;
                UpdateContent();
            }
        }

        /// <summary>
        /// Gets or sets the selected skills.
        /// </summary>
        public Skill SelectedSkill
        {
            get { return m_selectedSkill; }
            set
            {
                if (m_selectedSkill == value) return;

                m_selectedSkill = value;
                skillTreeDisplay.RootSkill = value;
                skillSelectControl.SelectedSkill = value;
                UpdateContent();
            }
        }

        /// <summary>
        /// Show the given skill in the skill explorer.
        /// </summary>
        /// <param name="skill"></param>
        public void ShowSkillInExplorer(Skill skill)
        {
            var planWindow = WindowsFactory<PlanWindow>.GetByTag(m_plan);
            var skillExplorer = WindowsFactory<SkillExplorerWindow>.ShowByTag(planWindow, (window) => new SkillExplorerWindow(skill, window));
            skillExplorer.Skill = skill;
        }


        #region Content update
        /// <summary>
        /// Occurs whenever a plan is changed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_PlanChanged(object sender, PlanChangedEventArgs e)
        {
            UpdatePlannedLevel();
        }

        /// <summary>
        /// Occurs whenever the settings changed.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            if (!Settings.UI.SafeForWork)
            {
                planToMenu.DisplayStyle = ToolStripItemDisplayStyle.ImageAndText;
                showSkillExplorerMenu.DisplayStyle = ToolStripItemDisplayStyle.ImageAndText;
            }
            else
            {
                planToMenu.DisplayStyle = ToolStripItemDisplayStyle.Text;
                showSkillExplorerMenu.DisplayStyle = ToolStripItemDisplayStyle.Text;
            }
        }

        /// <summary>
        /// Updates the combo on the top right, to change the planned level.
        /// </summary>
        private void UpdatePlannedLevel()
        {
            // Not visible
            if (m_selectedSkill == null)
                return;

            // Toolbar > Planned to... dropdown menu
            bool enabled = false;
            enabled |= PlanHelper.UpdatesRegularPlanToMenu(planTo0Menu, m_plan, m_selectedSkill, 0);
            enabled |= PlanHelper.UpdatesRegularPlanToMenu(planTo1Menu, m_plan, m_selectedSkill, 1);
            enabled |= PlanHelper.UpdatesRegularPlanToMenu(planTo2Menu, m_plan, m_selectedSkill, 2);
            enabled |= PlanHelper.UpdatesRegularPlanToMenu(planTo3Menu, m_plan, m_selectedSkill, 3);
            enabled |= PlanHelper.UpdatesRegularPlanToMenu(planTo4Menu, m_plan, m_selectedSkill, 4);
            enabled |= PlanHelper.UpdatesRegularPlanToMenu(planTo5Menu, m_plan, m_selectedSkill, 5);
            planToMenu.Enabled = enabled;

            // Toolbar > "Planned to" label
            int level = m_plan.GetPlannedLevel(m_selectedSkill);

            planToMenu.Text = (level == 0 ? "Plan To (none)..." :
                String.Format(CultureConstants.DefaultCulture, "Plan To Level {0}...", Skill.GetRomanForInt(level)));
        }

        /// <summary>
        /// Updates the browser's content.
        /// </summary>
        public void UpdateContent()
        {
            if (m_selectedSkill == null)
            {
                // View help message
                lblHelp.Visible = true;
                
                rightPanel.Visible = false;
                return;
            }

            // Hide help message
            lblHelp.Visible = false;

            // Updates controls visibility
            rightPanel.Visible = true;

            // Updates the main labels
            lblSkillClass.Text = m_selectedSkill.Group.Name;
            lblSkillName.Text = String.Format(CultureConstants.DefaultCulture, "{0} ({1})",
                                                m_selectedSkill.Name,
                                                m_selectedSkill.Rank);
            lblSkillCost.Text = String.Format(CultureConstants.DefaultCulture, "{0} ISK", m_selectedSkill.FormattedCost);
            descriptionTextBox.Text = m_selectedSkill.Description;
            if (!m_selectedSkill.IsPublic)
            {
                descriptionTextBox.Text += " ** THIS IS A NON-PUBLIC SKILL **";
            }
            lblAttributes.Text = String.Format(CultureConstants.DefaultCulture, "Primary: {0}, Secondary: {1} (SP/Hour: {2:#,##0})",
                                                    m_selectedSkill.PrimaryAttribute.ToString(),
                                                    m_selectedSkill.SecondaryAttribute.ToString(),
                                                    m_selectedSkill.SkillPointsPerHour);
            // Training time per level
            UpdateLevelLabel(lblLevel1Time, 1);
            UpdateLevelLabel(lblLevel2Time, 2);
            UpdateLevelLabel(lblLevel3Time, 3);
            UpdateLevelLabel(lblLevel4Time, 4);
            UpdateLevelLabel(lblLevel5Time, 5);

            // Update "owned" checkbox
            if (m_selectedSkill.IsKnown)
            {
                ownsBookMenu.Checked = false;
                ownsBookMenu.Enabled = false;
            }
            else
            {
                ownsBookMenu.Checked = m_selectedSkill.IsOwned;
                ownsBookMenu.Enabled = true;
            }

            // Update "planned level" combo (on the top left)
            UpdatePlannedLevel();

        }

        /// <summary>
        /// Updates the provided label with the training time to the given level.
        /// </summary>
        /// <param name="label"></param>
        /// <param name="level"></param>
        private void UpdateLevelLabel(Label label, int level)
        {
            // "Level III :"
            StringBuilder sb = new StringBuilder();

            sb.AppendFormat(CultureConstants.DefaultCulture, "Level {0}: ",Skill.GetRomanForInt(level));

            // Is it already trained ?
            if (m_selectedSkill.Level >= level)
            {
                label.Text = sb.Append("Already trained").ToString();
                return;
            }

            // Left training time for level only
            bool isPlanned = m_plan.IsPlanned(m_selectedSkill, level);
            TimeSpan timeOfLevelOnly = m_selectedSkill.GetLeftTrainingTimeForLevelOnly(level);
            sb.Append(timeOfLevelOnly.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas, false));

            // Total training time or completion percentage
            TimeSpan timeForPrereqs = m_selectedSkill.Character.GetTrainingTimeToMultipleSkills(m_selectedSkill.Prerequisites);
            TimeSpan totalPrereqTime = m_selectedSkill.GetLeftTrainingTimeToLevel(level - 1).Add(timeForPrereqs);
            if (totalPrereqTime > TimeSpan.Zero)
            {
                sb.AppendFormat(CultureConstants.DefaultCulture, " (plus {0})",
                    totalPrereqTime.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas));
            }
            else
            {
                // Completion percentage
                if (m_selectedSkill.Level != 5)
                {
                    float percentDone = m_selectedSkill.FractionCompleted;
                    sb.AppendFormat(CultureConstants.DefaultCulture, " ({0} complete)", percentDone.ToString("P0"));
                }
            }

            label.Text = sb.ToString();
        }
        #endregion


        #region Controls events handlers
        /// <summary>
        /// Every 30s, we update the time for the training skill
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tmrTrainingUpdateTick(object sender, EventArgs e)
        {
            UpdateContent();
        }

        /// <summary>
        /// Whenever the selection changes, we update the selected skill
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void skillSelectControl_SelectedSkillChanged(object sender, EventArgs e)
        {
            SelectedSkill = skillSelectControl.SelectedSkill;
        }

        /// <summary>
        /// Toolbar > Owns book
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ownsBookMenu_Click(object sender, EventArgs e)
        {
            m_selectedSkill.IsOwned = ownsBookMenu.Checked;
            UpdateContent();
            skillSelectControl.UpdateContent();
        }

        /// <summary>
        /// Toolbar > Show what this skills enables...
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void showSkillExplorerMenu_Click(object sender, EventArgs e)
        {
            ShowSkillInExplorer(m_selectedSkill);
        }
        #endregion


        #region Skill tree's context menu
        /// <summary>
        /// Whenever the user right-click the skill tree on the left, we display the context menu.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void skillTreeDisplay_SkillClicked(object sender, SkillClickedEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                bool isPlanned = false;
                isPlanned |= PlanHelper.UpdatesRegularPlanToMenu(miPlanTo0, m_plan, e.Skill, 0);
                isPlanned |= PlanHelper.UpdatesRegularPlanToMenu(miPlanTo1, m_plan, e.Skill, 1);
                isPlanned |= PlanHelper.UpdatesRegularPlanToMenu(miPlanTo2, m_plan, e.Skill, 2);
                isPlanned |= PlanHelper.UpdatesRegularPlanToMenu(miPlanTo3, m_plan, e.Skill, 3);
                isPlanned |= PlanHelper.UpdatesRegularPlanToMenu(miPlanTo4, m_plan, e.Skill, 4);
                isPlanned |= PlanHelper.UpdatesRegularPlanToMenu(miPlanTo5, m_plan, e.Skill, 5);
                cmsSkillContext.Show(skillTreeDisplay, e.Location);
            }
            else
            {
                SelectedSkill = e.Skill;
            }
        }

        /// <summary>
        /// Updates the given "Plan to N" menu item.
        /// </summary>
        /// <param name="menu"></param>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        private bool UpdatePlanToMenuItem(ToolStripMenuItem menu, Skill skill, int level)
        {
            bool isKnown = (level <= skill.Level);
            bool isPlanned = m_plan.IsPlanned(skill, level);
            bool isTraining = skill.IsTraining;
            menu.Tag = skill;

            // Already planned ?
            if (isPlanned)
            {
                menu.Enabled = false;
                return true;
            }

            // Level already known
            if (isKnown)
            {
                menu.Enabled = false;
                return false;
            }

            // Output with prerequisites : "Plan to level V (5d 3h 15m)" 
            menu.Enabled = true;
            return false;
        }

        /// <summary>
        /// Context menu > Plan to N / Remove
        /// Toolbar > Plan to... > Level N / Remove
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void planToMenu_Click(object sender, EventArgs e)
        {
            var operation = ((ToolStripMenuItem)sender).Tag as IPlanOperation;
            PlanHelper.SelectPerform(operation);
        }
        #endregion
    }
}
