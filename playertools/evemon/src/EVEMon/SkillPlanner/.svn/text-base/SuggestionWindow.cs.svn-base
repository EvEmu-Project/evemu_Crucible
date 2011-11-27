using System;
using System.Collections.Generic;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Presents the user with a list of learning skills suggestions for a plan.
    /// </summary>
    public partial class SuggestionWindow : EVEMonForm
    {
        PlanSuggestions m_suggestions;
        private Plan m_plan;

        /// <summary>
        /// Designer constructor
        /// </summary>
        public SuggestionWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Constructor to use in code
        /// </summary>
        /// <param name="plan"></param>
        public SuggestionWindow(Plan plan)
            : this()
        {
            m_plan = plan;
            m_suggestions = m_plan.GetSuggestions();
        }

        /// <summary>
        /// On load, fill the list and update the labels.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void SuggestionWindow_Load(object sender, EventArgs e)
        {
            if (this.DesignMode)
                return;

            // Update the suggestions list
            lbSkills.Items.Clear();
            foreach (var entry in m_suggestions)
            {
                lbSkills.Items.Add(entry.ToString());
            }

            // Update the times labels
            CharacterScratchpad character = m_plan.Character.After(m_plan.ChosenImplantSet);
            TimeSpan preTime = m_plan.GetTotalTime(character, true);
            TimeSpan postTime = preTime.Subtract(m_suggestions.TimeBenefit);

            lblBeforeTime.Text = preTime.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);
            lblAfterTime.Text = postTime.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);
            lblDiffTime.Text = m_suggestions.TimeBenefit.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);
        }

        /// <summary>
        /// On "cancel", nothing special.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        /// <summary>
        /// On "add", apply the suggestions
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnAdd_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            m_suggestions.ApplySuggestions();
            this.Close();
        }
    }
}