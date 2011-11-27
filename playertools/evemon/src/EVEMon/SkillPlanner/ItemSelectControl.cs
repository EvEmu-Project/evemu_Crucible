using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Linq;
using System.Text;

using EVEMon.Common;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common.Data;

namespace EVEMon.SkillPlanner
{
    public partial class ItemSelectControl : EveObjectSelectControl 
    {
        private readonly List<MarketGroup> m_presetGroups = new List<MarketGroup>();
        private readonly List<ItemMetaGroup> m_metaGroups = new List<ItemMetaGroup>();

        private Func<Item, Boolean> m_slotPredicate = (x) => true;
        private Func<Item, Boolean> m_metaGroupPredicate = (x) => true;
        private Func<Item, Boolean> m_fittingPredicate = (x) => true;
        private bool m_isLoaded;

        #region Initialisation
        /// <summary>
        /// Constructor
        /// </summary>
        public ItemSelectControl()
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

            m_metaGroups.AddRange(EnumExtensions.GetBaseValues<ItemMetaGroup>());

            // Set the presets
            m_presetGroups.Add(StaticItems.MarketGroups.First(x => x.ID == DBConstants.AmmosAndChargesGroupID));
            m_presetGroups.Add(StaticItems.MarketGroups.First(x => x.ID == DBConstants.ImplantsAndBoostersGroupID));
            m_presetGroups.Add(StaticItems.MarketGroups.First(x => x.ID == DBConstants.StarbaseStructuresGroupID));
            m_presetGroups.Add(StaticItems.MarketGroups.First(x => x.ID == DBConstants.ShipModificationsGroupID));
            m_presetGroups.Add(StaticItems.MarketGroups.First(x => x.ID == DBConstants.ShipEquipmentGroupID));
            m_presetGroups.Add(StaticItems.MarketGroups.First(x => x.ID == DBConstants.DronesGroupID));

            // Initialize the metagroup combo
            ccbGroupFilter.Items.Clear();
            ccbGroupFilter.Items.AddRange(m_metaGroups.Cast<Object>().ToArray());
            ccbGroupFilter.ItemCheck += ccbGroupFilter_ItemCheck;
            ccbGroupFilter.ToolTip = toolTip;

            // Initialize the "skills" combo box
            cbUsabilityFilter.Items[0] = "All Items";
            cbUsabilityFilter.Items[1] = "Items I can use";
            cbUsabilityFilter.Items[2] = "Items I cannot use";
            
            // Read the settings
            if (Settings.UI.UseStoredSearchFilters)
            {
                // Usability combo
                cbUsabilityFilter.SelectedIndex = (int)Settings.UI.ItemBrowser.UsabilityFilter;

                // Metagroups combo
                for (int i = 0; i < m_metaGroups.Count; i++)
                {
                    ccbGroupFilter.SetItemChecked(i,
                        (Settings.UI.ItemBrowser.MetagroupFilter & m_metaGroups[i]) != ItemMetaGroup.Empty);
                }

                // Slots combo
                switch (Settings.UI.ItemBrowser.SlotFilter)
                {
                    case ItemSlot.All:
                        cbSlotFilter.SelectedIndex = 0;
                        break;
                    case ItemSlot.High:
                        cbSlotFilter.SelectedIndex = 1;
                        break;
                    case ItemSlot.Medium:
                        cbSlotFilter.SelectedIndex = 2;
                        break;
                    case ItemSlot.Low:
                        cbSlotFilter.SelectedIndex = 3;
                        break;
                    case ItemSlot.None:
                        cbSlotFilter.SelectedIndex = 4;
                        break;
                    default:
                        throw new NotImplementedException();
                }

                tbSearchText.Text = Settings.UI.ItemBrowser.TextSearch;
                lbSearchTextHint.Visible = String.IsNullOrEmpty(tbSearchText.Text);
            }
            else
            {
                cbUsabilityFilter.SelectedIndex = 0;
                cbSlotFilter.SelectedIndex = 0;
            }

            // Update the display
            m_isLoaded = true;
            UpdateContent();
        }
        #endregion


        #region Events handlers
        /// <summary>
        /// When the search text changed, we store the next settings
        /// and update the list view and the list/tree visibilities.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected override void tbSearchText_TextChanged(object sender, EventArgs e)
        {
            if (m_isLoaded)
                Settings.UI.ItemBrowser.TextSearch = tbSearchText.Text;
            base.tbSearchText_TextChanged(sender, e);
        }

        /// <summary>
        /// When the skill filter combo is changed, we update the settings, the predicate and the content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbUsabilityFilter_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Update the settings
            if (m_isLoaded)
                Settings.UI.ItemBrowser.UsabilityFilter = (ObjectUsabilityFilter)cbUsabilityFilter.SelectedIndex;

            // Update the predicate
            switch (Settings.UI.ItemBrowser.UsabilityFilter)
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

            // Update the control's content
            UpdateContent();
        }

        /// <summary>
        /// When the slot filter combo changed, we update the settings, the predicate and the content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbSlotFilter_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Update the settings
            if (m_isLoaded)
            {
                switch (cbSlotFilter.SelectedIndex)
                {
                    default:
                        Settings.UI.ItemBrowser.SlotFilter = ItemSlot.All;
                        break;

                    case 1:
                        Settings.UI.ItemBrowser.SlotFilter = ItemSlot.High;
                        break;

                    case 2:
                        Settings.UI.ItemBrowser.SlotFilter = ItemSlot.Medium;
                        break;

                    case 3:
                        Settings.UI.ItemBrowser.SlotFilter = ItemSlot.Low;
                        break;

                    case 4:
                        Settings.UI.ItemBrowser.SlotFilter = ItemSlot.None;
                        break;
                }
            }

            // Update the predicate
            ItemSlot slot = Settings.UI.ItemBrowser.SlotFilter;
            m_slotPredicate = (x) => (x.FittingSlot & slot) != ItemSlot.Empty;

            // Update the control's content
            UpdateContent();
        }
        
        /// <summary>
        /// When the meta group combo changed, we update the settings, the predicate and the content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void ccbGroupFilter_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            // Update the settings
            if (m_isLoaded)
            {
                Settings.UI.ItemBrowser.MetagroupFilter = ItemMetaGroup.Empty;
                for (int i = 0; i < m_metaGroups.Count; i++)
                {
                    if (ccbGroupFilter.GetItemChecked(i))
                        Settings.UI.ItemBrowser.MetagroupFilter |= m_metaGroups[i];
                }
            }

            // Update the predicate
            var filter = Settings.UI.ItemBrowser.MetagroupFilter;
            m_metaGroupPredicate = (x) => (x.MetaGroup & filter) != ItemMetaGroup.Empty;

            // Update the control's content
            UpdateContent();
        }

        /// <summary>
        /// When the CPU's numeric box changed, we update the predicate, the content (no settings)
        /// and the numeric box's availability.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbCPU_CheckedChanged(object sender, EventArgs e)
        {
            numCPU.Enabled = cbCPU.Checked;
            UpdateFittingPredicate();
            UpdateContent();
        }

        /// <summary>
        /// When the powergrid's numeric box changed, we update the predicate, the content (no settings)
        /// and the numeric box's availability.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbPowergrid_CheckedChanged(object sender, EventArgs e)
        {
            numPowergrid.Enabled = cbPowergrid.Checked;
            UpdateFittingPredicate();
            UpdateContent();
        }

        /// <summary>
        /// When the CPU's numeric box changed, we update the predicate and the content (no settings).
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void numCPU_ValueChanged(object sender, EventArgs e)
        {
            UpdateFittingPredicate();
            UpdateContent();
        }

        /// <summary>
        /// When the powergrid's numeric box changed, we update the predicate and the content (no settings).
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void numPowergrid_ValueChanged(object sender, EventArgs e)
        {
            UpdateFittingPredicate();
            UpdateContent();
        }

        /// <summary>
        /// When the "show all items" checkbox changed, we update the settings and trigger a content update.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void showAllGroupsCheckbox_CheckedChanged(object sender, System.EventArgs e)
        {
            Settings.UI.ItemBrowser.ShowAllGroups = showAllGroupsCheckbox.Checked;
            UpdateContent();
        }

        /// <summary>
        /// Updates the fitting predicate.
        /// </summary>
        private void UpdateFittingPredicate()
        {
            if (!numCPU.Enabled && !numPowergrid.Enabled)
            {
                m_fittingPredicate = (x) => true;
            }
            else
            {
                double? gridAvailable = null;
                if (numPowergrid.Enabled)
                    gridAvailable = (double)numPowergrid.Value;

                double? cpuAvailable = null;
                if (numCPU.Enabled)
                    cpuAvailable = (double)numCPU.Value;

                m_fittingPredicate = (item) => item.CanActivate(cpuAvailable, gridAvailable);
            }
        }
        #endregion


        #region Content creation
        /// <summary>
        /// Refresh the controls.
        /// </summary>
        private void UpdateContent()
        {
            if (!m_isLoaded)
                return;

            BuildTreeView();
            BuildListView();
        }

        /// <summary>
        /// Rebuild the tree view.
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
                foreach (MarketGroup group in StaticItems.MarketGroups)
                {
                    // Skip some groups
                    if (!showAllGroupsCheckbox.Checked && !m_presetGroups.Contains(group))
                        continue;

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
        /// Create the tree nodes for the given category and add them to the given nodes collection.
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
            foreach (Item childItem in group.Items.Where(x => m_slotPredicate(x)
                                                            && m_metaGroupPredicate(x)
                                                            && m_fittingPredicate(x)
                                                            && m_usabilityPredicate(x)))
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
