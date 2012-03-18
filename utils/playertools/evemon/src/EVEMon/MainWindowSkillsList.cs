using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Controls;
using EVEMon.SkillPlanner;

using CommonProperties = EVEMon.Common.Properties;

namespace EVEMon
{
    public partial class MainWindowSkillsList : UserControl
    {
        // Skills drawing - Region & text padding
        private const int PadTop = 2;
        private const int PadLeft = 6;
        private const int PadRight = 7;
        private const int LineVPad = 0;

        // Skills drawing - Boxes
        private const int BoxWidth = 57;
        private const int BoxHeight = 14;
        private const int LowerBoxHeight = 8;
        private const int BoxHPad = 6;
        private const int BoxVPad = 2;
        private const int SkillDetailHeight = 31;

        // Skills drawing - Skills groups
        private const int SkillHeaderHeight = 21;
        private const int CollapserPadRight = 6;

        // Skills drawing - Font & brushes
        private readonly Font m_skillsFont;
        private readonly Font m_boldSkillsFont;
        private Object m_lastTooltipItem;
        private bool m_requireRefresh;
        private int m_count = 0;

        /// <summary>
        /// Constructor
        /// </summary>
        public MainWindowSkillsList()
        {
            InitializeComponent();

            lbSkills.Visible = false;

            m_skillsFont = FontFactory.GetFont("Tahoma", 8.25F);
            m_boldSkillsFont = FontFactory.GetFont("Tahoma", 8.25F, FontStyle.Bold);
            noSkillsLabel.Font = FontFactory.GetFont("Tahoma", 11.25F, FontStyle.Bold);

            m_requireRefresh = true;

            EveClient.CharacterChanged +=EveClient_CharacterChanged;
            EveClient.SettingsChanged += EveClient_SettingsChanged;
            EveClient.TimerTick += EveClient_TimerTick;
            this.Disposed +=OnDisposed;
        }
        
        /// <summary>
        /// Gets the character associated with this monitor.
        /// </summary>
        public Character Character { get; set; }

        #region Inherited events

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.CharacterChanged -= EveClient_CharacterChanged;
            EveClient.SettingsChanged -= EveClient_SettingsChanged;
            EveClient.TimerTick -= EveClient_TimerTick;
            this.Disposed -= OnDisposed;
        }

        /// <summary>
        /// <summary>
        /// When the control becomes visible again, we update the content.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            base.OnVisibleChanged(e);

            if (this.Visible)
                UpdateContent();
        }

        #endregion


        #region Content Management

        /// <summary>
        /// Updates all the content
        /// </summary>
        /// <remarks>
        /// Another high-complexity method for us to look at.
        /// </remarks>
        private void UpdateContent()
        {
            // Returns if not visible
            if (!this.Visible)
                return;

            // When no character, we just clear the list
            if (Character == null)
            {
                noSkillsLabel.Visible = true;
                lbSkills.Visible = false;
                lbSkills.Items.Clear();
                return;
            }

            // Update the skills list
            lbSkills.BeginUpdate();
            try
            {
                IEnumerable<Skill> skills = GetCharacterSkills();
                var groups = skills.GroupBy(x => x.Group).ToArray().OrderBy(x => x.Key.Name);

                // Scroll through groups
                lbSkills.Items.Clear();
                foreach (var group in groups)
                {
                    lbSkills.Items.Add(group.Key);

                    // Add items in the group when it's not collapsed
                    if (!Character.UISettings.CollapsedGroups.Contains(group.Key.Name))
                    {
                        foreach (var skill in group.ToArray().OrderBy(x => x.Name))
                        {
                            lbSkills.Items.Add(skill);
                        }
                    }
                }

                // Display or hide the "no skills" label.
                noSkillsLabel.Visible = skills.IsEmpty();
                lbSkills.Visible = !skills.IsEmpty();

                // Invalidate display
                lbSkills.Invalidate();
            }
            finally
            {
                lbSkills.EndUpdate();
            }
        }

        /// <summary>
        /// Gets a characters skills, filtered by MainWindow settings
        /// </summary>
        /// <returns>IEnumerable of <see cref="Skill"/>Skill</see></returns>
        private IEnumerable<Skill> GetCharacterSkills()
        {
            if (Settings.UI.MainWindow.ShowPrereqMetSkills)
                return Character.Skills.Where(x => x.IsKnown || (x.ArePrerequisitesMet && x.IsPublic));

            if (Settings.UI.MainWindow.ShowNonPublicSkills)
                return Character.Skills;

            if (Settings.UI.MainWindow.ShowAllPublicSkills)
                return Character.Skills.Where(x => x.IsKnown || x.IsPublic);

            return Character.Skills.Where(x => x.IsKnown);
        }

        #endregion


        #region Drawing
        /// <summary>
        /// Handles the DrawItem event of the lbSkills control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.DrawItemEventArgs"/> instance containing the event data.</param>
        private void lbSkills_DrawItem(object sender, DrawItemEventArgs e)
        {
            if (e.Index < 0)
                return;

            object item = lbSkills.Items[e.Index];
            if (item is SkillGroup)
            {
                DrawItem(item as SkillGroup, e);
            }
            else if (item is Skill)
            {
                DrawItem(item as Skill, e);
            }
        }

        /// <summary>
        /// Handles the MeasureItem event of the lbSkills control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.MeasureItemEventArgs"/> instance containing the event data.</param>
        private void lbSkills_MeasureItem(object sender, MeasureItemEventArgs e)
        {
            if (e.Index < 0)
                return;
            e.ItemHeight = GetItemHeight(lbSkills.Items[e.Index]);
        }

        /// <summary>
        /// Gets the item's height.
        /// </summary>
        /// <param name="e"></param>
        /// <param name="item"></param>
        private int GetItemHeight(object item)
        {
            if (item is SkillGroup)
                return SkillHeaderHeight;

            return Math.Max(m_skillsFont.Height * 2 + PadTop + LineVPad + PadTop, SkillDetailHeight);
        }

        /// <summary>
        /// Draws the list item for the given skill
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="e"></param>
        public void DrawItem(Skill skill, DrawItemEventArgs e)
        {
            Graphics g = e.Graphics;

            // Draw background
            if (skill.IsTraining)
            {
                // In training
                g.FillRectangle(Brushes.LightSteelBlue, e.Bounds);
            }
            else if ((e.Index % 2) == 0)
            {
                // Not in training - odd
                g.FillRectangle(Brushes.White, e.Bounds);
            }
            else
            {
                // Not in training - even
                g.FillRectangle(Brushes.LightGray, e.Bounds);
            }
            
            // Measure texts
            TextFormatFlags format = TextFormatFlags.NoPadding | TextFormatFlags.NoClipping;

            int skillPointsToNextLevel = skill.StaticData.GetPointsRequiredForLevel(Math.Min(skill.Level + 1, 5));
            
            string rankText = String.Format(CultureConstants.DefaultCulture, " (Rank {0})", skill.Rank);
            string spText = String.Format(CultureConstants.DefaultCulture,
                                    "SP: {0:#,##0}/{1:#,##0}", skill.SkillPoints, skillPointsToNextLevel);
            string levelText = String.Format(CultureConstants.DefaultCulture, "Level {0}", skill.Level);
            string pctText = String.Format(CultureConstants.DefaultCulture, "{0}% Done", Math.Floor(skill.PercentCompleted));

            Size skillNameSize = TextRenderer.MeasureText(g, skill.Name, m_boldSkillsFont, Size.Empty, format);
            Size rankTextSize = TextRenderer.MeasureText(g, rankText, m_skillsFont, Size.Empty, format);
            Size levelTextSize = TextRenderer.MeasureText(g, levelText, m_skillsFont, Size.Empty, format);
            Size spTextSize = TextRenderer.MeasureText(g, spText, m_skillsFont, Size.Empty, format);
            Size pctTextSize = TextRenderer.MeasureText(g, pctText, m_skillsFont, Size.Empty, format);


            // Draw texts
            Color highlightColor = Color.Black;
            if (!skill.IsKnown)
                highlightColor = Color.Red;
            if (!skill.IsPublic)
                highlightColor = Color.DarkRed;
            if (skill.ArePrerequisitesMet && skill.IsPublic && !skill.IsKnown)
                highlightColor = Color.SlateGray;
            if (Settings.UI.MainWindow.HighlightPartialSkills && skill.IsPartiallyTrained && !skill.IsTraining)
                highlightColor = Color.Green;
            if (Settings.UI.MainWindow.HighlightQueuedSkills && skill.IsQueued && !skill.IsTraining)
                highlightColor = Color.RoyalBlue;

            TextRenderer.DrawText(g, skill.Name, m_boldSkillsFont,
                                                            new Rectangle(
                                                                e.Bounds.Left + PadLeft,
                                                                e.Bounds.Top + PadTop,
                                                                skillNameSize.Width + PadLeft,
                                                                skillNameSize.Height), highlightColor);

            TextRenderer.DrawText(g, rankText, m_skillsFont,
                                                            new Rectangle(
                                                                e.Bounds.Left + PadLeft + skillNameSize.Width,
                                                                e.Bounds.Top + PadTop,
                                                                rankTextSize.Width + PadLeft,
                                                                rankTextSize.Height), highlightColor);

            TextRenderer.DrawText(g, spText, m_skillsFont,
                                                            new Rectangle(
                                                                e.Bounds.Left + PadLeft,
                                                                e.Bounds.Top + PadTop + skillNameSize.Height + LineVPad,
                                                                spTextSize.Width + PadLeft,
                                                                spTextSize.Height), highlightColor);

            // Boxes
            g.DrawRectangle(Pens.Black, new Rectangle(
                                    e.Bounds.Right - BoxWidth - PadRight, e.Bounds.Top + PadTop, BoxWidth, BoxHeight));
            
            int levelBoxWidth = (BoxWidth - 4 - 3) / 5;
            for (int level = 1; level <= 5; level++)
            {
                Rectangle brect = new Rectangle(
                                    e.Bounds.Right - BoxWidth - PadRight + 2 + (levelBoxWidth * (level - 1)) + (level - 1),
                                    e.Bounds.Top + PadTop + 2, levelBoxWidth, BoxHeight - 3);

                if (level <= skill.Level)
                {
                    g.FillRectangle(Brushes.Black, brect);
                }
                else
                {
                    g.FillRectangle(Brushes.DarkGray, brect);
                }

                // Color indicator for a queued level
                CCPCharacter ccpCharacter = Character as CCPCharacter;
                if (ccpCharacter != null)
                {
                    SkillQueue skillQueue = ccpCharacter.SkillQueue;
                    foreach (var qskill in skillQueue)
                    {
                        if ((!skill.IsTraining && skill == qskill.Skill && level == qskill.Level)
                           || (skill.IsTraining && skill == qskill.Skill && level == qskill.Level && level > skill.Level + 1))
                            g.FillRectangle(Brushes.RoyalBlue, brect);
                    }
                }
                
                // Blinking indicator of skill in training level
                if (skill.IsTraining && level == skill.Level + 1)
                {
                    if (m_count == 0)
                        g.FillRectangle(Brushes.White, brect);

                    if (m_count == 1)
                        m_count = -1;

                    m_count ++;
                }
           }

            // Draw progression bar
            g.DrawRectangle(Pens.Black, new Rectangle(e.Bounds.Right - BoxWidth - PadRight,
                                                      e.Bounds.Top + PadTop + BoxHeight + BoxVPad,
                                                      BoxWidth, LowerBoxHeight));

            Rectangle pctBarRect = new Rectangle(e.Bounds.Right - BoxWidth - PadRight + 2,
                                                 e.Bounds.Top + PadTop + BoxHeight + BoxVPad + 2,
                                                 BoxWidth - 3, LowerBoxHeight - 3);

            g.FillRectangle(Brushes.DarkGray, pctBarRect);
            int fillWidth = (int)(pctBarRect.Width * skill.FractionCompleted);
            if (fillWidth > 0)
            {
                Rectangle fillRect = new Rectangle(pctBarRect.X, pctBarRect.Y, fillWidth, pctBarRect.Height);
                g.FillRectangle(Brushes.Black, fillRect);
            }


            // Draw level and percent texts
            TextRenderer.DrawText(g, levelText, m_skillsFont,
                                                        new Rectangle(
                                                            e.Bounds.Right - BoxWidth - PadRight - BoxHPad - levelTextSize.Width,
                                                            e.Bounds.Top + PadTop, levelTextSize.Width + PadRight,
                                                            levelTextSize.Height), Color.Black);

            TextRenderer.DrawText(g, pctText, m_skillsFont,
                                                        new Rectangle(
                                                            e.Bounds.Right - BoxWidth - PadRight - BoxHPad - pctTextSize.Width,
                                                            e.Bounds.Top + PadTop + levelTextSize.Height + LineVPad,
                                                            pctTextSize.Width + PadRight, pctTextSize.Height), Color.Black);
        }

        /// <summary>
        /// Draws the list item for the given skill group.
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="e"></param>
        public void DrawItem(SkillGroup group, DrawItemEventArgs e)
        {
            Graphics g = e.Graphics;

            // Draws the background
            using (Brush b = new SolidBrush(Color.FromArgb(75, 75, 75)))
            {
                g.FillRectangle(b, e.Bounds);
            }

            using (Pen p = new Pen(Color.FromArgb(100, 100, 100)))
            {
                g.DrawLine(p, e.Bounds.Left, e.Bounds.Top, e.Bounds.Right + 1, e.Bounds.Top);
            }

            // Draw the header
            Size titleSizeInt = TextRenderer.MeasureText(
                                            g, group.Name, m_boldSkillsFont,
                                            Size.Empty,
                                            TextFormatFlags.NoPadding | TextFormatFlags.NoClipping);
            Rectangle titleTopLeftInt = new Rectangle(
                                            e.Bounds.Left + 3,
                                            e.Bounds.Top + ((e.Bounds.Height / 2) - (titleSizeInt.Height / 2)),
                                            titleSizeInt.Width + PadRight,
                                            titleSizeInt.Height);

            string skillInTrainingSuffix = String.Empty;
            bool hasTrainingSkill = group.Any(x => x.IsTraining);
            bool hasQueuedSkill = group.Any(x=> x.IsQueued && !x.IsTraining);
            if (hasTrainingSkill)
                skillInTrainingSuffix = ", ( 1 in training )";
            if (hasQueuedSkill)
                skillInTrainingSuffix += String.Format(CultureConstants.DefaultCulture,
                    ", ( {0} in queue )", group.Count(x=> x.IsQueued && !x.IsTraining));

            // Draws the rest of the text header
            string detailText = String.Format(CultureConstants.DefaultCulture,
                                              ", {0} of {1} skills, {2:#,##0} Points{3}",
                                              group.Count(x => x.IsKnown),
                                              group.Count(x => x.IsPublic),
                                              group.TotalSP,
                                              skillInTrainingSuffix);

            Size detailSizeInt = TextRenderer.MeasureText(
                g, detailText, m_skillsFont, Size.Empty, TextFormatFlags.NoPadding | TextFormatFlags.NoClipping);
            Rectangle detailTopLeftInt = new Rectangle(
                titleTopLeftInt.X + titleSizeInt.Width + 4, titleTopLeftInt.Y, detailSizeInt.Width, detailSizeInt.Height);

            TextRenderer.DrawText(g, group.Name, m_boldSkillsFont, titleTopLeftInt, Color.White);
            TextRenderer.DrawText(g, detailText, m_skillsFont, detailTopLeftInt, Color.White);

            // Draws the collapsing arrows
            bool isCollapsed = Character.UISettings.CollapsedGroups.Contains(group.Name);
            Image i = (isCollapsed ? CommonProperties.Resources.Expand : CommonProperties.Resources.Collapse);

            g.DrawImageUnscaled(i, new Point(e.Bounds.Right - i.Width - CollapserPadRight,
                                             (SkillHeaderHeight / 2) - (i.Height / 2) + e.Bounds.Top));
        }

        /// <summary>
        /// Gets the preferred size from the preferred size of the skills list.
        /// </summary>
        /// <param name="proposedSize"></param>
        /// <returns></returns>
        public override Size GetPreferredSize(Size proposedSize)
        {
            return lbSkills.GetPreferredSize(proposedSize);
        }
        #endregion


        #region Toggle / expand
        /// <summary>
        /// Toggles all the skill groups to collapse or open.
        /// </summary>
        public void ToggleAll()
        {
            // When at least one group collapsed, expand all
            if (Character.UISettings.CollapsedGroups.Count != 0)
            {
                Character.UISettings.CollapsedGroups.Clear();
            }
            // When none collapsed, collapse all
            else
            {
                foreach (var group in Character.SkillGroups)
                {
                    Character.UISettings.CollapsedGroups.Add(group.Name);
                }
            }

            // Update the list
            UpdateContent();
        }

        /// <summary>
        /// Toggles the expansion or collapsing of a single group
        /// </summary>
        /// <param name="group">The group to expand or collapse.</param>
        public void ToggleGroupExpandCollapse(SkillGroup group)
        {
            if (Character.UISettings.CollapsedGroups.Contains(group.Name))
            {
                ExpandSkillGroup(group);
            }
            else
            {
                CollapseSkillGroup(group);
            }
        }

        /// <summary>
        /// Expand a skill group
        /// </summary>
        /// <param name="group">Skill group in lbSkills</param>
        public void ExpandSkillGroup(SkillGroup group)
        {
            Character.UISettings.CollapsedGroups.Remove(group.Name);
            UpdateContent();
        }

        /// <summary>
        /// Collapse a skill group
        /// </summary>
        /// <param name="group">Skill group in lbSkills</param>
        public void CollapseSkillGroup(SkillGroup group)
        {
            Character.UISettings.CollapsedGroups.Add(group.Name);
            UpdateContent();
        }
        #endregion


        #region Local events

        /// <summary>
        /// Handles the MouseWheel event of the lbSkills control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.MouseEventArgs"/> instance containing the event data.</param>
        private void lbSkills_MouseWheel(object sender, MouseEventArgs e)
        {
            // Update the drawing based upon the mouse wheel scrolling.
            int numberOfItemLinesToMove = e.Delta * SystemInformation.MouseWheelScrollLines / 120;
            int lines = numberOfItemLinesToMove;
            if (lines == 0)
                return;

            // Compute the number of lines to move
            int direction = lines / Math.Abs(lines);
            int[] numberOfPixelsToMove = new int[lines * direction];
            for (int i = 1; i <= Math.Abs(lines); i++)
            {
                object item = null;

                // Going up
                if (direction == Math.Abs(direction))
                {
                    // Retrieve the next top item
                    if (lbSkills.TopIndex - i >= 0)
                        item = lbSkills.Items[lbSkills.TopIndex - i];
                }
                // Going down
                else
                {
                    // Compute the height of the items from current the topindex (included)
                    int height = 0; 
                    for (int j = lbSkills.TopIndex + i - 1; j < lbSkills.Items.Count; j++)
                    {
                        height += GetItemHeight(lbSkills.Items[j]);
                    }

                    // Retrieve the next bottom item
                    if (height > lbSkills.ClientSize.Height)
                        item = lbSkills.Items[lbSkills.TopIndex + i - 1];
                }

                // If found a new item as top or bottom
                if (item != null)
                {
                    numberOfPixelsToMove[i - 1] = GetItemHeight(item) * direction;
                }
                else
                {
                    lines -= direction;
                }
            }

            // Scroll 
            if (lines != 0)
                lbSkills.Invalidate();
        }

        /// <summary>
        /// On a mouse down event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbSkills_MouseDown(object sender, MouseEventArgs e)
        {
            // Retrieve the item at the given point and quit if none
            int index = lbSkills.IndexFromPoint(e.X, e.Y);
            if (index < 0 || index >= lbSkills.Items.Count)
                return;

            // Beware, this last index may actually means a click in the whitespace at the bottom
            // Let's deal with this special case
            if (index == lbSkills.Items.Count - 1)
            {
                Rectangle itemRect = lbSkills.GetItemRectangle(index);
                if (!itemRect.Contains(e.Location))
                    return;
            }

            // For a skill group, we have to handle the collapse/expand mechanism and the tooltip
            Object item = lbSkills.Items[index];
            if (item is SkillGroup)
            {
                // Left button : expand/collapse
                SkillGroup sg = (SkillGroup)item;
                if (e.Button != MouseButtons.Right)
                {
                    ToggleGroupExpandCollapse(sg);
                    return;
                }

                // If right click on the button, still expand/collapse
                Rectangle itemRect = lbSkills.GetItemRectangle(lbSkills.Items.IndexOf(item));
                Rectangle buttonRect = GetButtonRectangle(sg, itemRect);
                if (buttonRect.Contains(e.Location))
                {
                    ToggleGroupExpandCollapse(sg);
                    return;
                }

                // Regular right click, display the tooltip
                DisplayTooltip(sg);
                return;
            }

            // Right click for skills below lv5 : we display a context menu to plan higher levels.
            Skill skill = (Skill)item;
            if (e.Button == MouseButtons.Right)
            {
                // "Show in Skill Explorer" menu item
                ToolStripMenuItem tmSkillExplorer = new ToolStripMenuItem("Show In Skill Explorer", CommonProperties.Resources.LeadsTo);
                tmSkillExplorer.Click += new EventHandler(tmSkillExplorer_Click);
                tmSkillExplorer.Tag = skill;

                // Add to the context menu
                contextMenuStripPlanPopup.Items.Clear();
                contextMenuStripPlanPopup.Items.Add(tmSkillExplorer);

                if (skill.Level < 5)
                {
                    // Reset the menu.
                    ToolStripMenuItem tm = new ToolStripMenuItem(String.Format(CultureConstants.DefaultCulture, "Add {0}", skill.Name));

                    // Build the level options.
                    int nextLevel = Math.Min(5, skill.Level + 1);
                    for (int level = nextLevel; level < 6; level++)
                    {
                        ToolStripMenuItem menuLevel = new ToolStripMenuItem(
                            String.Format(CultureConstants.DefaultCulture, "Level {0} to", Skill.GetRomanForInt(level)));
                        tm.DropDownItems.Add(menuLevel);
                        Character.Plans.AddTo(menuLevel.DropDownItems, (menuPlanItem, plan) =>
                        {
                            menuPlanItem.Click += new EventHandler(menuPlanItem_Click);
                            menuPlanItem.Tag = new Pair<Plan, SkillLevel>(plan, new SkillLevel(skill, level));
                        });
                    }


                    // Add to the context menu
                    contextMenuStripPlanPopup.Items.Add(new ToolStripSeparator());
                    contextMenuStripPlanPopup.Items.Add(tm);
                }

                // Display the context menu
                contextMenuStripPlanPopup.Show((Control)sender, new Point(e.X, e.Y));
                return;
            }

            // Non-right click or already lv5, display the tooltip
            DisplayTooltip(skill);
        }

        /// <summary>
        /// On mouse move, we hide the tooltip.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void lbSkills_MouseMove(object sender, MouseEventArgs e)
        {
            for (int i = 0; i < lbSkills.Items.Count; i++)
            {
                // Skip until we found the mouse location
                var rect = lbSkills.GetItemRectangle(i);
                if (!rect.Contains(e.Location))
                    continue;

                // Updates the tooltip
                Object item = lbSkills.Items[i];
                DisplayTooltip(item);
                return;
            }

            // If we went so far, we're not over anything.
            m_lastTooltipItem = null;
            ttToolTip.Active = false;
        }

        /// <summary>
        /// Displays the tooltip for the given item (skill or skillgroup).
        /// </summary>
        /// <param name="item"></param>
        private void DisplayTooltip(Object item)
        {
            if (ttToolTip.Active && m_lastTooltipItem != null && m_lastTooltipItem == item)
                return;

            m_lastTooltipItem = item;

            ttToolTip.Active = false;
            if (item is SkillGroup)
            {
                ttToolTip.SetToolTip(lbSkills, GetTooltip(item as SkillGroup));
            }
            else
            {
                ttToolTip.SetToolTip(lbSkills, GetTooltip(item as Skill));
            }
            ttToolTip.Active = true;
        }

        /// <summary>
        /// Gets the tooltip text for the given skill
        /// </summary>
        /// <param name="skill"></param>
        private string GetTooltip(Skill skill)
        {
            int sp = skill.SkillPoints;
            int nextLevel = Math.Min(5, skill.Level + 1);
            int nextLevelSP = skill.StaticData.GetPointsRequiredForLevel(nextLevel);
            int pointsLeft = skill.GetLeftPointsRequiredToLevel(nextLevel);
            string remainingTimeText = skill.GetLeftTrainingTimeToLevel(nextLevel).ToDescriptiveText(
                DescriptiveTextOptions.IncludeCommas | DescriptiveTextOptions.UppercaseText);

            if (sp < skill.StaticData.GetPointsRequiredForLevel(1))
            {
                // Training hasn't got past level 1 yet
                StringBuilder untrainedToolTip = new StringBuilder();
                untrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                        "Not yet trained to Level I ({0}%)\n", Math.Floor(skill.PercentCompleted));
                untrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                        "Next level I: {0:#,##0} skill points remaining\n", pointsLeft);
                untrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                        "Training time remaining: {0}", remainingTimeText);
                AddSkillBoilerPlate(untrainedToolTip, skill);
                return untrainedToolTip.ToString();
            }

            // So, it's a left click on a skill, we display the tool tip
            // Partially trained skill ?
            if (skill.IsPartiallyTrained)
            {
                StringBuilder partiallyTrainedToolTip = new StringBuilder();
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                            "Partially Completed ({0}%)\n", Math.Floor(skill.PercentCompleted));
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                            "Training to level {0}: {1:#,##0} skill points remaining\n",
                                            Skill.GetRomanForInt(nextLevel), pointsLeft);
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                            "Training time remaining: {0}", remainingTimeText);
                AddSkillBoilerPlate(partiallyTrainedToolTip, skill);
                return partiallyTrainedToolTip.ToString();
            }

            // We've completed all the skill points for the current level
            if (!skill.IsPartiallyTrained)
            {
                if (skill.Level != 5)
                {
                    StringBuilder levelCompleteToolTip = new StringBuilder();
                    levelCompleteToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                            "Completed Level {0}: {1:#,##0}/{2:#,##0}\n",
                                             Skill.GetRomanForInt(skill.Level), sp, nextLevelSP);
                    levelCompleteToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                            "Next level {0}: {1:#,##0} skill points required\n",
                                             Skill.GetRomanForInt(nextLevel), pointsLeft);
                    levelCompleteToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                            "Training Time: {0}", remainingTimeText);
                    AddSkillBoilerPlate(levelCompleteToolTip, skill);
                    return levelCompleteToolTip.ToString();
                }

                // Lv 5 completed
                StringBuilder lv5ToolTip = new StringBuilder();
                lv5ToolTip.AppendFormat(CultureConstants.DefaultCulture, "Level V Complete: {0:#,##0}/{1:#,##0}\n", sp, nextLevelSP);
                lv5ToolTip.Append("No further training required\n");
                AddSkillBoilerPlate(lv5ToolTip, skill);
                return lv5ToolTip.ToString();
            }

            // Error in calculating SkillPoints
            StringBuilder calculationErrorToolTip = new StringBuilder();
            calculationErrorToolTip.AppendLine("Partially Trained (Could not cacluate all skill details)");
            calculationErrorToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                        "Next level {0}: {1:#,##0} skill points remaining\n",nextLevel, pointsLeft);
            calculationErrorToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                        "Training time remaining: {0}", remainingTimeText);
            AddSkillBoilerPlate(calculationErrorToolTip, skill);
            return calculationErrorToolTip.ToString();
        }

        /// <summary>
        /// Adds the skill boiler plate.
        /// </summary>
        /// <param name="toolTip">The tool tip.</param>
        /// <param name="skill">The skill.</param>
        private static void AddSkillBoilerPlate(StringBuilder toolTip, Skill skill)
        {
            toolTip.Append("\n\n");
            toolTip.AppendLine(skill.DescriptionNL);
            toolTip.AppendFormat(CultureConstants.DefaultCulture, "\nPrimary: {0}, ", skill.PrimaryAttribute);
            toolTip.AppendFormat(CultureConstants.DefaultCulture, "Secondary: {0} ", skill.SecondaryAttribute);
            toolTip.AppendFormat(CultureConstants.DefaultCulture, "({0:#,##0} SP/hour)", skill.SkillPointsPerHour);
        }

        /// <summary>
        /// Gets the tooltip content for the given skill group
        /// </summary>
        /// <param name="sg"></param>
        private string GetTooltip(SkillGroup group)
        {
            // Maximas are computed on public skills only
            int totalValidSP = group.Where(x => x.IsPublic).Sum(x => x.SkillPoints);
            int maxSP = group.Where(x => x.IsPublic).Sum(x => x.StaticData.GetPointsRequiredForLevel(5));
            int maxKnown = group.Where(x => x.IsPublic).Count();

            // Current achievements are computed on every skill, including non-public
            int totalSP = group.Sum(x => x.SkillPoints);
            int known = group.Where(x => x.IsKnown).Count();

            // If the group is not completed yet
            if (totalValidSP < maxSP)
            {
                double percentDonePoints = (1.0 * Math.Min(totalSP, maxSP)) / maxSP;
                double percentDoneSkills = (1.0 * Math.Min(known, maxKnown)) / maxKnown;

                return String.Format(CultureConstants.DefaultCulture,
                    "Points Completed: {0:#,##0} of {1:#,##0} ({2:P1})\nSkills Known: {3} of {4} ({5:P0})",
                    totalSP, maxSP, percentDonePoints, known, maxKnown, percentDoneSkills);
            }

            // The group has been completed !
            return String.Format(CultureConstants.DefaultCulture,
                "Skill Group completed: {0:#,##0}/{1:#,##0} (100%)\nSkills: {2:#}/{3:#} (100%)",
                totalSP, maxSP, known, maxKnown);
        }

        /// <summary>
        /// Gets the rectangle for the collapse/expand button.
        /// </summary>
        /// <param name="itemRect"></param>
        /// <returns></returns>
        public Rectangle GetButtonRectangle(SkillGroup group, Rectangle itemRect)
        {
            // Checks whether this group is collapsed
            bool isCollapsed = Character.UISettings.CollapsedGroups.Contains(group.Name);

            // Get the image for this state
            Image btnImage = (isCollapsed ? btnImage = CommonProperties.Resources.Expand : btnImage = CommonProperties.Resources.Collapse);

            // Compute the top left point
            Point btnPoint = new Point(itemRect.Right - btnImage.Width - CollapserPadRight,
                                       (SkillHeaderHeight / 2) - (btnImage.Height / 2) + itemRect.Top);

            return new Rectangle(btnPoint, btnImage.Size);
        }

        /// <summary>
        /// Handler for a plan item click on the plan's context menu.
        /// Add a skill to the plan.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void menuPlanItem_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem planItem = (ToolStripMenuItem)sender;
            var tag = (Pair<Plan, SkillLevel>)planItem.Tag;

            var operation = tag.A.TryPlanTo(tag.B.Skill, tag.B.Level);
            PlanHelper.SelectPerform(operation);
        }

        /// <summary>
        /// Shows the selected skill in Skill Explorer
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tmSkillExplorer_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem item = (ToolStripMenuItem)sender;
            Skill skill = (Skill) item.Tag;

            var window = WindowsFactory<SkillExplorerWindow>.ShowUnique();
            window.Skill = skill;
        }
        #endregion


        #region Global events
        /// <summary>
        /// On timer tick, we invalidate the training skill display
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_TimerTick(object sender, EventArgs e)
        {
            // We trigger a refresh of the list to eliminate a designer leftover (scrollbar)
            if (m_requireRefresh)
            {
                lbSkills.Invalidate();
                m_requireRefresh = false;
            }
            
            if (Character.IsTraining && this.Visible)
            {
                // Retrieves the trained skill for update but quit if the skill is null (was not in our datafiles)
                var training = Character.CurrentlyTrainingSkill;
                if (training.Skill == null)
                    return;

                // Invalidate the skill row
                int index = lbSkills.Items.IndexOf(training.Skill);
                if (index >= 0)
                    lbSkills.Invalidate(lbSkills.GetItemRectangle(index));

                // Invalidate the skill group row
                int groupIndex = lbSkills.Items.IndexOf(training.Skill.Group);
                if (groupIndex >= 0)
                    lbSkills.Invalidate(lbSkills.GetItemRectangle(groupIndex));
            }
        }

        /// <summary>
        /// When the character changed, we refresh the content
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            if (e.Character != Character)
                return;

            UpdateContent();
        }

        /// <summary>
        /// When the settings changed, we refresh the content (show all public skills, non-public skills, etc)
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            UpdateContent();
        }
        #endregion

    }
}
