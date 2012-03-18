using System;
using System.Text;
using System.Linq;
using System.Windows.Forms;
using System.Collections.Generic;


using EVEMon.Common;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common.Data;

namespace EVEMon.SkillPlanner
{
    public partial class BlueprintSelectControl : EveObjectSelectControl
    {
        private Func<Item, Boolean> m_metaGroupPredicate = (x) => true;

        #region Initialisation

        public BlueprintSelectControl()
        {
            InitializeComponent();
        }

        /// <summary>
        /// On load, we read the settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected override void EveObjectSelectControl_Load(object sender, EventArgs e)
        {
            // Return on design mode
            if (DesignMode || this.IsDesignModeHosted())
                return;

            // Call the base method
            base.EveObjectSelectControl_Load(sender, e);

            // Build the blueprints tree
            BuildTreeView();

            // Initialize the "filter" combo box
            cbUsabilityFilter.Items[0] = "All Blueprints";
            cbUsabilityFilter.Items[1] = "Blueprints I can use";
            cbUsabilityFilter.Items[2] = "Blueprints I cannot use";
            
            // Read the settings
            if (Settings.UI.UseStoredSearchFilters)
            {
                cbUsabilityFilter.SelectedIndex = (int)Settings.UI.BlueprintBrowser.UsabilityFilter;
                cbActivityFilter.SelectedIndex = (int)Settings.UI.BlueprintBrowser.ActivityFilter;           
                tbSearchText.Text = Settings.UI.BlueprintBrowser.TextSearch;
                lbSearchTextHint.Visible = String.IsNullOrEmpty(tbSearchText.Text);

                cbTech1.Checked = (Settings.UI.BlueprintBrowser.MetagroupFilter & ItemMetaGroup.T1) != ItemMetaGroup.Empty;
                cbTech2.Checked = (Settings.UI.BlueprintBrowser.MetagroupFilter & ItemMetaGroup.T2) != ItemMetaGroup.Empty;
                cbTech3.Checked = (Settings.UI.BlueprintBrowser.MetagroupFilter & ItemMetaGroup.T3) != ItemMetaGroup.Empty;
                cbFaction.Checked = (Settings.UI.BlueprintBrowser.MetagroupFilter & ItemMetaGroup.Faction) != ItemMetaGroup.Empty;
                cbStoryline.Checked = (Settings.UI.BlueprintBrowser.MetagroupFilter & ItemMetaGroup.Storyline) != ItemMetaGroup.Empty;
                cbOfficer.Checked = (Settings.UI.BlueprintBrowser.MetagroupFilter & ItemMetaGroup.Officer) != ItemMetaGroup.Empty;
            }
            else
            {
                cbUsabilityFilter.SelectedIndex = 0;
                cbActivityFilter.SelectedIndex = 0;
            }
        }

        #endregion


        #region Event Handlers

        /// <summary>
        /// When the combo for filter changes, we update the settings and the control content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbUsabilityFilter_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Update settings
            Settings.UI.BlueprintBrowser.UsabilityFilter = (ObjectUsabilityFilter)cbUsabilityFilter.SelectedIndex;

            // Enable/Disable the activity filter
            cbActivityFilter.Enabled = Settings.UI.BlueprintBrowser.UsabilityFilter != ObjectUsabilityFilter.All;

            // Update the filter delegate
            switch (Settings.UI.BlueprintBrowser.UsabilityFilter)
            {
                case ObjectUsabilityFilter.All:
                    m_usabilityPredicate = SelectAll;
                    break;

                case ObjectUsabilityFilter.Usable:
                    m_usabilityPredicate = CanUse;
                    break;

                case ObjectUsabilityFilter.Unusable:
                    m_usabilityPredicate = CannotUse;
                    break;

                default:
                    throw new NotImplementedException();
            }

            // Update content
            UpdateContent();
        }

        /// <summary>
        /// When the combo for activity filter changes, we update the settings and the control content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbActivity_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Update settings
            Settings.UI.BlueprintBrowser.ActivityFilter = (ObjectActivityFilter)cbActivityFilter.SelectedIndex;
            m_activityFilter = Settings.UI.BlueprintBrowser.ActivityFilter;

            switch (m_activityFilter)
            {               
                case ObjectActivityFilter.Manufacturing:
                    m_activity = BlueprintActivity.Manufacturing;
                    break;

                case ObjectActivityFilter.Copying:
                    m_activity = BlueprintActivity.Copying;
                    break;

                case ObjectActivityFilter.ResearchingMaterialProductivity:
                    m_activity = BlueprintActivity.ResearchingMaterialProductivity;
                    break;

                case ObjectActivityFilter.ResearchingTimeProductivity:
                    m_activity = BlueprintActivity.ResearchingTimeProductivity;
                    break;

                case ObjectActivityFilter.Invention:
                    m_activity = BlueprintActivity.Invention;
                    break;

                default:
                    m_activity = BlueprintActivity.None;
                    break;
            }

            cbUsabilityFilter_SelectedIndexChanged(sender, e);
        }

        /// <summary>
        /// When one of the metagroups combo is checked/unchecked, we update the settings and the control content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbMetagroup_CheckedChanged(object sender, EventArgs e)
        {
            // Retrieve the metagroup
            ItemMetaGroup metagroup = ItemMetaGroup.Empty;
            if (cbTech1.Checked)
                metagroup |= ItemMetaGroup.T1;
            if (cbTech2.Checked)
                metagroup |= ItemMetaGroup.T2;
            if (cbTech3.Checked)
                metagroup |= ItemMetaGroup.T3;
            if (cbFaction.Checked)
                metagroup |= ItemMetaGroup.Faction;
            if (cbStoryline.Checked)
                metagroup |= ItemMetaGroup.Storyline;
            if (cbOfficer.Checked)
                metagroup |= ItemMetaGroup.Officer;

            // Update the settings
            Settings.UI.BlueprintBrowser.MetagroupFilter |= metagroup;

            // Update the predicate
            m_metaGroupPredicate = (x) => (x.MetaGroup & metagroup) != ItemMetaGroup.Empty;

            // Update content
            UpdateContent();
        }

        /// <summary>
        /// When the search text changed, we store the next settings and update the list view and the list/tree visibilities.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected override void tbSearchText_TextChanged(object sender, EventArgs e)
        {
            Settings.UI.BlueprintBrowser.TextSearch = tbSearchText.Text;
            base.tbSearchText_TextChanged(sender, e);
        }

        #endregion


        #region Content creation

        /// <summary>
        /// Refresh the controls
        /// </summary>
        private void UpdateContent()
        {
            BuildTreeView();
            BuildListView();
        }

        /// <summary>
        /// Rebuild the tree view
        /// </summary>
        private void BuildTreeView()
        {
            // Reset selected object
            SelectedObject = null;

            int numberOfItems = 0;
            tvItems.BeginUpdate();
            try
            {
                tvItems.Nodes.Clear();

                // Create the nodes
                foreach (var group in StaticBlueprints.BlueprintMarketGroups)
                {
                    TreeNode node = new TreeNode()
                    {
                        Text = group.Name
                    };

                    int result = BuildSubtree(group, node.Nodes);

                    if (result != 0)
                    {
                        numberOfItems += result;
                        tvItems.Nodes.Add(node);
                    }
                }
            }
            finally
            {
                tvItems.EndUpdate();
                m_allExpanded = false;

                // If the filtered set is small enough to fit all nodes on screen, call expandAll()
                if (numberOfItems < (tvItems.DisplayRectangle.Height / tvItems.ItemHeight))
                {
                    tvItems.ExpandAll();
                    m_allExpanded = true;
                }
            }
        }

        /// <summary>
        /// Create the tree nodes for the given category and add them to the given nodes collection
        /// </summary>
        /// <param name="cat"></param>
        /// <param name="nodeCollection"></param>
        /// <returns></returns>
        private int BuildSubtree(BlueprintMarketGroup group, TreeNodeCollection nodeCollection)
        {
            // Total blueprints count in this category and its subcategories
            int result = 0; 

            // Add all subcategories
            foreach (BlueprintMarketGroup childGroup in group.SubGroups)
            {
                TreeNode node = new TreeNode()
                {
                    Text = childGroup.Name
                };

                // Add this subcategory's blueprints count
                result += BuildSubtree(childGroup, node.Nodes);

                // Only add if this subcategory has children
                if (node.GetNodeCount(true) > 0)
                    nodeCollection.Add(node);
            }

            // Add all blueprints
            foreach (Blueprint childItem in group.Blueprints.Where(x => m_usabilityPredicate(x) && m_metaGroupPredicate(x)))
            {
                TreeNode node = new TreeNode()
                {
                    Text = childItem.Name,
                    Tag = childItem
                };

                nodeCollection.Add(node);
                result++;
            }
            return result;
        }
      
        #endregion
    }
}
