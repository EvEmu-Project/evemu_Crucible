using System;
using System.IO;
using System.Linq;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Controls;
using SortOrder = EVEMon.Common.SortOrder;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// This window allows the user to manage all the plans : renaming, reordering, etc.
    /// </summary>
    public partial class PlanManagementWindow : EVEMonForm
    {
        private readonly PlanComparer m_columnSorter;
        private readonly Character m_character;

        /// <summary>
        /// Constructor for designer.
        /// </summary>
        public PlanManagementWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Regular constructor for use in code.
        /// </summary>
        /// <param name="character"></param>
        public PlanManagementWindow(Character character)
            : this()
        {
            m_character = character;
            m_columnSorter = new PlanComparer(PlanSort.Name);

            EveClient.CharacterPlanCollectionChanged += new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterPlanCollectionChanged);
        }

        /// <summary>
        /// Unsubscribe events on closing.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            EveClient.CharacterPlanCollectionChanged -= new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterPlanCollectionChanged);
            base.OnClosing(e);
        }

        /// <summary>
        /// On loading, populate the plan
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void PlanSelectWindow_Load(object sender, EventArgs e)
        {
            if (this.DesignMode || this.IsDesignModeHosted())
                return;

            UpdateContent(true);
            lbPlanList.ListViewItemSorter = null;
        }

        /// <summary>
        /// The button "open" is the same than "merge". When the button is in "open" state, we close the window and returns OK as a result.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnOpen_Click(object sender, EventArgs e)
        {
            // Are we performing a merge ?
            if (lbPlanList.SelectedItems.Count > 1)
            {
                MergePlans();
                return;
            }

            // Or are we just opening a plan ?
            var plan = (Plan)lbPlanList.SelectedItems[0].Tag;
            WindowsFactory<PlanWindow>.ShowByTag(plan);
            this.Close();
        }

        /// <summary>
        /// On close, nothing special.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        #region List management and creation
        /// <summary>
        /// Occurs when new plans are added or removed to the collection
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterPlanCollectionChanged(object sender, CharacterChangedEventArgs e)
        {
            UpdateContent(true);
        }

        /// <summary>
        /// Rebuild the plans list.
        /// </summary>
        /// <param name="restoreSelectionAndFocus"></param>
        private void UpdateContent(bool restoreSelectionAndFocus)
        {
            // Store selection and focus
            var selection = lbPlanList.Items.Cast<ListViewItem>().Where(x => x.Selected).Select(x => x.Tag as Plan).ToList();
            var focused = (lbPlanList.FocusedItem == null ? null : lbPlanList.FocusedItem.Tag as Plan);

            lbPlanList.BeginUpdate();
            try
            {
                // Recreate the list from scratch
                lbPlanList.Items.Clear();
                foreach (var plan in m_character.Plans)
                {
                    // Create the item and add it.
                    ListViewItem lvi = new ListViewItem(plan.Name);
                    lvi.Tag = plan;
                    lvi.SubItems.Add(plan.GetTotalTime(null, true).ToDescriptiveText(
                        DescriptiveTextOptions.FullText | DescriptiveTextOptions.IncludeCommas | DescriptiveTextOptions.SpaceText));
                    lvi.SubItems.Add(plan.UniqueSkillsCount.ToString());
                    lbPlanList.Items.Add(lvi);

                    // Restore selection and focus
                    if (restoreSelectionAndFocus)
                    {
                        lvi.Selected = selection.Contains(plan);
                        lvi.Focused = (focused == plan);
                    }
                }

                // Enable/disable the button
                btnOpen.Enabled = (lbPlanList.SelectedItems.Count > 0);
            }
            finally
            {
                lbPlanList.EndUpdate();
            }
        }

        /// <summary>
        /// Merge the selected plans.
        /// </summary>
        private void MergePlans()
        {
            // Build the merge plan
            var result = new Plan(m_character);
            using (result.SuspendingEvents())
            {
                // Merge the plans
                foreach (ListViewItem item in lbPlanList.SelectedItems)
                {
                    Plan plan = (Plan)item.Tag;
                    foreach (PlanEntry entry in plan)
                    {
                        // If not planned yet, we add the new entry
                        if (!result.IsPlanned(entry.Skill, entry.Level))
                            result.PlanTo(entry.Skill, entry.Level, entry.Priority, entry.Notes);

                        // Then we update the entry's groups
                        var newEntry = result.GetEntry(entry.Skill, entry.Level);
                        
                        // The entry may be null if the character already knows it.
                        if (newEntry != null)
                            newEntry.PlanGroups.Add(plan.Name);
                    }
                }
            }

            // Request a new name for this plan
            using (NewPlanWindow npw = new NewPlanWindow())
            {
                DialogResult dr = npw.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                // Change the plan's name and add it
                result.Name = npw.Result;
                m_character.Plans.Add(result);
            }
        }
        #endregion


        #region List's events
        /// <summary>
        /// When the user select another item, we update the control's status.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbPlanList_SelectedIndexChanged(object sender, EventArgs e)
        {
            // One one plan selected means we can move it
            tsbMoveUp.Enabled = (lbPlanList.SelectedItems.Count == 1);
            tsbMoveDown.Enabled = (lbPlanList.SelectedItems.Count == 1);

            // No items -> Disabled "open"
            // One item -> Enabled "open"
            // More items -> Enabled "merge"
            btnOpen.Enabled = (lbPlanList.SelectedItems.Count > 0);
            btnOpen.Text = (lbPlanList.SelectedItems.Count > 1 ? "Merge" : "Open");
        }

        /// <summary>
        /// When the user double-click an item, we open this plan.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbPlanList_DoubleClick(object sender, EventArgs e)
        {
            if (lbPlanList.SelectedItems.Count == 1)
                btnOpen_Click(this, new EventArgs());
        }

        /// <summary>
        /// On a column click, we update the sort.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbPlanList_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            // Click on the same column than the one already sorted ?
            if (e.Column == (int)m_columnSorter.Sort)
            {
                // Swap sort order
                m_columnSorter.Order = (m_columnSorter.Order == SortOrder.Ascending ? SortOrder.Descending : SortOrder.Ascending);
            }
            // Or a new column
            else
            {
                m_columnSorter.Sort = (PlanSort)e.Column;
                m_columnSorter.Order = SortOrder.Ascending;
            }

            // Update sort
            lbPlanList.ListViewItemSorter = new ListViewItemComparerByTag<Plan>(m_columnSorter);
            lbPlanList.Sort();

            // Rebuild the plans list from the listview items
            m_character.Plans.RebuildFrom(lbPlanList.Items.Cast<ListViewItem>().Select(x => x.Tag as Plan));
        }

        /// <summary>
        /// On reordering through drag'n drop, we fetch the new data to the character's plans collection.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbPlanList_ListViewItemsDragged(object sender, EventArgs e)
        {
            // Rebuild the plans list from the listview items
            m_character.Plans.RebuildFrom(lbPlanList.Items.Cast<ListViewItem>().Select(x => x.Tag as Plan));
        }
        #endregion


        #region Menus and buttons handlers
        /// <summary>
        /// File > New plan.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void miNewPlan_Click(object sender, EventArgs e)
        {
            // Request a new name for this plan
            using (NewPlanWindow npw = new NewPlanWindow())
            {
                DialogResult dr = npw.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                // Create the plan and add it
                var plan = new Plan(m_character);
                plan.Name = npw.Result;
                m_character.Plans.Add(plan);

                // Open a window for this plan
                WindowsFactory<PlanWindow>.ShowByTag(plan);
            }

            this.Close();
        }

        /// <summary>
        /// File > Load plan from file.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void miLoadPlanFromFile_Click(object sender, EventArgs e)
        {
            // Prompt the user to select a file
            DialogResult dr = ofdOpenDialog.ShowDialog();
            if (dr == DialogResult.Cancel)
                return;

            // Load from file and returns if an error occurred (user has already been warned)
            var serial = PlanExporter.ImportFromXML(ofdOpenDialog.FileName);
            if (serial == null)
                return;

            // Imports the plan
            Plan loadedPlan = new Plan(m_character);
            loadedPlan.Import(serial);

            // Prompt the user for the plan name
            using (NewPlanWindow npw = new NewPlanWindow())
            {
                npw.PlanName = Path.GetFileNameWithoutExtension(ofdOpenDialog.FileName);
                DialogResult xdr = npw.ShowDialog();
                if (xdr == DialogResult.Cancel)
                    return;

                loadedPlan.Name = npw.Result;
                m_character.Plans.Add(loadedPlan);
            }
        }

        /// <summary>
        /// File > Load plan from character.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void miLoadPlanFromCharacter_Click(object sender, EventArgs e)
        {
            // Prompt the user to choose the source character and plan.
            using (PlanImportationFromCharacterForm cps = new PlanImportationFromCharacterForm(m_character))
            {
                DialogResult dr = cps.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                // Retrieves the cloned plan
                var plan = cps.TargetPlan;

                // Adds and fixes the prerequisites order
                plan.Fix();

                // Prompt the user for the new plan's name
                using (NewPlanWindow f = new NewPlanWindow())
                {
                    f.PlanName = m_character.Name + "-" + plan.Name;
                    f.Text = "Save Plan As";

                    dr = f.ShowDialog();
                    if (dr == DialogResult.Cancel)
                        return;

                    plan.Name = f.Result;
                }

                // Add the plan to the character's list
                m_character.Plans.Add(plan);
            }
        }

        /// <summary>
        /// File > Delete
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void miDelete_Click(object sender, EventArgs e)
        {
            // Quit if none selected
            if (lbPlanList.SelectedItems.Count == 0)
                return;

            // Prepare the title and retrieve the plan's name for the incoming message box
            string planName;
            string title = "Delete Plan";
            if (lbPlanList.SelectedItems.Count > 1)
            {
                planName = "the selected plans";
                title += "s";
            }
            else
            {
                var plan = lbPlanList.SelectedItems[0].Tag as Plan;
                planName = "\"" + plan.Name + "\"";
            }

            // Prompt the user for confirmation with a message box
            DialogResult dr = MessageBox.Show("Are you sure you want to delete " + planName + "?", title, 
                MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2);

            if (dr != DialogResult.Yes)
                return;

            // Remove the items
            foreach (ListViewItem lvi in lbPlanList.SelectedItems)
            {
                m_character.Plans.Remove(lvi.Tag as Plan);
            }
        }

        /// <summary>
        /// Edit > Rename
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void miRename_Click(object sender, EventArgs e)
        {
            // Quit if none selected
            if (lbPlanList.SelectedItems.Count == 0)
                return;

            // Prompts the user for a new name
            var plan = lbPlanList.SelectedItems[0].Tag as Plan;
            using (NewPlanWindow f = new NewPlanWindow())
            {
                f.Text = "Rename Plan";
                f.PlanName = plan.Name;
                DialogResult dr = f.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                // Change the name
                plan.Name = f.Result;
                UpdateContent(true);
            }
        }

        /// <summary>
        /// Right toolbar > Move up.
        /// Move the plan and reinsert it at the proper position.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsbMoveUp_Click(object sender, EventArgs e)
        {
            int idx = lbPlanList.SelectedIndices[0];
            if (idx == 0)
                return;

            // Rebuild a plans array
            var plans = lbPlanList.Items.Cast<ListViewItem>().Select(x => x.Tag as Plan).ToArray();
            var temp = plans[idx - 1];
            plans[idx - 1] = plans[idx];
            plans[idx] = temp;

            lbPlanList.ListViewItemSorter = null;
            m_character.Plans.RebuildFrom(plans);
        }

        /// <summary>
        /// Right toolbar > Move down.
        /// Move the plan and reinsert it at the proper position.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsbMoveDown_Click(object sender, EventArgs e)
        {
            int idx = lbPlanList.SelectedIndices[0];
            if (idx == lbPlanList.Items.Count - 1)
                return;

            // Rebuild a plans array
            var plans = lbPlanList.Items.Cast<ListViewItem>().Select(x => x.Tag as Plan).ToArray();
            var temp = plans[idx + 1];
            plans[idx + 1] = plans[idx];
            plans[idx] = temp;

            lbPlanList.ListViewItemSorter = null;
            m_character.Plans.RebuildFrom(plans);
        }

        /// <summary>
        /// File > Export plan
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void miExport_Click(object sender, EventArgs e)
        {
            if (lbPlanList.SelectedItems.Count != 1)
                return;

            Plan plan = (Plan)lbPlanList.SelectedItems[0].Tag;
            UIHelper.ExportPlan(plan);
        }
        #endregion


        #region Context Menu
        /// <summary>
        /// When the context menu opens, we change the items states.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void contextMenu_Opening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (lbPlanList.SelectedItems.Count > 1)
            {
                cmiDelete.Enabled = true;
                cmiRename.Enabled = false;
                cmiExport.Enabled = false;
                cmiOpen.Text = "Merge";
            }
            else if (lbPlanList.SelectedItems.Count == 1)
            {
                cmiDelete.Enabled = true;
                cmiRename.Enabled = true;
                cmiExport.Enabled = true;
                cmiOpen.Enabled = true;
                cmiOpen.Text = "Open";
            }
            else
            {
                e.Cancel = true;
            }
        }

        /// <summary>
        /// When the "file" menu is opening, we enable or disable "load plans".
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mFile_DropDownOpening(object sender, EventArgs e)
        {
            // See if we have multiple characters to determine if load from character is enabled
            miLoadPlanFromCharacter.Enabled = (EveClient.Characters.Count > 1);
        }

        /// <summary>
        /// When the "edit" menu is opening, we enable or disable the options.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mEdit_DropDownOpening(object sender, EventArgs e)
        {
            miRename.Enabled = lbPlanList.SelectedItems.Count == 1;
            miExport.Enabled = lbPlanList.SelectedItems.Count == 1;
            miDelete.Enabled = lbPlanList.SelectedItems.Count >= 1;
        }

        /// <summary>
        /// Edit > Export plan
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmiExportPlan_Click(object sender, EventArgs e)
        {
            if (lbPlanList.SelectedItems.Count != 1)
                return;

            Plan plan = (Plan)lbPlanList.SelectedItems[0].Tag;
            UIHelper.ExportPlan(plan);
        }
        #endregion
    }
}
