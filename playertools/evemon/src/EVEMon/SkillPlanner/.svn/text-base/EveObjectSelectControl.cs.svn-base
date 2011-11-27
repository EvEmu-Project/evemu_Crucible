using System;
using System.Linq;
using System.Windows.Forms;
using System.ComponentModel;
using System.Collections.Generic;

using EVEMon.Common;
using EVEMon.Common.Data;
using EVEMon.Common.Controls;

namespace EVEMon.SkillPlanner
{
    public partial class EveObjectSelectControl : UserControl
    {
        public event EventHandler SelectionChanged;

        protected Func<Item, Boolean> m_usabilityPredicate;
        protected List<Item> m_selectedObjects = null;
        protected ObjectActivityFilter m_activityFilter;
        protected BlueprintActivity m_activity;
        protected Plan m_plan;

        protected bool m_allExpanded;

        /// <summary>
        /// Constructor.
        /// </summary>
        public EveObjectSelectControl()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Gets or sets the plan.
        /// </summary>
        public Plan Plan
        {
            get { return m_plan; }
            set { m_plan = value; }
        }

        /// <summary>
        /// Occurs when the control is loaded.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void EveObjectSelectControl_Load(object sender, EventArgs e)
        {
            m_usabilityPredicate = SelectAll;

            if (DesignMode || this.IsDesignModeHosted())
                return;

            // Subscribe the events
            EveClient.SettingsChanged += EveClient_SettingsChanged;
            Disposed += OnDisposed;

            // Update the controls
            EveClient_SettingsChanged(null, EventArgs.Empty);
        }

        /// <summary>
        /// Handles the SettingsChanged event of the EveClient control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            pbSearchImage.Visible = !Settings.UI.SafeForWork;
        }

        /// <summary>
        /// Called when [disposed].
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.SettingsChanged -= EveClient_SettingsChanged;
            Disposed -= OnDisposed;
        }

        #region Search
        /// <summary>
        /// Occurs when clicking on the search text control.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void lbSearchTextHint_Click(object sender, EventArgs e)
        {
            tbSearchText.Focus();
        }

        /// <summary>
        /// Occurs upon entering the search text control.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void tbSearchText_Enter(object sender, EventArgs e)
        {
            lbSearchTextHint.Visible = false;
        }

        /// <summary>
        /// Occurs upon leaving the search text control.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void tbSearchText_Leave(object sender, EventArgs e)
        {
            lbSearchTextHint.Visible = String.IsNullOrEmpty(tbSearchText.Text);
        }

        /// <summary>
        /// Occurs when the search text changed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void tbSearchText_TextChanged(object sender, EventArgs e)
        {
            if (!tbSearchText.Focused)
                lbSearchTextHint.Visible = String.IsNullOrEmpty(tbSearchText.Text);

            BuildListView();
        }

        /// <summary>
        /// Parses the tree node and extracts all the items to build the content of the list box. 
        /// It also deals with text filtering and the treeview/listbox visibility.
        /// </summary>
        protected void BuildListView()
        {

            string searchText = tbSearchText.Text.Trim().ToLower(CultureConstants.DefaultCulture);

            if (String.IsNullOrEmpty(searchText))
            {
                tvItems.Visible = true;
                lbSearchList.Visible = false;
                lbNoMatches.Visible = false;
                return;
            }

            // Find everything in the current tree that matches the search string
            List<Item> filteredItems = new List<Item>();
            foreach (TreeNode n in tvItems.Nodes)
            {
                SearchNode(n, searchText, filteredItems);
            }
            filteredItems.Sort((x, y) => String.CompareOrdinal(x.Name, y.Name));

            lbSearchList.BeginUpdate();
            try
            {
                lbSearchList.Items.Clear();
                if (filteredItems.Count > 0)
                {
                    foreach (Item eo in filteredItems)
                    {
                        lbSearchList.Items.Add(eo);
                    }
                }
            }
            finally
            {
                lbSearchList.EndUpdate();
            }
            lbSearchList.Visible = true;
            tvItems.Visible = false;
            lbNoMatches.Visible = (filteredItems.Count == 0);
        }

        /// <summary>
        /// Creates the node.
        /// </summary>
        /// <param name="tn"></param>
        /// <param name="searchText"></param>
        /// <param name="filteredItems"></param>
        private void SearchNode(TreeNode tn, string searchText, List<Item> filteredItems)
        {
            Item itm = tn.Tag as Item;
            if (itm == null)
            {
                foreach (TreeNode subNode in tn.Nodes)
                {
                    SearchNode(subNode, searchText, filteredItems);
                }
                return;
            }

            if (itm.Name.ToLower(CultureConstants.DefaultCulture).Contains(searchText) || itm.Description.ToLower(CultureConstants.DefaultCulture).Contains(searchText))
            {
                filteredItems.Add(itm);
            }
        }

        /// <summary>
        /// Occurs when pressing a key while inside the search text control.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tbSearchText_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 0x01)
            {
                tbSearchText.SelectAll();
                e.Handled = true;
            }
        }
        #endregion


        #region Selected Objects
        /// <summary>
        /// All the selected objects (through multi-select)
        /// </summary>
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        [DefaultValue(null)]
        [Browsable(false)]
        public List<Item> SelectedObjects
        {
            get { return m_selectedObjects; }
        }

        /// <summary>
        /// The primary selected object
        /// </summary>
        [Browsable(false)]
        public Item SelectedObject
        {
            get
            {
                if (m_selectedObjects == null || m_selectedObjects.Count == 0)
                    return null;

                return m_selectedObjects[0];
            }
            set
            {
                List<Item> selectedObjects = new List<Item>();
                if (value != null)
                    selectedObjects.Add(value);

                SetSelectedObjects(selectedObjects);
            }
        }

        /// <summary>
        /// Selects the given nodes.
        /// </summary>
        /// <param name="s"></param>
        protected void SetSelectedObjects(IEnumerable<Item> s)
        {
            // Updates selection
            m_selectedObjects = (s == null ? new List<Item>() : new List<Item>(s));

            // Selects the proper nodes
            if (m_selectedObjects.Count == 1)
            {
                // If the object is not already selected
                var obj = m_selectedObjects[0];
                tvItems.SelectNodeWithTag(obj);
            }

            // Notify subscribers
            if (SelectionChanged != null)
                SelectionChanged(this, new EventArgs());
        }

        /// <summary>
        /// Update the selected tree node.
        /// </summary>
        private void UpdateSelectedTreeNodes()
        {
            if (tvItems.SelectedNodes.Count != 0)
            {
                List<Item> selectedObjects = new List<Item>();
                foreach (TreeNode node in tvItems.SelectedNodes)
                {
                    var obj = node.Tag as Item;
                    if (obj != null)
                        selectedObjects.Add(obj);
                }
                SetSelectedObjects(selectedObjects);
            }
            else
            {
                SetSelectedObjects(null);
            }
        }
        #endregion


        #region Events

        /// <summary>
        /// Occurs when selection changed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvItems_SelectionsChanged(object sender, EventArgs e)
        {
            UpdateSelectedTreeNodes();
        }

        /// <summary>
        /// Occurs when the search list selection changed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbSearchList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lbSearchList.SelectedItems.Count != 0)
            {
                List<Item> selectedObjects = new List<Item>();
                foreach (Object node in lbSearchList.SelectedItems)
                {
                    var obj = node as Item;
                    if (obj != null)
                        selectedObjects.Add(obj);
                }
                SetSelectedObjects(selectedObjects);
            }
            else
            {
                SetSelectedObjects(null);
            }
        }

        /// <summary>
        /// Treeview's context menu > Expand all.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmiExpandAll_Click(object sender, EventArgs e)
        {
            tvItems.ExpandAll();
            m_allExpanded = true;
        }

        /// <summary>
        /// Treeview's context menu > Collapse all.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmiCollapseAll_Click(object sender, EventArgs e)
        {
            tvItems.CollapseAll();
            m_allExpanded = false;
        }

        /// <summary>
        /// Treeview's context menu > Expand.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmiExpandSelected_Click(object sender, EventArgs e)
        {
            tvItems.SelectedNode.Expand();
        }

        /// <summary>
        /// Treeview's context menu > Collapse.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmiCollapseSelected_Click(object sender, EventArgs e)
        {
            tvItems.SelectedNode.Collapse();
        }

        /// <summary>
        /// Occurs upon opening the context menu.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void contextMenu_Opening(object sender, CancelEventArgs e)
        {
            var node = tvItems.SelectedNode;

            tsSeparator.Visible = (node != null && node.GetNodeCount(true) > 0);

            // "Expand" and "Collapse" selected menu
            cmiExpandSelected.Visible = (node != null && node.GetNodeCount(true) > 0 && !node.IsExpanded);
            cmiCollapseSelected.Visible = (node != null && node.GetNodeCount(true) > 0 && node.IsExpanded);

            cmiExpandSelected.Text = (node != null && node.GetNodeCount(true) > 0 && !node.IsExpanded ?
                String.Format("Expand {0}", node.Text) : String.Empty);
            cmiCollapseSelected.Text = (node != null && node.GetNodeCount(true) > 0 && node.IsExpanded ?
                String.Format("Collapse {0}", node.Text) : String.Empty);

            // "Expand All" and "Collapse All" menu
            cmiCollapseAll.Enabled = cmiCollapseAll.Visible = m_allExpanded;
            cmiExpandAll.Enabled = cmiExpandAll.Visible = !cmiCollapseAll.Enabled;
        }

        #endregion


        #region Predicates
        /// <summary>
        /// Filter for all items
        /// </summary>
        /// <param name="eo"></param>
        /// <returns></returns>
        protected bool SelectAll(Item eo)
        {
            return true;
        }

        /// <summary>
        /// Filter for items which can be used (prereqs met)
        /// </summary>
        /// <param name="eo"></param>
        /// <returns></returns>
        protected bool CanUse(Item eo)
        {
            var prerequisites = eo.Prerequisites.Where(x => !x.Activity.Equals(BlueprintActivity.ReverseEngineering));
            bool hasSelectedActivity = true;
            bool bpBrowserControl = this is BlueprintSelectControl;

            // Is item a blueprint and supports the selected activity ?  
            if (bpBrowserControl)
            {
                hasSelectedActivity = prerequisites.Any(x => x.Activity.Equals(m_activity))
                       || ((Blueprint)eo).MaterialRequirements.Any(x => x.Activity.Equals(m_activity));

                // Can not be used when item doesn't support the selected activity
                if ((m_activityFilter.Equals(ObjectActivityFilter.Manufacturing) || m_activityFilter.Equals(ObjectActivityFilter.Invention))
                    && !hasSelectedActivity)
                    return false;

                // Enumerates the prerequisites skills to the selected activity 
                if (!m_activityFilter.Equals(ObjectActivityFilter.All) && !m_activityFilter.Equals(ObjectActivityFilter.Any))
                    prerequisites = prerequisites.Where(x => x.Activity.Equals(m_activity));
            }

            // Item doesn't have prerequisites skills
            if (prerequisites.IsEmpty())
                return true;

            // Is this the "Blueprint Browser" and the activity filter is set to "Any" ?
            List<Boolean> prereqTrained = new List<Boolean>();
            if (bpBrowserControl && m_activityFilter.Equals(ObjectActivityFilter.Any))
            {
                List<BlueprintActivity> prereqActivity = new List<BlueprintActivity>();

                // Create a list with the activities this item supports
                foreach (var prereq in prerequisites.Where(x => !prereqActivity.Contains(x.Activity)))
                {
                    prereqActivity.Add(prereq.Activity);
                }

                // Create a list with each prereq skill trained status for the questioned activity
                foreach (var activity in prereqActivity)
                {
                    prereqTrained.Clear();

                    foreach (var prereq in prerequisites.Where(x => x.Activity.Equals(activity)))
                    {
                        int level = m_plan.Character.GetSkillLevel(prereq.Skill);
                        prereqTrained.Add(level >= prereq.Level);
                    }

                    // Has the character trained all prereq skills for this activity ?
                    if (prerequisites.IsEmpty() || prereqTrained.All(x => x.Equals(true)))
                        return true;
                }
                return false;
            }
            // Do a simple predication
            else
            {
                // Create a list with each prereq skill trained status
                foreach (var prereq in prerequisites)
                {
                    int level = m_plan.Character.GetSkillLevel(prereq.Skill);
                    prereqTrained.Add(level >= prereq.Level);
                }
            }

            // Has the character trained all prereq skills ?
            return (prereqTrained.All(x => x == true));
        }

        /// <summary>
        /// Filter for items which can not be used (prereqs not met).
        /// </summary>
        /// <param name="eo"></param>
        /// <returns></returns>
        protected bool CannotUse(Item eo)
        {
            return !CanUse(eo);
        }
        #endregion

    }
}
