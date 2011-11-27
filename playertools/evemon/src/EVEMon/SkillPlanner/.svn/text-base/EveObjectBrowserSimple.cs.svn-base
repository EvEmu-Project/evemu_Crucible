using System;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Superclass for simple one page item browsers.
    /// Extends EveObjectBrowserControl with the addition of Description and Required Skills panes
    /// </summary>
    public partial class EveObjectBrowserSimple : EveObjectBrowserControl
    {
        #region Constructors
        /// <summary>
        /// Default constructor
        /// </summary>
        public EveObjectBrowserSimple()
        {
            InitializeComponent();
        }

        #endregion

        #region Event Handlers
        /// <summary>
        /// Updates Required Skills control when selected plan is changed
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveObjectBrowserSimple_PlanChanged(object sender, EventArgs e)
        {
            requiredSkillsControl.Plan = this.Plan;
        }

        #endregion

        #region Methods
        /// <summary>
        /// Updates the controls when the selection is changed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected override void OnSelectionChanged(object sender, EventArgs e)
        {
            base.OnSelectionChanged(sender, e);
            if (SelectedObject == null) return;

            // Description
            tbDescription.Text = SelectedObject.Description;

            // Required Skills
            requiredSkillsControl.Object = SelectedObject;
        }

        protected override void OnPlanChanged()
        {
            base.OnPlanChanged();
            requiredSkillsControl.Plan = Plan;

            // We recalculate the right panels minimum size
            int reqSkillControlMinWidth = requiredSkillsControl.MinimumSize.Width;
            int reqSkillPanelMinWidth = scDetails.Panel2MinSize;
            scDetails.Panel2MinSize = (reqSkillPanelMinWidth > reqSkillControlMinWidth ?
                                         reqSkillPanelMinWidth : reqSkillControlMinWidth );
        }
        #endregion
    }
}

