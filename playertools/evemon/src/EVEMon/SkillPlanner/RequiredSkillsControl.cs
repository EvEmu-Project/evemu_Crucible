using System;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Data;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// User control to display required skills for a given eveobject and update a plan object for requirements not met
    /// </summary>
    public partial class RequiredSkillsControl : UserControl
    {
        private BlueprintActivity m_activity;
        private Item m_object;
        private Plan m_plan;

        #region Object Lifecycle

        /// <summary>
        /// Default constructor
        /// </summary>
        public RequiredSkillsControl()
        {
            InitializeComponent();

            tvSkillList.MouseDown += new MouseEventHandler(tvSkillList_MouseDown);

            this.Disposed += new EventHandler(OnDisposed);
            EveClient.PlanChanged += new EventHandler<PlanChangedEventArgs>(EveClient_PlanChanged);
        }

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            this.Disposed -= new EventHandler(OnDisposed);
            EveClient.PlanChanged -= new EventHandler<PlanChangedEventArgs>(EveClient_PlanChanged);
        }

        /// <summary>
        /// Occurs when the plan changes, when update the display.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_PlanChanged(object sender, PlanChangedEventArgs e)
        {
            UpdateDisplay();
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// </summary>
        public BlueprintActivity Activity
        {
            get { return m_activity; }
            set
            { 
                m_activity = value;
                UpdateDisplay();
            }
        }

        /// <summary>
        /// An EveObject for which we want to show required skills
        /// </summary>
        public Item Object
        {
            get { return m_object; }
            set
            {
                m_object = value;
                UpdateDisplay();
            }
        }

        /// <summary>
        /// The target Plan object to add any required skills
        /// </summary>
        public Plan Plan
        {
            get { return m_plan; }
            set
            {
                m_plan = value;
                UpdateDisplay();
            }
        }

        #endregion


        #region Content creation

        /// <summary>
        /// Updates control contents
        /// </summary>
        private void UpdateDisplay()
        {
            // We have nothing to display
            if (m_object == null || m_plan == null)
                return;

            // Default all known flag to true. Will be set to false in getSkillNode() if a requirement is not met
            bool allSkillsKnown = true;

            // Default unplanned skills flag to false. Will be set to true in getSkillNode() if a requirement is neither met nor planned
            bool skillsUnplanned = false;

            // Treeview update
            tvSkillList.BeginUpdate();

            var prerequisites = (Activity == BlueprintActivity.None ?
                m_object.Prerequisites.Where(x => !x.Level.Equals(0) && x.Activity.Equals(Activity)) :
                m_object.Prerequisites.Where(x => !x.Level.Equals(0) && x.Activity.Equals(Activity)).OrderBy(x => x.Skill.Name));

            try
            {
                tvSkillList.Nodes.Clear();

                // Recursively create nodes
                foreach (StaticSkillLevel prereq in prerequisites)
                {
                    tvSkillList.Nodes.Add(GetSkillNode(prereq, ref allSkillsKnown, ref skillsUnplanned));
                }
            }
            finally
            {
                tvSkillList.EndUpdate();
            }

            // Set training time required label
            if (allSkillsKnown)
            {
                lblTimeRequired.Text = "No training required";
            }
            else
            {
                TimeSpan trainTime = m_plan.Character.GetTrainingTimeToMultipleSkills(prerequisites);
                lblTimeRequired.Text = trainTime.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);
            }

            // Set minimun control size
            Size timeRequiredTextSize = TextRenderer.MeasureText(lblTimeRequired.Text, Font);
            Size newMinimumSize = new Size(timeRequiredTextSize.Width + btnAddSkills.Width, 0);
            if (this.MinimumSize.Width < newMinimumSize.Width)
                this.MinimumSize = newMinimumSize;

            // Enable / disable button
            btnAddSkills.Enabled = skillsUnplanned;
        }

        /// <summary>
        /// Recursive method to generate treenodes for tvSkillList
        /// </summary>
        /// <param name="requiredSkill">An EntityRequiredSkill object</param>
        /// <returns></returns>
        private TreeNode GetSkillNode(StaticSkillLevel prereq, ref bool allSkillsKnown, ref bool skillsUnplanned)
        {
            var character = (Character)m_plan.Character;
            Skill skill = character.Skills[prereq.Skill];

            TreeNode node = new TreeNode(prereq.ToString());
            node.Tag = new SkillLevel(skill, prereq.Level);

            // Skill requirement met
            if (skill.Level >= prereq.Level)
            {
                node.ImageIndex = 1;
                node.SelectedImageIndex = 1;
            }
            // Requirement not met, but planned
            else if (m_plan.IsPlanned(skill, prereq.Level))
            {
                node.ImageIndex = 2;
                node.SelectedImageIndex = 2;
                allSkillsKnown = false;
            }
            // Requirement not met, but trainable
            else if (skill.Level < prereq.Level && skill.IsKnown)
            {
                node.ImageIndex = 3;
                node.SelectedImageIndex = 3;
                allSkillsKnown = false;
                skillsUnplanned = true;
            }
            // Requirement not met
            else
            {
                node.ImageIndex = 0;
                node.SelectedImageIndex = 0;
                allSkillsKnown = false;
                skillsUnplanned = true;
            }

            // Generate child nodes if required
            foreach (StaticSkillLevel childPrereq in skill.StaticData.Prerequisites)
            {
                node.Nodes.Add(GetSkillNode(childPrereq, ref allSkillsKnown, ref skillsUnplanned));
            }

            return node;
        }

        /// <summary>
        /// When the treeview is clicked, we manually select nodes since the bounding boxes are incorrect.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvSkillList_MouseDown(object sender, MouseEventArgs e)
        {
            // Perform the selection manually since the bound's width and x are incorrect
            TreeNode selection = null;
            for (TreeNode node = tvSkillList.TopNode; node != null; node = node.NextVisibleNode)
            {
                if (node.Bounds.Top <= e.Y && node.Bounds.Bottom >= e.Y)
                {
                    // If the user clicked the "arrow zone", we do not change the selection and just return
                    if (e.X < (node.Bounds.Left - 32))
                        return;

                    selection = node;
                    break;
                }
            }

            // Updates the selection
            if (selection != tvSkillList.SelectedNode)
                tvSkillList.SelectedNode = selection;
        }

        #endregion


        #region Event Handlers

        /// <summary>
        /// Event handler method for Add Skills button
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnAddSkills_Click(object sender, EventArgs e)
        {
            // Add skills to plan
            var operation = m_plan.TryAddSet(m_object.Prerequisites.Where(x => x.Activity.Equals(Activity)), m_object.Name);
            PlanHelper.Perform(operation);

            // Refresh display to reflect plan changes
            UpdateDisplay();
        }

        /// <summary>
        /// Forces the selection update when a node is right-clicked.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvSkillList_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
                tvSkillList.SelectedNode = e.Node;
        }

        /// <summary>
        /// Event handler for treenode double click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvSkillList_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            // Get selected node
            TreeNode thisNode = e.Node as TreeNode;

            // Make sure we have a skill to use
            if (thisNode.Tag == null)
                return;

            // Open skill browser tab for this skill
            PlanWindow pw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
            Skill skill = ((SkillLevel) thisNode.Tag).Skill;
            pw.ShowSkillInBrowser(skill);
        }

        #endregion


        #region Context menu

        /// <summary>
        /// Context menu opening, updates the "plan to" menus
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void contextMenu_Opening(object sender, CancelEventArgs e)
        {
            if (tvSkillList.SelectedNode == null)
            {
                planToMenu.Enabled = false;

                // Update "show in..." menu
                showInMenuSeparator.Visible = false;
                showInSkillBrowserMenu.Visible = false;
                showInSkillExplorerMenu.Visible = false;
            }
            else
            {
                planToMenu.Enabled = true;
                showInMenuSeparator.Visible = true;
                showInSkillBrowserMenu.Visible = true;
                showInSkillExplorerMenu.Visible = true;
                
                // "Plan to N" menus
                var skillLevel = (SkillLevel)tvSkillList.SelectedNode.Tag;
                var skill = skillLevel.Skill;
                for (int i = 0; i <= 5; i++)
                {
                    PlanHelper.UpdatesRegularPlanToMenu(planToMenu.DropDownItems[i], m_plan, skill, i);
                }
            }
        }

        /// <summary>
        /// Context > Show in Skill Browser
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void showInSkillBrowserMenu_Click(object sender, EventArgs e)
        {
            // Retrieve the owner window
            PlanWindow npw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
            if (npw == null || npw.IsDisposed)
                return;

            // Open the skill explorer
            var skillLevel = (SkillLevel)tvSkillList.SelectedNode.Tag;
            npw.ShowSkillInBrowser(skillLevel.Skill);
        }

        /// <summary>
        /// Context > Show in Skill Explorer
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void showInSkillExplorerMenu_Click(object sender, EventArgs e)
        {
            // Retrieve the owner window
            PlanWindow npw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
            if (npw == null || npw.IsDisposed)
                return;

            // Open the skill explorer
            var skillLevel = (SkillLevel)tvSkillList.SelectedNode.Tag;
            npw.ShowSkillInExplorer(skillLevel.Skill);
        }

        /// <summary>
        /// Treeview's context menu > Expand All
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmExpandAll_Click(object sender, EventArgs e)
        {
            tvSkillList.ExpandAll();
        }

        /// <summary>
        /// Treeview's context menu > Collapse All
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmCollapseAll_Click(object sender, EventArgs e)
        {
            tvSkillList.CollapseAll();
        }

        /// <summary>
        /// Context > Plan To > Level N
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void planToMenuItem_Click(object sender, EventArgs e)
        {
            var menu = (ToolStripMenuItem)sender;
            var operation = (IPlanOperation)menu.Tag;
            PlanHelper.SelectPerform(operation);
        }

        #endregion

    }
}