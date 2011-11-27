using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.SettingsObjects;

using DescriptionAttribute = System.ComponentModel.DescriptionAttribute;

namespace EVEMon.SkillPlanner
{
    public partial class SkillSelectControl : UserControl
    {
        public event EventHandler<EventArgs> SelectedSkillChanged;

        private bool m_hostedInSkillBrowser;
        private bool m_allExpanded;
        private Character m_character;
        private Skill m_selectedSkill;
        private Plan m_plan;
        private IEnumerable<Skill> m_skills;

        #region Lifecycle

        /// <summary>
        /// Constructor
        /// </summary>
        public SkillSelectControl()
        {
            InitializeComponent();
       }

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            Disposed -= OnDisposed;
            EveClient.SettingsChanged -= EveClient_SettingsChanged;
        }

        /// <summary>
        /// On load, restore settings and update the content
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void SkillSelectControl_Load(object sender, EventArgs e)
        {
            if (DesignMode || this.IsDesignModeHosted())
                return;

            EveClient.SettingsChanged += EveClient_SettingsChanged;
            Disposed += OnDisposed;

            cbShowNonPublic.Checked = Settings.UI.SkillBrowser.ShowNonPublicSkills;
            cbSorting.SelectedIndex = (int)Settings.UI.SkillBrowser.Sort;
            cbSkillFilter.SelectedIndex = (int)Settings.UI.SkillBrowser.Filter;

            if (Settings.UI.UseStoredSearchFilters)
            {
                tbSearchText.Text = Settings.UI.SkillBrowser.TextSearch;
                lbSearchTextHint.Visible = String.IsNullOrEmpty(tbSearchText.Text);
            }

            // Updates the controls
            EveClient_SettingsChanged(null, EventArgs.Empty);
        }

        /// <summary>
        /// When the settings are changed, update the display
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            pbSearchImage.Visible = !Settings.UI.SafeForWork;
            
            UpdateContent();
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets or sets the plan.
        /// </summary>
        public Plan Plan
        {
            get { return m_plan; }
            set 
            {
                if (m_plan == value)
                    return;

                m_plan = value;
                m_character = (Character)m_plan.Character;
            }
        }

        /// <summary>
        /// Gets the selected skill
        /// </summary>
        public Skill SelectedSkill
        {
            get { return m_selectedSkill; }
            set
            {
                if (m_selectedSkill == value)
                    return;
                m_selectedSkill = value;

                // Expands the tree view
                tvItems.SelectNodeWithTag(value);

                // Notify subscribers
                if (SelectedSkillChanged != null)
                {
                    SelectedSkillChanged(this, new EventArgs());
                }
            }
        }

        /// <summary>
        /// Gets or sets true whether the control is hosted in the skill browser. When true, the "Show in skill browser" context menus won't be displayed.
        /// </summary>
        [Category("Behavior")]
        [Description("When true, the \"Show in Skill Browser\" context menus won't be displayed.")]
        [DefaultValue(false)]
        public bool HostedInSkillBrowser
        {
            get { return m_hostedInSkillBrowser; }
            set { m_hostedInSkillBrowser = value; }
        }


        #endregion


        #region Icon set
        /// <summary>
        /// Gets the icon set for the given index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public ImageList GetIconSet(int index)
        {
            return GetIconSet(index, ilSkillIcons);
        }

        /// <summary>
        /// Gets the icon set for the given index, using the given list for missing icons.
        /// </summary>
        /// <param name="index"></param>
        /// <param name="defaultList"></param>
        /// <returns></returns>
        public static ImageList GetIconSet(int index, ImageList defaultList)
        {
            ImageList def = new ImageList();
            def.ColorDepth = ColorDepth.Depth32Bit;
            string groupname = null;
            if (index > 0 && index < EVEMon.Common.Resources.Skill_Select.IconSettings.Default.Properties.Count)
            {
                groupname = EVEMon.Common.Resources.Skill_Select.IconSettings.Default.Properties["Group" + index].DefaultValue.ToString();
            }
            if ((groupname != null && !System.IO.File.Exists(String.Format(
                        "{1}Resources{0}Skill_Select{0}Group{2}{0}{3}.resources",
                        Path.DirectorySeparatorChar,
                        System.AppDomain.CurrentDomain.BaseDirectory,
                        index,
                        groupname)) ||
                !System.IO.File.Exists(String.Format(
                        "{1}Resources{0}Skill_Select{0}Group0{0}Default.resources",
                        Path.DirectorySeparatorChar,
                        System.AppDomain.CurrentDomain.BaseDirectory))))
            {
                groupname = null;
            }
            if (groupname != null)
            {
                System.Resources.IResourceReader basic = new System.Resources.ResourceReader(String.Format(
                        "{1}Resources{0}Skill_Select{0}Group0{0}Default.resources",
                        Path.DirectorySeparatorChar,
                        System.AppDomain.CurrentDomain.BaseDirectory));
                IDictionaryEnumerator basicx = basic.GetEnumerator();
                while (basicx.MoveNext())
                {
                    def.Images.Add(basicx.Key.ToString(), (Icon)basicx.Value);
                }
                basic.Close();
                basic = new System.Resources.ResourceReader(String.Format(
                        "{1}Resources{0}Skill_Select{0}Group{2}{0}{3}.resources",
                        Path.DirectorySeparatorChar,
                        System.AppDomain.CurrentDomain.BaseDirectory,
                        index,
                        groupname));
                basicx = basic.GetEnumerator();
                while (basicx.MoveNext())
                {
                    if (def.Images.ContainsKey(basicx.Key.ToString()))
                    {
                        def.Images.RemoveByKey(basicx.Key.ToString());
                    }
                    def.Images.Add(basicx.Key.ToString(), (Icon)basicx.Value);
                }
                basic.Close();
                return def;
            }
            else
            {
                return defaultList;
            }
        }
        #endregion


        #region Content creation and update
        /// <summary>
        /// Updates the skills list content
        /// </summary>
        public void UpdateContent()
        {
            if (m_plan == null)
                return;

            m_skills = GetFilteredData();
            UpdateTree();

            tvItems.Visible = false;
            lbSearchList.Visible = false;
            lvSortedSkillList.Visible = false;
            lbNoMatches.Visible = false;

            // Nothing to display ?
            if (m_skills.IsEmpty())
            {
                lbNoMatches.Visible = true;
            }
            // Is it sorted ?
            else if (cbSorting.SelectedIndex != 0)
            {
                lvSortedSkillList.Visible = true;
                UpdateListView();
           }
            // Not sorted but there is a text filter
            else if (!String.IsNullOrEmpty(tbSearchText.Text))
            {
                lbSearchList.Visible = true;
                UpdateListBox();
            }
            // Regular display, the tree
            else
            {
                tvItems.Visible = true;
            }
        }

        /// <summary>
        /// Gets the filtered skills.
        /// </summary>
        /// <returns></returns>
        private IEnumerable<Skill> GetFilteredData()
        {
            IEnumerable<Skill> skills = m_character.Skills;

            // Non-public skills
            if (!cbShowNonPublic.Checked)
                skills = skills.Where(x => x.IsPublic);

            // Filter
            var predicate = GetFilter();
            skills = skills.Where(x => predicate(x));

            // Text search
            if (!String.IsNullOrEmpty(tbSearchText.Text))
            {
                string searchText = tbSearchText.Text.ToLower(CultureConstants.DefaultCulture).Trim();
                skills = skills.Where(x => (x.Name.ToLower(CultureConstants.DefaultCulture).Contains(searchText)
                                         || x.Description.ToLower(CultureConstants.DefaultCulture).Contains(searchText)));
            }

            // When sorting by "time to...", remove lv5 skills
            if (cbSorting.SelectedIndex == (int)SkillSort.TimeToLevel5 || cbSorting.SelectedIndex == (int)SkillSort.TimeToNextLevel)
                skills = skills.Where(x => x.Level < 5);

            return skills;
        }

        /// <summary>
        /// Gets the skill predicate for the selected combo filter index.
        /// </summary>
        /// <returns></returns>
        private Func<Skill, bool> GetFilter()
        {
            switch ((SkillFilter)cbSkillFilter.SelectedIndex)
            {
                default:
                case SkillFilter.All:
                    return (x) => true;

                case SkillFilter.Known:
                    return (x) => x.IsKnown;

                case SkillFilter.TrailAccountFriendly:
                    return (x) => x.IsTrainableOnTrialAccount;

                case SkillFilter.Unknown:
                    return (x) => !x.IsKnown;

                case SkillFilter.UnknownAndNotOwned:
                    return (x) => !x.IsKnown && !x.IsOwned;

                case SkillFilter.UnknownButOwned:
                    return (x) => !x.IsKnown && x.IsOwned;

                case SkillFilter.UnknownButTrainable:
                    return (x) => !x.IsKnown && x.ArePrerequisitesMet;

                case SkillFilter.UnknownAndNotTrainable:
                    return (x) => !x.IsKnown && !x.ArePrerequisitesMet;

                case SkillFilter.Planned:
                    return (x) => m_plan.IsPlanned(x);

                case SkillFilter.Lv1Ready:
                    return (x) => x.Level == 0 && x.ArePrerequisitesMet && !x.IsPartiallyTrained;

                case SkillFilter.Trainable:
                    return (x) => x.ArePrerequisitesMet && x.Level < 5;

                case SkillFilter.PartiallyTrained:
                    return (x) => x.IsPartiallyTrained;

                case SkillFilter.NotPlanned:
                    return (x) => !(m_plan.IsPlanned(x) || x.Level == 5);

                case SkillFilter.NotPlannedButTrainable:
                    return (x) => !m_plan.IsPlanned(x) && x.ArePrerequisitesMet && x.Level < 5;

                case SkillFilter.NoLv5:
                    return (x) => x.Level < 5;
            }
        }

        /// <summary>
        /// Updates the skills tree.
        /// </summary>
        private void UpdateTree()
        {
            //Reset selected object
            SelectedSkill = null;

            // Update the image list choice
            int iconGroupIndex = Settings.UI.SkillBrowser.IconsGroupIndex;
            if (iconGroupIndex == 0)
                iconGroupIndex = 1;

            tvItems.ImageList = GetIconSet(iconGroupIndex);
            tvItems.ImageList.ColorDepth = ColorDepth.Depth32Bit;

            // Rebuild the nodes
            int numberOfItems = 0;
            tvItems.BeginUpdate();
            try
            {
                tvItems.Nodes.Clear();
                foreach (var group in m_skills.GroupBy(x => x.Group).ToArray().OrderBy(x => x.Key.Name))
                {
                    int index = tvItems.ImageList.Images.IndexOfKey("book");

                    TreeNode groupNode = new TreeNode()
                    { 
                        Text = group.Key.Name,
                        ImageIndex = index,
                        SelectedImageIndex = index,
                        Tag = group.Key
                    };

                    // Add nodes for skills in this group
                    foreach (var skill in group)
                    {
                        // Choose image index
                        int imageIndex = -1;
                        if (skill.Level != 0)
                        {
                            imageIndex = tvItems.ImageList.Images.IndexOfKey("lvl" + skill.Level);
                        }
                        else if (skill.IsKnown)
                        {
                            imageIndex = tvItems.ImageList.Images.IndexOfKey("lvl0");
                        }
                        else if (skill.IsOwned)
                        {
                            imageIndex = tvItems.ImageList.Images.IndexOfKey("Book");
                        }
                        else if (skill.ArePrerequisitesMet) // prereqs met
                        {
                            imageIndex = tvItems.ImageList.Images.IndexOfKey("PrereqsMet");
                        }
                        else
                        {
                            imageIndex = tvItems.ImageList.Images.IndexOfKey("PrereqsNOTMet");
                        }

                        // Create node and adds it
                        TreeNode node = new TreeNode()
                        {
                            Text = String.Format("{0} ({1})", skill.Name, skill.Rank),
                            ImageIndex = imageIndex,
                            SelectedImageIndex = imageIndex,
                            Tag = skill
                        };


                        // We color some nodes
                        if (!skill.IsPublic && Settings.UI.SkillBrowser.ShowNonPublicSkills)
                            node.ForeColor = Color.DarkRed;

                        if (skill.IsPartiallyTrained && Settings.UI.PlanWindow.HighlightPartialSkills)
                            node.ForeColor = Color.Green;

                        if (skill.IsQueued && !skill.IsTraining && Settings.UI.PlanWindow.HighlightQueuedSkills)
                            node.ForeColor = Color.RoyalBlue;

                        if (skill.IsTraining)
                        {
                            node.BackColor = Color.LightSteelBlue;
                            node.ForeColor = Color.Black;
                        }

                        numberOfItems++;
                        groupNode.Nodes.Add(node);
                    }

                    // Add group when not empty
                    tvItems.Nodes.Add(groupNode);
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
        /// Updates the list box displayed when there is a text filter and no sort criteria.
        /// </summary>
        /// <param name="skills"></param>
        private void UpdateListBox()
        {
            lbSearchList.BeginUpdate();
            try
            {
                lbSearchList.Items.Clear();
                foreach (var skill in m_skills)
                {
                    lbSearchList.Items.Add(skill);
                }
            }
            finally
            {
                lbSearchList.EndUpdate();
            }
        }

        /// <summary>
        /// Updates the listview displayed when there is a sort criteria.
        /// </summary>
        /// <param name="skills"></param>
        private void UpdateListView()
        {
            // Retrieve the data to fetch into the list
            IEnumerable<string> labels = null;
            string column = GetSortedListData(ref m_skills, ref labels);
            if (labels == null)
                return;

            // Update the listview
            lvSortedSkillList.BeginUpdate();
            try
            {
                lvSortedSkillList.Items.Clear();

                using (var labelsEnumerator = labels.GetEnumerator())
                {
                    foreach (var skill in m_skills)
                    {
                        // Retrieves the label for the second column (sort key)
                        labelsEnumerator.MoveNext();
                        var label = labelsEnumerator.Current;

                        // Creates the item and adds it
                        ListViewItem lvi = new ListViewItem(skill.Name);
                        lvi.SubItems.Add(label);
                        lvi.Tag = skill;

                        lvSortedSkillList.Items.Add(lvi);
                    }
                }

                // Auto adjust column widths
                chSortKey.AutoResize(ColumnHeaderAutoResizeStyle.ColumnContent);
                chName.Width = lvSortedSkillList.ClientSize.Width - chSortKey.Width;
                chSortKey.Text = column;
            }
            finally
            {
                lvSortedSkillList.EndUpdate();
            }
        }

        /// <summary>
        /// Gets the data for the sorted list view
        /// </summary>
        /// <param name="skills"></param>
        /// <param name="labels"></param>
        /// <returns></returns>
        private string GetSortedListData(ref IEnumerable<Skill> skills, ref IEnumerable<string> labels)
        {
            switch ((SkillSort)cbSorting.SelectedIndex)
            {
                // Sort by name, default, occurs on initialization
                default:
                    return String.Empty;

                // Time to next level
                case SkillSort.TimeToNextLevel:
                    var times = skills.Select(x => m_character.GetTrainingTimeToMultipleSkills(x.Prerequisites) + x.GetLeftTrainingTimeToNextLevel());

                    var timesArray = times.ToArray();
                    var skillsArray = skills.ToArray();
                    Array.Sort(timesArray, skillsArray);

                    var labelsArray = new string[skillsArray.Length];
                    for (int i = 0; i < labelsArray.Length; i++)
                    {
                        var time = timesArray[i];
                        if (time == TimeSpan.Zero)
                        {
                            labelsArray[i] = "-";
                        }
                        else
                        {
                            labelsArray[i] = String.Format(CultureConstants.DefaultCulture, "{0}: {1}",
                                Skill.GetRomanForInt(skillsArray[i].Level + 1),
                                time.ToDescriptiveText(DescriptiveTextOptions.Default));
                        }
                    }

                    skills = skillsArray;
                    labels = labelsArray;

                    return "Time";

                // Time to level 5
                case SkillSort.TimeToLevel5:
                    times = skills.Select(
                        x => m_character.GetTrainingTimeToMultipleSkills(x.Prerequisites).Add(x.GetLeftTrainingTimeToLevel(5)));

                    timesArray = times.ToArray();
                    skillsArray = skills.ToArray();
                    Array.Sort(timesArray, skillsArray);

                    labelsArray = new string[skillsArray.Length];
                    for (int i = 0; i < labelsArray.Length; i++)
                    {
                        var time = timesArray[i];
                        if (time == TimeSpan.Zero)
                        {
                            labelsArray[i] = "-";
                        }
                        else
                        {
                            labelsArray[i] = time.ToDescriptiveText(DescriptiveTextOptions.Default);
                        }
                    }

                    skills = skillsArray;
                    labels = labelsArray;

                    return "Time to V";

                // Skill rank
                case SkillSort.Rank:
                    skills = skills.ToArray().OrderBy(x => x.Rank);
                    labels = skills.Select(x => x.Rank.ToString());
                    return "Rank";

                // Skill SP/hour
                case SkillSort.SPPerHour:
                    skills = skills.ToArray().OrderBy(x => x.SkillPointsPerHour).Reverse();
                    labels = skills.Select(x => x.SkillPointsPerHour.ToString());
                    return "SP/hour";
            }
        }
        #endregion


        #region Event Handlers
        /// <summary>
        /// When the user click the text search hint, he actually wants to select the text box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lblSearchTextHint_Click(object sender, EventArgs e)
        {
            tbSearchText.Focus();
        }

        /// <summary>
        /// When the user begins to write in the text box, we don't want to bother him with the text search hint.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tbSearch_Enter(object sender, EventArgs e)
        {
            lbSearchTextHint.Visible = false;
        }

        /// <summary>
        /// When the user leaves the text box, we display the text search hint if the box is empty.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tbSearch_Leave(object sender, EventArgs e)
        {
            lbSearchTextHint.Visible = String.IsNullOrEmpty(tbSearchText.Text);
        }

        /// <summary>
        /// When the search text box changes, we update the content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tbSearch_TextChanged(object sender, EventArgs e)
        {
            Settings.UI.SkillBrowser.TextSearch = tbSearchText.Text;
            UpdateContent();
        }

        /// <summary>
        /// When the selection of the listbox changes, updates the control's selection and fires the event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbSearchList_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedSkill = (lbSearchList.SelectedIndex >= 0 ? (Skill) lbSearchList.Items[lbSearchList.SelectedIndex] : null);
        }

        /// <summary>
        /// When the selection of the tree changes, updates the control's selection and fires the event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvSkillList_AfterSelect(object sender, TreeViewEventArgs e)
        {
            TreeNode tn = e.Node;
            Skill gs = tn.Tag as Skill;
            SelectedSkill = gs;
        }

        /// <summary>
        /// When the selection of the listview changes, updates the control's selection and fires the event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lvSortedSkillList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lvSortedSkillList.SelectedItems.Count == 0)
            {
                SelectedSkill = null;
            }
            else
            {
                ListViewItem lvi = lvSortedSkillList.SelectedItems[0];
                SelectedSkill = lvi.Tag as Skill;
            }
        }

        /// <summary>
        /// When the "show non public" checkbox is checked, we update settings and the content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbShowNonPublic_CheckedChanged(object sender, EventArgs e)
        {
            Settings.UI.SkillBrowser.ShowNonPublicSkills = cbShowNonPublic.Checked;
            UpdateContent();
        }

        /// <summary>
        /// When the sorting combo box changes, we update settings and the content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbSorting_SelectedIndexChanged(object sender, EventArgs e)
        {
            Settings.UI.SkillBrowser.Sort = (SkillSort)cbSorting.SelectedIndex;
            UpdateContent();
        }

        /// <summary>
        /// When the filter combo box changes, we update the settings and the content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbSkillFilter_SelectedIndexChanged(object sender, EventArgs e)
        {
            Settings.UI.SkillBrowser.Filter = (SkillFilter)cbSkillFilter.SelectedIndex;
            UpdateContent();
        }

        /// <summary>
        /// When the user clicks the search text box, we select the whole text.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tbSearch_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == (int)Keys.LButton)
            {
                tbSearchText.SelectAll();
                e.Handled = true;
            }
        }

        /// <summary>
        /// Changes the selection when you right click on a skill node
        /// </summary>
        /// <remarks>
        /// This fixes an issue with XP (and possibly Vista) where
        /// right click does not select the list item the mouse is
        /// over. in Windows 7 (Beta) this is default behaviour and
        /// this event has no effect.
        /// </remarks>
        /// <param name="sender">is tvItems</param>
        /// <param name="e">is a member of tvItems.Items</param>
        private void tvItems_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
                tvItems.SelectedNode = e.Node;
        }

        /// <summary>
        /// Changes the selection when you right click on a search
        /// </summary>
        /// <remarks>
        /// This fixes an issue with XP (and possibly Vista) where
        /// right click does not select the list item the mouse is
        /// over. in Windows 7 (Beta) this is default behaviour and
        /// this event has no effect.
        /// </remarks>
        /// <param name="sender">is lbSearchList</param>
        /// <param name="e"></param>
        private void lbSearchList_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
                lbSearchList.SelectedIndex = lbSearchList.IndexFromPoint(e.Location);
        }

        /// <summary>
        /// When the user begins dragging a skill from the treeview, we start a drag'n drop operation.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvItems_ItemDrag(object sender, ItemDragEventArgs e)
        {
            var node = tvItems.SelectedNode as TreeNode;
            if (node == null)
                return;

            if (node.Nodes.Count == 0)
            {
                Skill skill = node.Tag as Skill;
                if (skill == null || m_plan.GetPlannedLevel(skill) == 5 || skill.Level == 5)
                    return;

                DoDragDrop(node, DragDropEffects.Move);
            }
        }
        #endregion


        #region Context menu
        /// <summary>
        /// When the tree's context menu opens, we update the submenus' statuses.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmSkills_Opening(object sender, CancelEventArgs e)
        {
            Skill skill = null;
            var node = tvItems.SelectedNode;
            if (node != null)
                skill = tvItems.SelectedNode.Tag as Skill;

            tsSeparatorBrowser.Visible = (node != null);

            // "Show in skill browser/explorer"
            showInSkillExplorerMenu.Visible = (skill != null);
            showInSkillBrowserMenu.Visible = (skill != null && !m_hostedInSkillBrowser);

            // "Collapse" and "Expand" menus
            cmiCollapseSelected.Visible = (skill == null && node != null && node.IsExpanded);
            cmiExpandSelected.Visible = (skill == null && node != null && !node.IsExpanded);

            cmiExpandSelected.Text = (skill == null && node != null && !node.IsExpanded ? String.Format("Expand {0}", node.Text) : String.Empty);
            cmiCollapseSelected.Text = (skill == null && node != null && node.IsExpanded ? String.Format("Collapse {0}", node.Text) : String.Empty);

            // "Expand All" and "Collapse All" menus
            cmiCollapseAll.Enabled = cmiCollapseAll.Visible = m_allExpanded;
            cmiExpandAll.Enabled = cmiExpandAll.Visible = !cmiCollapseAll.Enabled;

            // "Plan to N" menus
            cmiPlanTo.Enabled = (skill != null && skill.Level < 5);
            if (skill != null)
            {
                for (int i = 0; i <= 5; i++)
                {
                    PlanHelper.UpdatesRegularPlanToMenu(cmiPlanTo.DropDownItems[i], m_plan, SelectedSkill, i);
                }
            }
        }
        
        /// <summary>
        /// When the list's context menu opens, we update the menus status
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmListSkills_Opening(object sender, CancelEventArgs e)
        {
            if (SelectedSkill == null)
            {
                e.Cancel = true;
                return;
            }

            // "Plan to N" menus
            for (int i = 0; i <= 5; i++)
            {
                PlanHelper.UpdatesRegularPlanToMenu(cmiLvPlanTo.DropDownItems[i], m_plan, SelectedSkill, i);
            }

            // "Show in skill browser"
            showInSkillBrowserListMenu.Visible = !m_hostedInSkillBrowser;
        }

        /// <summary>
        /// Listview, listbox, and tree view contexts menu > Plan to > Plan to N
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void planToLevelMenuItem_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem levelItem = (ToolStripMenuItem)sender;
            var operation = levelItem.Tag as IPlanOperation;
            PlanHelper.SelectPerform(operation);
        }

        /// <summary>
        /// Treeview's context menu > Collapse
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmiCollapseSelected_Click(object sender, EventArgs e)
        {
            tvItems.SelectedNode.Collapse();
        }

        /// <summary>
        /// Treeview's context menu > Expand
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmiExpandSelected_Click(object sender, EventArgs e)
        {
            tvItems.SelectedNode.Expand();
        }

        /// <summary>
        /// Treeview's context menu > Expand all
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmiExpandAll_Click(object sender, EventArgs e)
        {
            tvItems.ExpandAll();
            m_allExpanded = true;
        }

        /// <summary>
        /// Treeview's context menu > Collapse all
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmiCollapseAll_Click(object sender, EventArgs e)
        {
            tvItems.CollapseAll();
            m_allExpanded = false;
        }

        /// <summary>
        /// Context menu > Show In Skill Browser
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
            npw.ShowSkillInBrowser(SelectedSkill);
        }

        /// <summary>
        /// Context menu > Show In Skill Explorer
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
            npw.ShowSkillInExplorer(SelectedSkill);
        }
        #endregion

    }
}
