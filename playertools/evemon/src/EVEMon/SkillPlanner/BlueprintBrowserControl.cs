using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Data;

namespace EVEMon.SkillPlanner
{
    public partial class BlueprintBrowserControl : EveObjectBrowserControl
    {
        private string[] m_laboratories = new string[] { "Mobile Laboratory", "Advance Mobile Laboratory", "Any Other Laboratory" };

        private double m_timeMultiplier;
        private double m_materialMultiplier;
        private double m_waste;
        private bool m_hasInvention;
        private Character m_character;
        private Blueprint m_blueprint;
        private BlueprintActivity m_activity;

        #region Constructors

        /// <summary>
        /// Constructor.
        /// </summary>
        public BlueprintBrowserControl()
        {
            InitializeComponent();
            scObjectBrowser.RememberDistanceKey = "BlueprintBrowser_Left";
            Initialize(lvManufacturing, blueprintSelectControl, false);
        }

        #endregion


        #region Private Properties

        /// <summary>
        /// Gets the activity of this blueprint.
        /// </summary>
        private BlueprintActivity Activity
        {
            get { return m_activity; }
        }

        #endregion


        #region Inherited Events

        /// <summary>
        /// Occurs when the control is loaded.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            // Return on design mode
            if (DesignMode || this.IsDesignModeHosted())
                return;

            lblHelp.Text = "Use the tree on the left to select a blueprint to view.";
            gbDescription.Text = "Attributes";
            pnlAttributes.AutoScroll = true;

            // Call the base method
            base.OnLoad(e);
        }

        /// <summary>
        /// Updates the controls when the selection is changed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected override void OnSelectionChanged(object sender, EventArgs e)
        {
            // Call the base method
            base.OnSelectionChanged(sender, e);
            
            if (SelectedObject == null)
                return;

            m_blueprint = SelectedObject as Blueprint;

            // Update Tabs
            UpdateTabs();

            // Update Required Skills
            m_activity = GetActivity();
            requiredSkillsControl.Object = SelectedObject;
            requiredSkillsControl.Activity = m_activity;

            // Update Facility Modifier
            UpdateFacilityModifier();
        }

        /// <summary>
        /// Updates the plan when the selection is changed.
        /// </summary>
        protected override void OnPlanChanged()
        {
            base.OnPlanChanged();
            requiredSkillsControl.Plan = Plan;

            // We recalculate the right panels minimum size
            int reqSkillControlMinWidth = requiredSkillsControl.MinimumSize.Width;
            int reqSkillPanelMinWidth = scDetails.Panel2MinSize;
            scDetails.Panel2MinSize = (reqSkillPanelMinWidth > reqSkillControlMinWidth ?
                                         reqSkillPanelMinWidth : reqSkillControlMinWidth);
        }


        /// <summary>
        /// Updates the implant modifier list when the settings changed.
        /// </summary>
        protected override void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            base.EveClient_SettingsChanged(sender, e);
            UpdateImplantSetModifier();
        }

        #endregion


        #region Helper Methods

        /// <summary>
        /// Create the necessary tabs.
        /// </summary>
        private void UpdateTabs()
        {
            // Determine the blueprints' activities  
            bool hasCopying = m_blueprint.Prerequisites.Any(x => x.Activity == BlueprintActivity.Copying)
                || m_blueprint.MaterialRequirements.Any(x => x.Activity == BlueprintActivity.Copying);

            bool hasResearchingMaterialProductivity = m_blueprint.Prerequisites.Any(x => x.Activity == BlueprintActivity.ResearchingMaterialProductivity)
                || m_blueprint.MaterialRequirements.Any(x => x.Activity == BlueprintActivity.ResearchingMaterialProductivity);

            bool hasResearchingTimeProductivity = m_blueprint.Prerequisites.Any(x => x.Activity == BlueprintActivity.ResearchingTimeProductivity)
                || m_blueprint.MaterialRequirements.Any(x => x.Activity == BlueprintActivity.ResearchingTimeProductivity);

            m_hasInvention = m_blueprint.Prerequisites.Any(x => x.Activity == BlueprintActivity.Invention)
                || m_blueprint.MaterialRequirements.Any(x => x.Activity == BlueprintActivity.Invention);

            try
            {
                // Hide the tab control if the selected tab is not the first
                // (avoids tab creation be visible to user)
                if (tabControl.SelectedIndex != 0)
                    tabControl.Hide();

                // Store the selected tab index for later use
                int storedTabIndex = tabControl.SelectedIndex;

                tabControl.TabPages.Clear();

                // Add the appropriate tabs
                tabControl.TabPages.Add(tpManufacturing);

                if (hasCopying)
                    tabControl.TabPages.Add(tpCopying);

                if (hasResearchingMaterialProductivity)
                    tabControl.TabPages.Add(tpResearchME);

                if (hasResearchingTimeProductivity)
                    tabControl.TabPages.Add(tpResearchPE);

                if (!hasCopying && !hasResearchingMaterialProductivity && !hasResearchingTimeProductivity)
                    tabControl.TabPages.Add(tpResearching);

                if (m_hasInvention)
                    tabControl.TabPages.Add(tpInvention);

                // Restore the index of the previous selected tab,
                // if the index doesn't exist it smartly selects
                // the first one by its own
                tabControl.SelectedIndex = storedTabIndex;
            }
            finally
            {
                tabControl.Show();

                // Return focus to selector
                blueprintSelectControl.tvItems.Focus();
            }
        }

        /// <summary>
        /// Update the attributes info.
        /// </summary>
        private void UpdateAttributes()
        {           
            // Produce item
            lblItem.ForeColor = Color.Blue;
            lblItem.Text = (m_blueprint.ProducesItem == null ? // This should not happen but be prepared if something changes in CCP DB
                m_blueprint.Name.Replace(" Blueprint", String.Empty) :
                m_blueprint.ProducesItem.Name); 
            lblItem.Tag = m_blueprint.ProducesItem;
            
            // Invents blueprint
            InventBlueprintListBox.Items.Clear();
            foreach (var item in m_blueprint.InventsBlueprint)
            {
                InventBlueprintListBox.Items.Add(item);
            }
            
            // Runs per copy
            lblRunsPerCopy.Text = m_blueprint.RunsPerCopy.ToString();

            // Wastage factor
            m_waste = ((double)m_blueprint.WasteFactor / 100) * (double)(nudME.Value >= 0 ? 1 / (nudME.Value + 1) : (1 - nudME.Value));
            lblWaste.Text = m_waste.ToString("0.0#%");
            
            // Multipliers
            m_timeMultiplier = GetFacilityMultiplier(out m_materialMultiplier);

            // Manufacturing base time
            double factor = 0.04d;
            double peModifier = (double)(nudPE.Value >= 0 ? (nudPE.Value / (1 + nudPE.Value)) : (nudPE.Value - 1));
            double pModifier = 1 - (m_blueprint.ProductivityModifier / m_blueprint.ProductionTime) * peModifier;
            lblProductionBaseTime.Text = BaseActivityTime(m_blueprint.ProductionTime * pModifier);

            // Manufacturing character time
            double activityTime = m_blueprint.ProductionTime * pModifier * m_timeMultiplier * GetImplantMultiplier("F");
            lblProductionCharTime.Text = CharacterActivityTime(activityTime, DBConstants.IndustrySkillID, factor, false);

            // Researching material efficiency base time
            lblResearchMEBaseTime.Text = BaseActivityTime(m_blueprint.ResearchMaterialTime);

            // Researching material efficiency character time
            factor = 0.05d;
            activityTime = m_blueprint.ResearchMaterialTime
                * GetFacilityResearchMultiplier(BlueprintActivity.ResearchingMaterialProductivity) * GetImplantMultiplier("J");
            lblResearchMECharTime.Text = CharacterActivityTime(activityTime, DBConstants.MetallurgySkillID, factor, false);

            // Researching copy base time
            lblResearchCopyBaseTime.Text = BaseActivityTime(m_blueprint.ResearchCopyTime);

            // Researching copy character time
            activityTime = (m_blueprint.ResearchCopyTime / m_blueprint.RunsPerCopy)
                * GetFacilityResearchMultiplier(BlueprintActivity.Copying) * GetImplantMultiplier("K");
            lblResearchCopyCharTime.Text = CharacterActivityTime(activityTime, DBConstants.ScienceSkillID, factor, true);

            // Researching productivity efficiency base time
            lblResearchPEBaseTime.Text = BaseActivityTime(m_blueprint.ResearchProductivityTime);

            // Researching productivity efficiency character time
            activityTime = m_blueprint.ResearchProductivityTime
                * GetFacilityResearchMultiplier(BlueprintActivity.ResearchingTimeProductivity) * GetImplantMultiplier("I");
            lblResearchPECharTime.Text = CharacterActivityTime(activityTime, DBConstants.ResearchSkillID, factor, false);

            gbResearching.Visible = !m_blueprint.MarketGroup.BelongsIn(new int[] { DBConstants.BlueprintRootNonMarketGroupID });
            gbInvention.Text = (gbResearching.Visible ? "INVENTION" : "RESEARCHING");
            lblInventionTime.Text = (gbResearching.Visible ? "Invention Time:" : "Research Tech Time:");
            gbInvention.Location = (gbResearching.Visible ? new Point(3, 385) : new Point(3, 225));
            gbInvention.Visible = (!gbResearching.Visible || m_hasInvention);

            if (!gbInvention.Visible)
                return;

            // Invention time
            lblInventionBaseTime.Text = BaseActivityTime(m_blueprint.ResearchTechTime * GetFacilityResearchMultiplier(BlueprintActivity.Invention));
        }

        /// <summary>
        /// Update the required materials list.
        /// </summary>
        private void UpdateRequiredMaterialsList()
        {
            int productionEfficiencyLevel = (m_character.Skills.FirstOrDefault(x => x.ID == DBConstants.ProductionEfficiencySkillID)).LastConfirmedLvl;

            m_propertiesList.BeginUpdate();
            try
            {
                // Clear everything
                m_propertiesList.Items.Clear();
                m_propertiesList.Groups.Clear();
                m_propertiesList.Columns.Clear();

                // Create the columns
                m_propertiesList.Columns.Add("Item");
                m_propertiesList.Columns.Add("Quantity (You)");
                m_propertiesList.Columns.Add("Quantity (Perfect)");
                m_propertiesList.Columns.Add("Damage Per Run");

                int perfectME = 0;
                int perfectMELevel = 0;
                bool hasPerfect = false;
                bool hasDamagePerRun = false;
                var items = new List<ListViewItem>();
                foreach (var marketGroup in StaticItems.AllGroups)
                {
                    // Create the groups
                    var group = new ListViewGroup(marketGroup.FullCategoryName);
                    bool hasItem = false;
                    foreach (var material in m_blueprint.MaterialRequirements.Where(x => x.Activity == Activity && marketGroup.Items.Any(y => y.ID == x.ID)))
                    {
                        hasItem = true;

                        // Create the item
                        ListViewItem item = new ListViewItem(group);
                        item.Tag = StaticItems.GetItemByID(material.ID);
                        item.Text = material.Name;
                        
                        // Add the item to the list
                        items.Add(item);

                        // Set if the item is a raw material and therefore waste affected
                        bool isRawMaterial = material.WasteAffected;

                        // Calculate the base material quantity
                        int baseMaterialQuantity = (int)Math.Round(material.Quantity * m_materialMultiplier * GetImplantMultiplier("G"), 0, MidpointRounding.AwayFromZero);
                        
                        // Calculate the perfect material efficiency level if it's a raw material
                        if (isRawMaterial)
                            perfectMELevel = (int)Math.Round((0.02 * m_blueprint.WasteFactor * baseMaterialQuantity), 0, MidpointRounding.AwayFromZero);

                        // Store the highest perfect material efficiency level
                        if (perfectMELevel > perfectME)
                            perfectME = perfectMELevel;
                        
                        // Calculate the needed quantity by the character skills
                        int youQuantity = (Activity == BlueprintActivity.Manufacturing && isRawMaterial ?
                            (int)Math.Round(baseMaterialQuantity * (1.25 - (0.05 * productionEfficiencyLevel)) + (baseMaterialQuantity * m_waste), 0, MidpointRounding.AwayFromZero) :
                            baseMaterialQuantity);

                        // Calculate the perfect quantity
                        int perfectQuantity = (Activity == BlueprintActivity.Manufacturing && isRawMaterial ?
                            (int)Math.Round(baseMaterialQuantity * (1 + m_waste), 0, MidpointRounding.AwayFromZero) : baseMaterialQuantity);

                        // Add the quantity for every item
                        var subItemYou = new ListViewItem.ListViewSubItem(item, youQuantity.ToString());
                        item.SubItems.Add(subItemYou);

                        // Has perfect values ?
                        hasPerfect |= (youQuantity != perfectQuantity);

                        // Add the perfect quantity for every item
                        var subItemPerfect = new ListViewItem.ListViewSubItem(item, perfectQuantity.ToString());
                        item.SubItems.Add(subItemPerfect);

                        // Has damage per run ?
                        hasDamagePerRun |= (material.DamagePerJob > 0 && material.DamagePerJob < 1);

                        // Add the damage per run for every item (empty string if it's 1)
                        string damagePerRun = (material.DamagePerJob > 0 && material.DamagePerJob < 1 ?
                            String.Format("{0:P1}", material.DamagePerJob) : String.Empty); 
                        var subItemDamagePerRun = new ListViewItem.ListViewSubItem(item, damagePerRun);
                        item.SubItems.Add(subItemDamagePerRun);
                    }

                    // Add the group that has an item
                    if (hasItem)
                        m_propertiesList.Groups.Add(group);
                }

                // Remove the "Perfect" column if all values are empty
                if (!hasPerfect)
                    RemoveColumn(items, 2);

                // Remove the "Damage Per Run" column if all values are empty
                if (!hasDamagePerRun)
                    RemoveColumn(items, m_propertiesList.Columns.Count - 1);

                // Add the items
                m_propertiesList.Items.AddRange(items.OrderBy(x => x.Text).ToArray());

                // Display the Perfect ME
                if (tabControl.SelectedTab == tpManufacturing)
                    lblPerfectMEValue.Text = perfectME.ToString("#,##0");

                // Show/Hide the "no item required" label and autoresize the columns 
                m_propertiesList.Visible = m_propertiesList.Items.Count > 0;
                if (m_propertiesList.Visible)
                    m_propertiesList.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);
            }
            finally
            {
                m_propertiesList.EndUpdate();
            }
        }

        /// <summary>
        /// Update the facility modifier list. 
        /// </summary>
        private void UpdateFacilityModifier()
        {
            cbFacility.Items.Clear();

            cbFacility.Items.Add("NPC Station");

            Item producedItem = m_blueprint.ProducesItem;

            if (producedItem != null)// This should not happen but be prepared if something changes in CCP DB
            {
                switch (Activity)
                {
                    case BlueprintActivity.Manufacturing:
                        if (producedItem.MarketGroup.BelongsIn(new int[] { DBConstants.DronesGroupID })
                            && !producedItem.MarketGroup.BelongsIn(DBConstants.SmallToXLargeShipsGroupIDs))
                            cbFacility.Items.Add("Drone Assembly Array");

                        if (producedItem.MarketGroup.BelongsIn(new int[] { DBConstants.AmmosAndChargesGroupID }))
                            cbFacility.Items.Add("Ammunition Assembly Array");

                        if (producedItem.MarketGroup.BelongsIn(new int[] { DBConstants.ShipEquipmentGroupID }))
                        {
                            cbFacility.Items.Add("Equipment Assembly Array");
                            cbFacility.Items.Add("Rapid Equipment Assembly Array");
                        }

                        if (producedItem.MarketGroup.BelongsIn(new int[] { DBConstants.ComponentsGroupID }))
                            cbFacility.Items.Add("Component Assembly Array");

                        if (producedItem.MarketGroup.BelongsIn(DBConstants.StategicComponentsGroupIDs))
                            cbFacility.Items.Add("Subsystem Assembly Array");

                        if (producedItem.MarketGroup.BelongsIn(DBConstants.SmallToXLargeShipsGroupIDs))
                            cbFacility.Items.Add("Ship Assembly Array (Ship Size)");

                        if (producedItem.MarketGroup.BelongsIn(DBConstants.CapitalShipsGroupIDs))
                            cbFacility.Items.Add("Capital Assembly Array");

                        if (producedItem.MarketGroup.BelongsIn(DBConstants.AdvancedSmallToLargeShipsGroupIDs))
                            cbFacility.Items.Add("Advanced Ship Assembly Array (Ship Size)");

                        break;
                    default:
                        cbFacility.Items.AddRange(m_laboratories);
                        break;
                }
            }

            // Update the selected index
            if (Activity == BlueprintActivity.Manufacturing)
            {
                cbFacility.SelectedIndex = (Settings.UI.BlueprintBrowser.ProductionFacilityIndex < cbFacility.Items.Count ?
                    Settings.UI.BlueprintBrowser.ProductionFacilityIndex : 0);
            }
            else
            {
                cbFacility.SelectedIndex = Settings.UI.BlueprintBrowser.ResearchFacilityIndex;
            }
        }

        /// <summary>
        /// Update the implant set modifier list. 
        /// </summary>
        private void UpdateImplantSetModifier()
        {
            m_character = (Character)m_plan.Character;
            cbImplantSet.Items.Clear();
            foreach (var set in m_character.ImplantSets)
            {
                cbImplantSet.Items.Add(set);
            }

            // Update the selected index
            cbImplantSet.SelectedIndex = (Settings.UI.BlueprintBrowser.ImplantSetIndex < cbImplantSet.Items.Count ? 
                Settings.UI.BlueprintBrowser.ImplantSetIndex : 0);
        }

        /// <summary>
        /// Remove values and column from the listview.
        /// </summary>
        /// <param name="items"></param>
        /// <param name="columnIndex"></param>
        private void RemoveColumn(List<ListViewItem> items, int columnIndex)
        {
            // Remove the values of the column
            for (int i = 0; i < items.Count; i++)
            {
                items[i].SubItems.RemoveAt(columnIndex);
            }

            // Remove the column control
            m_propertiesList.Columns.RemoveAt(columnIndex);
        }

        /// <summary>
        /// Calculate the base activity time.
        /// </summary>
        /// <param name="activityTime"></param>
        /// <returns></returns>
        private string BaseActivityTime(double activityTime)
        {
            TimeSpan time = TimeSpan.FromSeconds(activityTime);
            bool includeSeconds = (time.Hours < 1);
            return TimeSpanToText(time, includeSeconds);
        }

        /// <summary>
        /// Calculate the character's activity time.
        /// </summary>
        /// <param name="activityTime"></param>
        /// <param name="skillID"></param>
        /// <returns></returns>
        private string CharacterActivityTime(double activityTime, int skillID, double factor, bool copyActivity)
        {
            int skillLevel = (m_character.Skills.FirstOrDefault(x => x.ID == skillID)).LastConfirmedLvl;
            double activityTimeModifier = (1 - (factor * skillLevel));
            TimeSpan time = TimeSpan.FromSeconds(activityTime * activityTimeModifier );
            bool includeSeconds = (time.Hours == 0 && time.Minutes < 10);
            return String.Format("{0} (You{1})", TimeSpanToText(time, includeSeconds), (copyActivity ? " Per Single Copy" : String.Empty));
        }

        /// <summary>
        /// Transpose the timespan to text.
        /// </summary>
        /// <param name="time"></param>
        /// <param name="includeSeconds"></param>
        /// <returns></returns>
        private string TimeSpanToText(TimeSpan time, bool includeSeconds)
        {
            return time.ToDescriptiveText(
                DescriptiveTextOptions.FirstLetterUppercase
                | DescriptiveTextOptions.SpaceText
                | DescriptiveTextOptions.FullText
                | DescriptiveTextOptions.IncludeCommas,
                includeSeconds);
        }

        /// <summary>
        /// Gets the activity from the selected tab.
        /// </summary>
        /// <returns></returns>
        private BlueprintActivity GetActivity()
        {
            if (tabControl.SelectedTab == null)
                return BlueprintActivity.None;

            switch (tabControl.SelectedTab.Text)
            {
                case "Manufacturing":
                    m_propertiesList = lvManufacturing;
                    return BlueprintActivity.Manufacturing;
                case "Copying":
                    m_propertiesList = lvCopying;
                    return BlueprintActivity.Copying;
                case "Researching Material Efficiency":
                    m_propertiesList = lvResearchME;
                    return BlueprintActivity.ResearchingMaterialProductivity;
                case "Researching Time Productivity":
                    m_propertiesList = lvResearchPE;
                    return BlueprintActivity.ResearchingTimeProductivity;
                case "Invention":
                    m_propertiesList = lvInvention;
                    return BlueprintActivity.Invention;
                default:
                    m_propertiesList = lvManufacturing;
                    return BlueprintActivity.None;
            }
        }

        /// <summary>
        /// Show the item in its appropriate browser.
        /// </summary>
        /// <param name="item"></param>
        private void ShowInBrowser(Item item)
        {
            PlanWindow pw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
            if (item is Ship)
            {
                pw.ShowShipInBrowser(item);
            }
            else
            {
                pw.ShowItemInBrowser(item);
            }
        }

        /// <summary>
        /// Gets the manufacturing time and material multiplier of a facility.
        /// </summary>
        /// <param name="materialMultiplier"></param>
        /// <returns></returns>
        private double GetFacilityMultiplier(out double materialMultiplier)
        {
            var text = cbFacility.Text;
            m_timeMultiplier = 0.75d;
            materialMultiplier = 1.0d;

            if (Activity != BlueprintActivity.Manufacturing)
                return 1.0d;

            if (text.StartsWith("Rapid"))
            {
                m_timeMultiplier = 0.65d;
                materialMultiplier = 1.2d;
            }

            if (text.StartsWith("Subsystem") || text.StartsWith("Capital") || text.StartsWith("NPC"))
                m_timeMultiplier = 1.0d;

            if (text.StartsWith("Advanced"))
                materialMultiplier = 1.1d;
            
            return m_timeMultiplier;
        }

        /// <summary>
        /// Gets the research time multiplier of a facility.
        /// </summary>
        /// <param name="activity"></param>
        /// <returns></returns>
        private double GetFacilityResearchMultiplier(BlueprintActivity activity)
        {
            var text = cbFacility.Text;

            if (text.StartsWith("Mobile"))
            {
                switch (activity)
                {
                    case BlueprintActivity.Invention:
                        return 0.5d;
                    default:
                        return 0.75d;
                }
            }

            if (text.StartsWith("Advance Mobile"))
            {
                switch (activity)
                {
                    case BlueprintActivity.ResearchingMaterialProductivity:
                        return 0.75d;
                    case BlueprintActivity.Copying:
                        return 0.65d;
                    case BlueprintActivity.Invention:
                        return 0.5d;
                }
            }

            return 1.0d;
        }
        
        /// <summary>
        /// Gets the multiplier of an implant.
        /// </summary>
        /// <param name="implantType"></param>
        /// <returns></returns>
        private double GetImplantMultiplier(string implantType)
        {
            ImplantSet implantSet = (ImplantSet)cbImplantSet.Tag;
            string implantSubname = String.Format("Zainou 'Beancounter' {0}", implantType);

            Implant implant = implantSet.FirstOrDefault(x => x.Name.Contains(implantSubname));

            if (implant == null)
                return 1.0d;

            double bonus = implant.Properties.FirstOrDefault(x=> Array.IndexOf(DBConstants.IndustryModifyingPropertyIDs, x.Property.ID) != -1).IValue;
            double multiplier = 1.0d + (bonus / 100);

            return multiplier;
        }

        #endregion


        #region Event Handlers

        /// <summary>
        /// Occurs when a tab is selected.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        /// <remarks>Updates the required skills control according to the seleted tab</remarks>
        private void tabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (tabControl.SelectedIndex == -1)
                return;

            m_activity = GetActivity();
            requiredSkillsControl.Activity = m_activity;
            UpdateFacilityModifier();
        }

        /// <summary>
        /// Occurs when the value of the control changes.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nudME_ValueChanged(object sender, EventArgs e)
        {
            UpdateAttributes();
            UpdateRequiredMaterialsList();
        }

        /// <summary>
        /// Occurs when the value of the control changes.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nudPE_ValueChanged(object sender, EventArgs e)
        {
            UpdateAttributes();
            UpdateRequiredMaterialsList();
        }

        /// <summary>
        /// Occurs on facility selection change.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbFacility_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (Activity == BlueprintActivity.Manufacturing)
            {
                Settings.UI.BlueprintBrowser.ProductionFacilityIndex = cbFacility.SelectedIndex;
            }
            else
            {
                Settings.UI.BlueprintBrowser.ResearchFacilityIndex = cbFacility.SelectedIndex;
            }

            UpdateAttributes();
            UpdateRequiredMaterialsList();
        }

        /// <summary>
        /// Occurs on implant set selection change.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbImplantSet_SelectedIndexChanged(object sender, EventArgs e)
        {
            Settings.UI.BlueprintBrowser.ImplantSetIndex = cbImplantSet.SelectedIndex;
            cbImplantSet.Tag = cbImplantSet.SelectedItem;

            if (m_blueprint == null)
                return;

            UpdateAttributes();
            UpdateRequiredMaterialsList();
        }

        /// <summary>
        /// Occurs when clicking on the control.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lblItem_Click(object sender, EventArgs e)
        {
            Item item = (Item)lblItem.Tag;

            if (item == null)
                return;

            ShowInBrowser(item);

            lblItem.ForeColor = Color.Purple;
        }

        /// <summary>
        /// Occurs when clicking on a list box item.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbInventBlueprint_SelectedIndexChanged(object sender, EventArgs e)
        {
            Blueprint blueprint = (Blueprint)InventBlueprintListBox.SelectedItem;

            if (blueprint == null)
                return;

            SelectedObject = blueprint;
            InventBlueprintListBox.ClearSelected();
        }

        /// <summary>
        /// Occurs when double clicking on a list view item.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_propertiesList_DoubleClick(object sender, EventArgs e)
        {
            ListViewItem listItem = ((ListView)sender).FocusedItem;
            Item item = (Item)listItem.Tag;
            
            if (item == null)
                return;

            ShowInBrowser(item);
        }
 
        #endregion

   }
}
