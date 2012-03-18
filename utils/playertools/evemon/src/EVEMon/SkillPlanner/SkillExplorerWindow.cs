using System;
using System.Linq;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon;
using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.Data;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Skill Explorer Window - allows easy exploration of skills.
    /// </summary>
    /// <remarks>
    /// This class is the Skill Explorer - or "What items/ships other skills does this skill enable for me" form.
    /// For a given skill, we show 2 tree controls (with splitter bars seperating them), one for skills, one for either ships or items, depending on the skill we're exploring.
    /// (There is no skill that enables both a ship and an item)
    /// Each tree shows a node for each level of the chosen skill with child nodes for all items that this level
    /// is a direct prerequisite for. If we can use the item, it is shown in a normal font. If this level of skill would 
    /// enable use of the item, we show it dimmed. If this level is one of a number of missing skills for the item, then
    /// we show the item in red (with a tooltip indicating the other missing skills).
    /// The user can chose to view the trees in alphabetic order, or by the categories (alpha is default).
    /// For items, you can chose to show just the base T1/T2 items, or all variants of an item (default is base items)
    /// 
    /// There's a dropdown listbox that keeps track of what skills you've been exploring in this session.
    /// 
    /// Each node has a context menu that enables you to view the item in the relevant browser, add missing skills to 
    /// your plan, show list of all prerequisites (including costs if unowned), and for the skill tree, use the selected skill as a new base skill.
    /// 
    /// Doubleclicking a ship/item leaf node will show the selected item in the browser.
    /// Doubleclicking a skill in the tree will set that skill as the base skill we're exploring
    /// </remarks>
    public partial class SkillExplorerWindow : EVEMonForm
    {
        private PlanWindow m_planWindow;
        private Character m_character;
        private Skill m_skill;
        private bool m_hasItems;
        private bool m_hasShips;
        private bool m_hasBlueprints;

        #region Constructor

        /// <summary>
        /// Constructor for designer.
        /// </summary>
        public SkillExplorerWindow()
        {
            InitializeComponent();
            splitContainer1.RememberDistanceKey = "SkillExplorer";

            toolTip.SetToolTip(cbHistory, "A history of the skills that you have been looking at");
        }

        /// <summary>
        /// Constructor for use in code
        /// </summary>
        /// <param name="skill">The skill we want to analyze</param>
        /// <param name="planWindow">The plan window</param>
        public SkillExplorerWindow(Skill skill, PlanWindow planWindow) 
            : this()
        {
            m_planWindow = planWindow;
            Skill = skill;

            EveClient.PlanChanged += new EventHandler<PlanChangedEventArgs>(EveClient_PlanChanged);
            EveClient.CharacterChanged += new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterChanged);
        }

        #endregion


        #region Inherited Events

        /// <summary>
        /// Unsubscribe events on closing.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosing(CancelEventArgs e)
        {
            EveClient.CharacterChanged -= new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterChanged);
            EveClient.PlanChanged -= new EventHandler<PlanChangedEventArgs>(EveClient_PlanChanged);
            base.OnClosing(e);
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets or sets the represented skill.
        /// </summary>
        public Skill Skill
        {
            get { return m_skill; }
            set
            {
                if (m_skill == value)
                    return;
                
                // we can't unset a skill
                if (value == null)
                    return;

                m_character = value.Character;
                m_skill = value;

                // If already in history combo, remove it to reinsert it at the top
                if (cbHistory.Items.Contains(m_skill.Name))
                {
                    cbHistory.Items.RemoveAt(cbHistory.Items.IndexOf(m_skill.Name));
                }
                cbHistory.Items.Insert(0, m_skill.Name);
                cbHistory.SelectedIndex = 0;

                UpdateContent();
            }
        }

        #endregion


        #region Content creation and update
        /// <summary>
        /// Update all the content.
        /// </summary>
        private void UpdateContent()
        {
            UpdatePlanName();
            UpdateHeader();
            UpdateTrees();
        }

        /// <summary>
        /// Update the character and plan name on the skill header group title.
        /// </summary>
         private void UpdatePlanName()
        {
            if (m_planWindow == null)
                return;

            grpPlanName.Text = String.Format(CultureConstants.DefaultCulture, "{0} - {1}", m_character.Name, m_planWindow.Plan.Name);
        }

        /// <summary>
        /// Sets the details for the skill we're analysing.
        /// Work out if we know the skill, if so, how many  points.
        /// If we don't know it, show if we could train it now or not
        /// and indicate if we own the book, if not, show the cost.
        /// </summary>
        private void UpdateHeader()
        {
            StringBuilder sb = new StringBuilder();
            if (m_skill.IsTraining)
                sb.AppendFormat(CultureConstants.DefaultCulture, "{0} - In Training ", m_skill.Name);

            sb.Append("(");
            if (m_skill.IsKnown)
            {
                sb.AppendFormat(CultureConstants.DefaultCulture, "Trained to level {0} with {1}", m_skill.Level,
                    (m_skill.SkillPoints > 0 ? String.Format(CultureConstants.DefaultCulture,"{0:0,0,0} sp", m_skill.SkillPoints) : "0 sp"));
            }
            else
            {
                sb.AppendFormat(CultureConstants.DefaultCulture, "Not Trained - prereqs {0}met, skillbook is {1}",
                    (m_skill.ArePrerequisitesMet ? String.Empty : "not "),
                    (m_skill.IsOwned ? "owned" : String.Format(CultureConstants.DefaultCulture, "not owned, book costs {0} ISK", m_skill.FormattedCost)));
            }
            sb.Append(")");

            lblSkillInfo.Text = sb.ToString();
            tmrAutoUpdate.Enabled = m_skill.IsTraining;
        }

        /// <summary>
        /// Recreate the trees and their headers.
        /// 
        /// Items, ships and skills may be presented alphabetically (skill level > enabled skill) or grouped (skill level > enabled skill group > enabled skill).
        /// 
        /// Each item is colored appropriately : normal if we know all the prereqs for the item,
        /// dimmed if this level enables the item or red if we need other skills to enable the item in
        /// addition to this level of skill.
        /// </summary>
        private void UpdateTrees()
        {
            UpdateItemsTree();
            UpdateSkillsTree();
            UpdateTreesHeaders();
        }

        /// <summary>
        /// Update the trees' headers : "blueprints", "items" or "ships".
        /// </summary>
        private void UpdateTreesHeaders()
        {
            if (m_hasBlueprints && m_hasItems)
            {
                lblItems.BackColor = Color.Thistle;
                pnlItemHeader.BackColor = Color.Thistle;
                lblItems.Text = "Enabled Blueprints and Items";
            }
            else if (m_hasShips && m_hasBlueprints)
            {
                lblItems.BackColor = Color.Lavender;
                pnlItemHeader.BackColor = Color.Lavender;
                lblItems.Text = "Enabled Ships and Blueprints";
            }
            else if (m_hasShips && m_hasItems)
            {
                lblItems.BackColor = Color.Honeydew;
                pnlItemHeader.BackColor = Color.Honeydew;
                lblItems.Text = "Enabled Ships and Items";
            }
            else if (m_hasBlueprints)
            {
                lblItems.BackColor = Color.LightBlue;
                pnlItemHeader.BackColor = Color.LightBlue;
                lblItems.Text = "Enabled Blueprints";
            }
            else if (m_hasShips)
            {
                lblItems.BackColor = Color.LightCyan;
                pnlItemHeader.BackColor = Color.LightCyan;
                lblItems.Text = "Enabled Ships";
            }
            else if (m_hasItems)
            {
                lblItems.BackColor = Color.MistyRose;
                pnlItemHeader.BackColor = Color.MistyRose;
                lblItems.Text = "Enabled Items";
            }
            else
            {
                lblItems.BackColor = Color.WhiteSmoke;
                pnlItemHeader.BackColor = Color.WhiteSmoke;
                lblItems.Text = "Enabled Ships, Blueprints or Items";
            }
        }

        /// <summary>
        /// Set up the enabled Skills tree.
        /// </summary>
        private void UpdateSkillsTree()
        {
            tvSkills.BeginUpdate();
            try
            {
                tvSkills.Nodes.Clear();
                if (m_skill == null)
                    return;

                for (int i = 1; i <= 5; i++)
                {
                    SkillLevel skillLevel = new SkillLevel(m_skill, i);

                    // Gets the enabled skills and check it's not empty
                    Skill[] enabledSkills = m_skill.Character.Skills.Where(x => x.Prerequisites.Any(y => y.Skill == m_skill && y.Level == i) && x.IsPublic).ToArray();
                    if (enabledSkills.IsEmpty())
                        continue;

                    // Add a node for this skill level
                    TreeNode levelNode = new TreeNode(skillLevel.ToString());
                    if (m_skill.Level >= i)
                        levelNode.Text += " (Trained)";

                    levelNode.ForeColor = Color.DarkBlue;

                    // Is it a plain alphabetical presentation ?
                    if (rbShowAlpha.Checked)
                    {
                        foreach (Skill skill in enabledSkills.OrderBy(x => x.Name))
                        {
                            levelNode.Nodes.Add(CreateNode(skill, skill.Prerequisites));
                        }
                    }
                    // Or do we need to group skills by their groups ?
                    else if (rbShowTree.Checked)
                    {
                        foreach (IGrouping<SkillGroup, Skill> group in enabledSkills
                            .GroupBy(x => x.Group).ToArray().OrderBy(x => x.Key.Name))
                        {
                            TreeNode groupNode = new TreeNode(group.Key.Name);
                            foreach (Skill skill in group.ToArray().OrderBy(x => x.Name))
                            {
                                groupNode.Nodes.Add(CreateNode(skill, skill.Prerequisites));
                            }
                            levelNode.Nodes.Add(groupNode);
                        }
                    }

                    // Add node
                    levelNode.Expand();
                    tvSkills.Nodes.Add(levelNode);
                }

                // No enabled skill found for any level ?
                if (tvSkills.Nodes.Count == 0)
                    tvSkills.Nodes.Add(new TreeNode("No skills enabled by this skill"));

            }
            finally
            {
                tvSkills.EndUpdate();
            }
        }

        /// <summary>
        /// Set up the items/ships/blueprints tree.
        /// </summary>
        private void UpdateItemsTree()
        {
            m_hasShips = false;
            m_hasItems = false;
            m_hasBlueprints = false;

            tvEntity.BeginUpdate();
            try
            {
                tvEntity.Nodes.Clear();
                if (m_skill == null)
                    return;

                List<Item> items = new List<Item>(StaticItems.AllItems.Concat(StaticBlueprints.AllBlueprints).
                    Where(x => x.MarketGroup.ParentGroup != null && x.MarketGroup.ParentGroup.ID != DBConstants.SkillGroupID). // exclude skills
                    Where(x => x.Prerequisites.Any(y => y.Skill == m_skill.StaticData)).
                    Where(x => !cbShowBaseOnly.Checked || x.MetaGroup == ItemMetaGroup.T1 || x.MetaGroup == ItemMetaGroup.T2));

                // Scroll through levels
                for (int i = 1; i <= 5; i++)
                {
                    SkillLevel skillLevel = new SkillLevel(m_skill, i);

                    // Gets the enabled objects and check it's not empty
                    IEnumerable<Item> enabledObjects = items.Where(x => x.Prerequisites.Any(y => y.Skill == m_skill.StaticData && y.Level == i));
                    if (enabledObjects.IsEmpty())
                        continue;

                    // Add a node for this skill level
                    TreeNode levelNode = new TreeNode(skillLevel.ToString());
                    if (m_skill.Level >= i)
                        levelNode.Text += " (Trained)";

                    levelNode.ForeColor = Color.DarkBlue;

                    // Is it a plain alphabetical presentation ?
                    if (rbShowAlpha.Checked)
                    {
                        foreach (Item ship in enabledObjects.Where(x => x is Ship).ToArray().OrderBy(x => x.Name))
                        {
                            levelNode.Nodes.Add(CreateNode(ship, ship.Prerequisites.ToCharacter(m_character)));
                            m_hasShips = true;
                        }

                        foreach (Item blueprint in enabledObjects.Where(x => x is Blueprint).ToArray().OrderBy(x => x.Name))
                        {
                            List<string> listOfActivities = blueprint.Prerequisites.Where(x => x.Skill == m_skill.StaticData && x.Level == i)
                                                                        .Select(x => x.Activity.GetDescription()).ToList();
                            TreeNode node = CreateNode(blueprint, blueprint.Prerequisites
                                .Where(x => listOfActivities.Contains(x.Activity.GetDescription())).ToCharacter(m_character));
                            node.Text = String.Format("{0} ({1})", node.Text, string.Join(", ", listOfActivities));
                            levelNode.Nodes.Add(node);
                            m_hasBlueprints = true;
                        }

                        foreach (Item item in enabledObjects.Where(x => !(x is Ship) && !(x is Blueprint)).ToArray().OrderBy(x => x.Name))
                        {
                            levelNode.Nodes.Add(CreateNode(item, item.Prerequisites.ToCharacter(m_character)));
                            m_hasItems = true;
                        }
                    }
                    // Or do we need to group items by their groups ?
                    else if (rbShowTree.Checked)
                    {
                        // Add ships
                        IGrouping<MarketGroup, Item>[] shipsToAdd = enabledObjects.Where(x => x is Ship).GroupBy(x => x.MarketGroup.ParentGroup).ToArray();
                        foreach (IGrouping<MarketGroup, Item> shipGroup in shipsToAdd.OrderBy(x => x.Key.Name))
                        {
                            TreeNode groupNode = new TreeNode(shipGroup.Key.Name);
                            foreach (Item ship in shipGroup.ToArray().OrderBy(x => x.Name))
                            {
                                groupNode.Nodes.Add(CreateNode(ship, ship.Prerequisites.ToCharacter(m_skill.Character)));
                            }
                            levelNode.Nodes.Add(groupNode);
                            m_hasShips = true;
                        }

                        // Add blueprints recursively                       
                        foreach (BlueprintMarketGroup blueprintMarketGroup in StaticBlueprints.BlueprintMarketGroups)
                        {
                            foreach (TreeNode node in CreateMarketGroupsNode(blueprintMarketGroup, enabledObjects.Where(x => x is Blueprint), i))
                            {
                                levelNode.Nodes.Add(node);
                                m_hasBlueprints = true;
                            }
                        }

                        // Add items recursively
                        foreach (MarketGroup marketGroup in StaticItems.MarketGroups)
                        {
                            foreach (TreeNode node in CreateMarketGroupsNode(marketGroup, enabledObjects.Where(x => !(x is Ship) && !(x is Blueprint))))
                            {
                                levelNode.Nodes.Add(node);
                                m_hasItems = true;
                            }
                        }
                    }

                    // Add node
                    levelNode.Expand();
                    tvEntity.Nodes.Add(levelNode);
                }
                
                // No enabled skill found for any level ?
                if (tvEntity.Nodes.Count == 0)
                    tvEntity.Nodes.Add(new TreeNode("No ships, blueprints or items enabled by this skill"));
            }
            finally
            {
                tvEntity.EndUpdate();
            }
        }

        /// <summary>
        /// Recursively creates tree nodes for the children market groups of the given group.
        /// The added blueprints will be the ones which require the current skill (<see cref="m_skill"/>) at the specified level.
        /// </summary>
        /// <param name="blueprintMarketGroup">The blueprint market group.</param>
        /// <param name="blueprints">The blueprints.</param>
        /// <param name="level">The level.</param>
        /// <returns></returns>
        private IEnumerable<TreeNode> CreateMarketGroupsNode(BlueprintMarketGroup blueprintMarketGroup, IEnumerable<Item> blueprints, int level)
        {
            // Add categories
            foreach (BlueprintMarketGroup category in blueprintMarketGroup.SubGroups)
            {
                IEnumerable<TreeNode> children = CreateMarketGroupsNode(category, blueprints, level);
                if (children.IsEmpty())
                    continue;

                TreeNode node = new TreeNode(category.Name);
                node.Nodes.AddRange(children.ToArray());
                yield return node;
            }

            // Add blueprints
            foreach (Item blueprint in blueprints.Where(x => x.MarketGroup == blueprintMarketGroup))
            {
                List<string> listOfActivities = blueprint.Prerequisites
                                        .Where(x => x.Skill == m_skill.StaticData && x.Level == level)
                                        .Select(x => x.Activity.GetDescription()).ToList();

                TreeNode node = CreateNode(blueprint, blueprint.Prerequisites
                                        .Where(x => listOfActivities.Contains(x.Activity.GetDescription())).ToCharacter(m_character));
                node.Text = String.Format("{0} ({1})", node.Text, string.Join(", ", listOfActivities));
                yield return node;
            }
        }

        /// <summary>
        /// Recursively creates tree nodes for the children market groups of the given group.
        /// The added items will be the ones which require the current skill (<see cref="m_skill"/>) at the specified level.
        /// </summary>
        /// <param name="marketGroup">The market group.</param>
        /// <param name="items">The items.</param>
        /// <returns></returns>
        private IEnumerable<TreeNode> CreateMarketGroupsNode(MarketGroup marketGroup, IEnumerable<Item> items)
        {
            // Add categories
            foreach (MarketGroup category in marketGroup.SubGroups)
            {
                IEnumerable<TreeNode> children = CreateMarketGroupsNode(category, items);
                if (children.IsEmpty())
                    continue;

                TreeNode node = new TreeNode(category.Name);
                node.Nodes.AddRange(children.ToArray());
                yield return node;
            }

            // Add items
            foreach (Item item in items.Where(x => x.MarketGroup == marketGroup))
            {
                yield return CreateNode(item, item.Prerequisites.ToCharacter(m_character));
            }
        }

        /// <summary>
        /// Adds a node to the tree, and colors it appropriately, and set the tooltip.
        /// Also sets the tag of the node to the Skill object.
        /// </summary>
        /// <param name="obj">The object.</param>
        /// <param name="prerequisites">The object's prerequisites.</param>
        /// <returns></returns>
        private TreeNode CreateNode(Object obj, IEnumerable<SkillLevel> prerequisites)
        {
            TreeNode node = new TreeNode(obj.ToString());
            node.ToolTipText = String.Empty;
            node.Tag = obj;

            // When all prereqs satisifed, keep the default color
            if (prerequisites.All(x => x.IsKnown))
                    return node;

            // Are all other prerequisites known ?
            if (prerequisites.All(x => x.IsKnown || x.Skill == m_skill))
            {
                node.ForeColor = Color.Gray;
                return node;
            }

            // Then we need to list the other prerequisites
            StringBuilder sb = new StringBuilder("Also Need To Train:");
            foreach (SkillLevel prereq in CreatePrereqList(prerequisites).Where(x => x.Skill != m_skill && !x.IsKnown))
            {
                sb.AppendLine().Append(prereq.ToString());
            }

            node.ToolTipText = sb.ToString();
            node.ForeColor = Color.Red;
            return node;
        }

        /// <summary>
        /// Creates a prereq skills list for a blueprint with the given activity.
        /// </summary>
        /// <param name="prerequisites">The prerequisites of a blueprint activities</param>
        /// <param name="prereqList">A list of the unique prerequisites skills for the given activities</param>
        private List<SkillLevel> CreatePrereqList(IEnumerable<SkillLevel> prerequisites)
        {
            List<SkillLevel> prereqList = new List<SkillLevel>();
            foreach (SkillLevel prereq in prerequisites)
            {
                if (prereqList.Contains(prereq))
                    continue;

                prereqList.Add(prereq);
            }

            return prereqList;
        }
        #endregion


        #region Global events and auto-update
        /// <summary>
        /// Occurs whenever the plan changes
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_PlanChanged(object sender, PlanChangedEventArgs e)
        {
            UpdatePlanName();
        }

        /// <summary>
        /// occurs whenever the character is updated from CCP, skills are estimed to have completed, etc.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            if (e.Character != m_character)
                return;

            UpdateContent();
        }

        /// <summary>
        /// Update skill header every 30s if we're training it
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tmrAutoUpdate_Tick(object sender, EventArgs e)
        {
            UpdateHeader();
        }
        #endregion


        #region Skills context Menu
        /// <summary>
        /// Returns the currently selected skill or null if non is selected.
        /// </summary>
        /// <returns></returns>
        private Skill GetSelectedSkill()
        {
            if (tvSkills.SelectedNode == null)
                return null;

            return tvSkills.SelectedNode.Tag as Skill;
        }

        /// <summary>
        /// When the user clicks the node, we select it and checks whether we must display the context menu.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void tvSkills_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            if (e.Button != MouseButtons.Right)
                return;

            // Updates selection
            tvSkills.SelectedNode = e.Node;

            // Display menu only when we access the skill explorer through a plan
            if (m_planWindow == null)
                return;
           
            // Do not display menu for non-skill nodes
            Skill skill = GetSelectedSkill();
            if (skill == null)
                return;

            // Update the "plan to X" menus
            tsAddPlan.Enabled = false;
            if (skill.Level < 5)
                tsAddPlan.Enabled = true;

            for (int i = 1; i <= 5; i++)
            {
                PlanHelper.UpdatesRegularPlanToMenu(tsAddPlan.DropDownItems[i - 1], m_planWindow.Plan, skill, i);
            }

            // Update the "show prerequisites" menu
            tsShowSkillPrereqs.Enabled = !skill.ArePrerequisitesMet;

            // Show menu
            cmSkills.Show(tvSkills, e.Location);
        }

        /// <summary>
        /// Skill context menu > Plan to level 
        /// theres a menu item for each level, each one is tagged with a 
        /// string representing level number.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsAddLevel_Click(object sender, EventArgs e)
        {
            IPlanOperation operation = ((ToolStripMenuItem)sender).Tag as IPlanOperation;
            PlanHelper.SelectPerform(operation);
        }

        /// <summary>
        /// Skill context menu > Show me what this skill unlocks.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsSwitch_Click(object sender, EventArgs e)
        {
            Skill skill = GetSelectedSkill();
            if (skill == null)
                return;

            Skill = skill;
        }

        /// <summary>
        /// Skill context menu > Show skill in browser.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsShowInBrowser_Click(object sender, EventArgs e)
        {
            Skill skill = GetSelectedSkill();
            if (skill == null)
                return;

            m_planWindow.ShowSkillInBrowser(skill);
        }

        /// <summary>
        /// Skill Menu - Show all prereq stats in a dialog box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsShowSkillPrereqs_Click(object sender, EventArgs e)
        {
            Skill skill = GetSelectedSkill();
            if (skill == null)
                return;

            int index = 0;
            StringBuilder sb = new StringBuilder();
            foreach (SkillLevel prereq in skill.Prerequisites)
            {
                FormatPrerequisite(sb, prereq, ref index);
            }

            MessageBox.Show(sb.ToString(), "Untrained Prerequisites for " + skill.Name, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        /// <summary>
        /// Helper method for the Show Prereqs menu used by both ship and item trees.
        /// This method adds one prereq to the string that will be displayed in the
        /// dialog box.
        /// </summary>
        /// <param name="sb">The StringBuilder object.</param>
        /// <param name="prereq">The prereq</param>
        /// <param name="index">The index.</param>
        private void FormatPrerequisite(StringBuilder sb, SkillLevel prereq, ref int index)
        {
            if (prereq.Skill.IsKnown)
            {
                // We know this prereq, but not to a high enough level
                if (!prereq.IsKnown)
                {
                    index++;
                    string level = prereq.Skill.Level > 0 ? String.Format(CultureConstants.DefaultCulture, "(Trained to level {0})", prereq.Skill.RomanLevel) : "(Not yet trained)";
                    sb.AppendFormat(CultureConstants.DefaultCulture, "{0}. {1} {2}\n", index, prereq.ToString(), level);
                }
                return;
            }

            // We don't know this prereq at all
            index++;
            sb.AppendFormat(CultureConstants.DefaultCulture, "{0}. {1} (Prereqs {2}met, skillbook {3}", index, prereq.ToString(),
                (prereq.Skill.Prerequisites.AreTrained() ? String.Empty : "not "),
                (prereq.Skill.IsOwned ? "owned)": String.Format(CultureConstants.DefaultCulture, "not owned,\n costs {0} ISK)\n", prereq.Skill.FormattedCost)));
        }
        #endregion


        #region Items Ships and Blueprints context menu
        /// <summary>
        /// Returns the currently selected item or null if non is selected.
        /// </summary>
        /// <returns></returns>
        private Item GetSelectedItem()
        {
            if (tvEntity.SelectedNode == null)
                return null;

            return tvEntity.SelectedNode.Tag as Item;
        }

        /// <summary>
        /// Gets the selected item activities.
        /// </summary>
        /// <param name="entity">The entity.</param>
        /// <returns></returns>
        private List<string> GetSelectedItemActivities(Item entity)
        {
            List<string> list = tvEntity.SelectedNode.Text.Replace(entity.Name, String.Empty).Trim().Trim("()".ToCharArray()).Split(',').ToList();

            if (list[0] == String.Empty)
                list[0] = (BlueprintActivity.None.GetDescription());

            return list;
        }

        /// <summary>
        /// When the user clicks the node, we select it and checks whether we must display the context menu.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void tvEntity_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            if (e.Button != MouseButtons.Right)
                return;

            // Updates selection
            tvEntity.SelectedNode = e.Node;
            
            // Display menu only when we access the skill explorer through a plan
            if (m_planWindow == null)
                return;

            // Display menu only for items, ships or blueprint nodes (not market groups and level nodes)
            Item entity = GetSelectedItem();
            if (entity == null)
                return;

            // "Add to plan" is enabled if we don't know all the prereqs 
            // and we're not already planning at least one of the unknown prereqs
            List<string> listOfActivities = GetSelectedItemActivities(entity);
            bool canPlan = entity.Prerequisites.Where(x => listOfActivities.Contains(x.Activity.GetDescription())).ToCharacter(m_character).Any(x => !x.IsKnown && !m_planWindow.Plan.IsPlanned(x.Skill, x.Level));
            tsShowObjectPrereqs.Enabled = canPlan;
            tsAddObjectToPlan.Enabled = canPlan;

            // Other menus
            tsShowObjectInBrowser.Text = String.Format(CultureConstants.DefaultCulture, "Show \"{0}\" In Browser", entity.Name);

            // Show menu
            cmEntity.Show(tvEntity, e.Location);
        }

        /// <summary>
        /// Shared context menu - add ship/item/blueprint to plan.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsAddEntityToPlan_Click(object sender, EventArgs e)
        {
            Item entity = GetSelectedItem();
        
            if (entity == null)
                return;

            List<string> listOfActivities = GetSelectedItemActivities(entity);
            IPlanOperation operation = m_planWindow.Plan.TryAddSet(entity.Prerequisites.Where(x => listOfActivities.Contains(x.Activity.GetDescription())), entity.Name);
            PlanHelper.Perform(operation);
        }

        /// <summary>
        /// Shared Ship/Blueprint/Item Show Prereqs menu.
        /// Builds a nicely formatted list of prereqs for the ship/item/blueprint
        /// and shows them in a dialog box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsShowItemPrereqs_Click(object sender, EventArgs e)
        {
            Item entity = GetSelectedItem();
            if (entity == null)
                return;

            List<string> listOfActivities = GetSelectedItemActivities(entity);
            List<SkillLevel> prereqList = CreatePrereqList(entity.Prerequisites
                                            .Where(x => listOfActivities.Contains(x.Activity.GetDescription())).ToCharacter(m_character));

            int index = 0;
            StringBuilder sb = new StringBuilder();
            foreach (SkillLevel prereq in prereqList)
            {
                FormatPrerequisite(sb, prereq, ref index);
            }

            MessageBox.Show(sb.ToString(), "Untrained Prerequisites for " + entity.Name, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
        #endregion


        #region Controls' events
        /// <summary>
        /// Toggling the radio buttons to switch between sorted list and category views
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void rbShowAlpha_CheckedChanged(object sender, EventArgs e)
        {
            UpdateTrees();
        }

        /// <summary>
        /// Toggling the "Show base items/show variants",
        /// just redisplay the items tree
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbShowBaseOnly_CheckedChanged(object sender, EventArgs e)
        {
            UpdateItemsTree();
        }

        /// <summary>
        /// We're done!
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        /// <summary>
        /// Doubleclicks on a ship/item leaf node will show the ship/item in the browser
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvEntity_DoubleClick(object sender, EventArgs e)
        {
            if (m_planWindow == null)
                return;

            Item item = GetSelectedItem();
            if (item == null)
                return;
            
            Item ship = item as Ship;
            Item blueprint = item as Blueprint;

            if (ship != null)
            {
                m_planWindow.ShowShipInBrowser(ship);
            }
            else if (blueprint != null)
            {
                m_planWindow.ShowBlueprintInBrowser(item);
            }
            else if (item != null)
            {
                m_planWindow.ShowItemInBrowser(item);
            }
        }

        /// <summary>
        /// Sklll context menu > Show me what this skill unlocks
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvSkills_DoubleClick(object sender, EventArgs e)
        {
            Skill skill = GetSelectedSkill();
            if (skill == null)
                return;

            Skill = skill;
        }

        /// <summary>
        /// We want to go look at a skill in the history list again
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbHistory_SelectedIndexChanged(object sender, EventArgs e)
        {
            string skillName = cbHistory.Items[cbHistory.SelectedIndex] as string;
            Skill = m_character.Skills[skillName];
        }
        #endregion 
    }
}
