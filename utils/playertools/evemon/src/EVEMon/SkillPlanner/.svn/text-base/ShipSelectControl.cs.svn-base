using System;
using System.Linq;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Data;

namespace EVEMon.SkillPlanner
{
    public sealed partial class ShipSelectControl : EveObjectSelectControl
    {
        private Func<Item, Boolean> m_racePredicate = x => true;
        private bool m_isLoaded;

        #region Initialisation

        /// <summary>
        /// Constructor
        /// </summary>
        public ShipSelectControl()
        {
            InitializeComponent();
        }

        /// <summary>
        /// On load, we read the settings and fill the tree.
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

            // Build the ships tree
            BuildTreeView();

            // Initialize the "skills" combo box
            cbUsabilityFilter.Items[0] = "All Ships";
            cbUsabilityFilter.Items[1] = "Ships I can fly";
            cbUsabilityFilter.Items[2] = "Ships I cannot fly";

            // Read the settings
            if (Settings.UI.UseStoredSearchFilters)
            {
                cbUsabilityFilter.SelectedIndex = (int)Settings.UI.ShipBrowser.UsabilityFilter;
                tbSearchText.Text = Settings.UI.ShipBrowser.TextSearch;
                lbSearchTextHint.Visible = String.IsNullOrEmpty(tbSearchText.Text);

                cbAmarr.Checked = (Settings.UI.ShipBrowser.RacesFilter & Race.Amarr) != Race.None;
                cbCaldari.Checked = (Settings.UI.ShipBrowser.RacesFilter & Race.Caldari) != Race.None;
                cbGallente.Checked = (Settings.UI.ShipBrowser.RacesFilter & Race.Gallente) != Race.None;
                cbMinmatar.Checked = (Settings.UI.ShipBrowser.RacesFilter & Race.Minmatar) != Race.None;
                cbFaction.Checked = (Settings.UI.ShipBrowser.RacesFilter & Race.Faction) != Race.None;
                cbORE.Checked = (Settings.UI.ShipBrowser.RacesFilter & Race.Ore) != Race.None;
            }
            else
            {
                cbUsabilityFilter.SelectedIndex = 0;
            }
        }

        #endregion


        #region Callbacks

        /// <summary>
        /// When the combo for filter changes, we update the settings and the control content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbUsabilityFilter_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Update settings
            Settings.UI.ShipBrowser.UsabilityFilter = (ObjectUsabilityFilter)cbUsabilityFilter.SelectedIndex;

            // Update the filter delegate
            switch (Settings.UI.ShipBrowser.UsabilityFilter)
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
        /// When one of the races combo is checked/unchecked, we update the settings and the control content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbRace_SelectedChanged(object sender, EventArgs e)
        {
            // Retrieve the race
            Race race = Race.None;
            if (cbAmarr.Checked)
                race |= Race.Amarr;
            if (cbCaldari.Checked)
                race |= Race.Caldari;
            if (cbGallente.Checked)
                race |= Race.Gallente;
            if (cbMinmatar.Checked)
                race |= Race.Minmatar;
            if (cbFaction.Checked)
                race |= Race.Faction;
            if (cbORE.Checked)
                race |= Race.Ore;

            // Update the settings
            Settings.UI.ShipBrowser.RacesFilter |= race;

            // Update the predicate
            m_racePredicate = x => (x.Race & race) != Race.None;

            // Update content
            m_isLoaded = true;
            UpdateContent();
        }

        /// <summary>
        /// When the search text changed, we store the next settings and update the list view and the list/tree visibilities.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected override void tbSearchText_TextChanged(object sender, EventArgs e)
        {
            Settings.UI.ShipBrowser.TextSearch = tbSearchText.Text;
            base.tbSearchText_TextChanged(sender, e);
        }

        #endregion


        #region Content creation

        /// <summary>
        /// Refresh the controls
        /// </summary>
        private void UpdateContent()
        {
            if (!m_isLoaded)
                return;

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
                foreach (MarketGroup group in StaticItems.Ships.SubGroups)
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
        /// Create the tree nodes for the given group and add them to the given nodes collection
        /// </summary>
        /// <param name="group"></param>
        /// <param name="nodeCollection"></param>
        /// <returns></returns>
        private int BuildSubtree(MarketGroup group, TreeNodeCollection nodeCollection)
        {
            // Total items count in this category and its subcategories
            int result = 0;

            // Add all subcategories
            foreach (MarketGroup childGroup in group.SubGroups)
            {
                TreeNode node = new TreeNode()
                {
                    Text = childGroup.Name
                };

                // Add this subcategory's items count
                result += BuildSubtree(childGroup, node.Nodes);

                // Only add if this subcategory has children
                if (node.GetNodeCount(true) > 0)
                    nodeCollection.Add(node);
            }

            // Add all items
            foreach (Item childItem in group.Items.Where(m_usabilityPredicate).Where(m_racePredicate))
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
