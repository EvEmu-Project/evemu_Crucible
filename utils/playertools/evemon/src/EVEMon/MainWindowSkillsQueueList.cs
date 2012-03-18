using System;
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
    public partial class MainWindowSkillsQueueList : UserControl
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
        private const int MinimumHeight = 39;

        // Skills drawing - Font & brushes
        private readonly Font m_boldSkillsQueueFont;
        private readonly Font m_skillsQueueFont;

        private CCPCharacter m_ccpCharacter;

        private int m_count;
        private QueuedSkill m_item;
        private Object m_lastTooltipItem;
        private DateTime m_nextRepainting = DateTime.MinValue;
        private QueuedSkill[] m_skillQueue;

        /// <summary>
        /// Constructor
        /// </summary>
        public MainWindowSkillsQueueList()
        {
            InitializeComponent();

            lbSkillsQueue.Visible = false;

            m_skillsQueueFont = FontFactory.GetFont("Tahoma", 8.25F);
            m_boldSkillsQueueFont = FontFactory.GetFont("Tahoma", 8.25F, FontStyle.Bold);
            noSkillsQueueLabel.Font = FontFactory.GetFont("Tahoma", 11.25F, FontStyle.Bold);

            EveClient.CharacterChanged += EveClient_CharacterChanged;
            EveClient.QueuedSkillsCompleted += EveClient_QueuedSkillsCompleted;
            EveClient.SettingsChanged += EveClient_SettingsChanged;
            EveClient.TimerTick += EveClient_TimerTick;
            Disposed += OnDisposed;
        }

        /// <summary>
        /// Gets the character associated with this monitor.
        /// </summary>
        public Character Character { get; set; }

        /// <summary>
        /// Gets the item's height.
        /// </summary>
        private int GetItemHeight
        {
            get
            {
                return Math.Max((m_skillsQueueFont.Height * 2) + PadTop + LineVPad + PadTop + LowerBoxHeight,
                                MinimumHeight);
            }
        }

        #region Inherited events

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.CharacterChanged -= EveClient_CharacterChanged;
            EveClient.QueuedSkillsCompleted -= EveClient_QueuedSkillsCompleted;
            EveClient.SettingsChanged -= EveClient_SettingsChanged;
            EveClient.TimerTick -= EveClient_TimerTick;
            Disposed -= OnDisposed;
        }

        /// <summary>
        /// When the control becomes visible again, we update the content.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            base.OnVisibleChanged(e);

            if (Visible)
                UpdateContent();
        }

        #endregion

        #region Display update

        /// <summary>
        /// Updates all the content
        /// </summary>
        private void UpdateContent()
        {
            // Returns if not visible
            if (!Visible)
                return;

            // When no character, we just clear the list
            if (Character == null)
            {
                noSkillsQueueLabel.Visible = true;
                lbSkillsQueue.Visible = false;
                lbSkillsQueue.Items.Clear();
                return;
            }

            m_ccpCharacter = Character as CCPCharacter;

            // If the character is not a CCPCharacter it does not have a skill queue
            if (m_ccpCharacter == null)
                return;

            // When the skill queue hasn't changed don't do anything
            if (!QueueHasChanged(m_ccpCharacter.SkillQueue.ToArray()))
                return;

            m_skillQueue = m_ccpCharacter.SkillQueue.ToArray();

            // Update the skills queue list
            lbSkillsQueue.BeginUpdate();
            try
            {
                // Add items in the list
                lbSkillsQueue.Items.Clear();
                foreach (QueuedSkill skill in m_ccpCharacter.SkillQueue)
                {
                    lbSkillsQueue.Items.Add(skill);
                }

                // Display or hide the "no queue skills" label.
                noSkillsQueueLabel.Visible = m_ccpCharacter.SkillQueue.IsEmpty();
                lbSkillsQueue.Visible = !m_ccpCharacter.SkillQueue.IsEmpty();

                // Invalidate display
                lbSkillsQueue.Invalidate();
            }
            finally
            {
                lbSkillsQueue.EndUpdate();
            }
        }

        /// <summary>
        /// Check if the queue list has changed
        /// </summary>
        public bool QueueHasChanged(QueuedSkill[] queue)
        {
            if (m_skillQueue == null)
                return true;

            if (queue.Length != m_skillQueue.Length)
                return true;

            for (int i = 0; i < queue.Length; i++)
            {
                if (queue[i] != m_skillQueue[i])
                    return true;
            }

            return false;
        }

        #endregion

        #region Drawing

        /// <summary>
        /// Handles the DrawItem event of the lbSkillsQueue control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.DrawItemEventArgs"/> instance containing the event data.</param>
        private void lbSkillsQueue_DrawItem(object sender, DrawItemEventArgs e)
        {
            if (e.Index < 0)
                return;

            m_item = lbSkillsQueue.Items[e.Index] as QueuedSkill;
            DrawItem(m_item, e);
        }

        /// <summary>
        /// Handles the MeasureItem event of the lbSkillsQueue control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.MeasureItemEventArgs"/> instance containing the event data.</param>
        private void lbSkillsQueue_MeasureItem(object sender, MeasureItemEventArgs e)
        {
            if (e.Index < 0)
                return;

            e.ItemHeight = GetItemHeight;
        }

        /// <summary>
        /// Draws the list item for the given skill
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="e"></param>
        public void DrawItem(QueuedSkill skill, DrawItemEventArgs e)
        {
            Graphics g = e.Graphics;

            // Draw background
            if (e.Index%2 == 0)
            {
                // Not in training - odd
                g.FillRectangle(Brushes.LightGray, e.Bounds);
            }
            else
            {
                // Not in training - even
                g.FillRectangle(Brushes.White, e.Bounds);
            }


            // Measure texts
            var format = TextFormatFlags.NoPadding | TextFormatFlags.NoClipping;

            double percentCompleted = 0;
            int skillPoints = (skill.Skill == null ? 0 : skill.Skill.SkillPoints);
            int skillPointsToNextLevel = (skill.Skill == null
                                              ? 0
                                              : skill.Skill.StaticData.GetPointsRequiredForLevel(
                                                    Math.Min(skill.Level, 5)));

            if (skill.Skill != null && skill.Level == skill.Skill.Level + 1)
                percentCompleted = skill.Skill.PercentCompleted;

            if (skill.Skill != null && skill.Level > skill.Skill.Level + 1)
                skillPoints = skill.CurrentSP;

            string rankText = String.Format(CultureConstants.DefaultCulture, " (Rank {0})",
                                            (skill.Skill == null ? 0 : skill.Skill.Rank));
            string spText = String.Format(CultureConstants.DefaultCulture, "SP: {0:#,##0}/{1:#,##0}", skillPoints,
                                          skillPointsToNextLevel);
            string levelText = String.Format(CultureConstants.DefaultCulture, "Level {0}", skill.Level);
            string pctText = String.Format(CultureConstants.DefaultCulture, "{0}% Done", Math.Floor(percentCompleted));

            Size skillNameSize = TextRenderer.MeasureText(g, skill.SkillName, m_boldSkillsQueueFont, Size.Empty, format);
            Size rankTextSize = TextRenderer.MeasureText(g, rankText, m_skillsQueueFont, Size.Empty, format);
            Size levelTextSize = TextRenderer.MeasureText(g, levelText, m_skillsQueueFont, Size.Empty, format);
            Size spTextSize = TextRenderer.MeasureText(g, spText, m_skillsQueueFont, Size.Empty, format);
            Size pctTextSize = TextRenderer.MeasureText(g, pctText, m_skillsQueueFont, Size.Empty, format);


            // Draw texts
            Color highlightColor = Color.Black;

            TextRenderer.DrawText(g, skill.SkillName, m_boldSkillsQueueFont,
                                  new Rectangle(e.Bounds.Left + PadLeft, e.Bounds.Top + PadTop,
                                                skillNameSize.Width + PadLeft, skillNameSize.Height), highlightColor);
            TextRenderer.DrawText(g, rankText, m_skillsQueueFont,
                                  new Rectangle(e.Bounds.Left + PadLeft + skillNameSize.Width, e.Bounds.Top + PadTop,
                                                rankTextSize.Width + PadLeft, rankTextSize.Height), highlightColor);
            TextRenderer.DrawText(g, spText, m_skillsQueueFont,
                                  new Rectangle(e.Bounds.Left + PadLeft,
                                                e.Bounds.Top + PadTop + skillNameSize.Height + LineVPad,
                                                spTextSize.Width + PadLeft, spTextSize.Height), highlightColor);


            // Boxes
            g.DrawRectangle(Pens.Black,
                            new Rectangle(e.Bounds.Right - BoxWidth - PadRight, e.Bounds.Top + PadTop, BoxWidth,
                                          BoxHeight));

            int levelBoxWidth = (BoxWidth - 4 - 3)/5;
            for (int level = 1; level <= 5; level++)
            {
                var brect =
                    new Rectangle(e.Bounds.Right - BoxWidth - PadRight + 2 + (levelBoxWidth * (level - 1)) + (level - 1),
                                  e.Bounds.Top + PadTop + 2, levelBoxWidth, BoxHeight - 3);

                if (skill.Skill != null && level <= skill.Skill.Level)
                {
                    g.FillRectangle(Brushes.Black, brect);
                }
                else
                {
                    g.FillRectangle(Brushes.DarkGray, brect);
                }

                // Color indicator for a queued level
                SkillQueue skillQueue = m_ccpCharacter.SkillQueue;
                if (skill.Skill != null)
                {
                    Brush brush = (Settings.UI.SafeForWork ? Brushes.Gray : Brushes.RoyalBlue);

                    foreach (QueuedSkill qskill in skillQueue)
                    {
                        if ((!skill.Skill.IsTraining && skill == qskill && level == qskill.Level)
                            || (skill == qskill && level <= qskill.Level && level > skill.Skill.Level
                            && percentCompleted == 0))
                            g.FillRectangle(brush, brect);

                        // Blinking indicator of skill level in training
                        if (skill.Skill.IsTraining && skill == qskill && level == skill.Level
                            && percentCompleted > 0)
                        {
                            if (m_count == 0)
                                g.FillRectangle(Brushes.White, brect);

                            if (m_count == 1)
                                m_count = -1;

                            m_count++;
                        }
                    }
                }
            }

            // Draw progression bar
            g.DrawRectangle(Pens.Black,
                            new Rectangle(e.Bounds.Right - BoxWidth - PadRight,
                                          e.Bounds.Top + PadTop + BoxHeight + BoxVPad, BoxWidth, LowerBoxHeight));

            var pctBarRect = new Rectangle(e.Bounds.Right - BoxWidth - PadRight + 2,
                                           e.Bounds.Top + PadTop + BoxHeight + BoxVPad + 2,
                                           BoxWidth - 3, LowerBoxHeight - 3);

            g.FillRectangle(Brushes.DarkGray, pctBarRect);
            var fillWidth = (int)(pctBarRect.Width * (percentCompleted / 100));
            if (fillWidth > 0)
            {
                var fillRect = new Rectangle(pctBarRect.X, pctBarRect.Y, fillWidth, pctBarRect.Height);
                g.FillRectangle(Brushes.Black, fillRect);
            }


            // Draw level and percent texts
            TextRenderer.DrawText(g, levelText, m_skillsQueueFont, new Rectangle(
                                                                       e.Bounds.Right - BoxWidth - PadRight - BoxHPad -
                                                                       levelTextSize.Width,
                                                                       e.Bounds.Top + PadTop,
                                                                       levelTextSize.Width + PadRight,
                                                                       levelTextSize.Height), Color.Black);

            TextRenderer.DrawText(g, pctText, m_skillsQueueFont, new Rectangle(
                                                                     e.Bounds.Right - BoxWidth - PadRight - BoxHPad -
                                                                     pctTextSize.Width,
                                                                     e.Bounds.Top + PadTop + levelTextSize.Height +
                                                                     LineVPad, pctTextSize.Width + PadRight,
                                                                     pctTextSize.Height), Color.Black);

            // Draw the queue color bar
            DrawQueueColorBar(g, e);
        }

        /// <summary>
        /// Draws the queue color bar.
        /// </summary>
        /// <param name="g">The graphic surface.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.DrawItemEventArgs"/> instance containing the event data.</param>
        private void DrawQueueColorBar(Graphics g, DrawItemEventArgs e)
        {
            // Draw skill queue color bar
            Brush brush = (Settings.UI.SafeForWork ? Brushes.DarkGray : Brushes.CornflowerBlue);
            var qBarRect = new Rectangle(e.Bounds.Left, GetItemHeight - LowerBoxHeight, e.Bounds.Width, LowerBoxHeight);
            g.FillRectangle(Brushes.DimGray, qBarRect);
            Rectangle skillRect = SkillQueueControl.GetSkillRect(m_item, qBarRect.Width, LowerBoxHeight - 1);
            g.FillRectangle(brush,
                            new Rectangle(skillRect.X, GetItemHeight - LowerBoxHeight, skillRect.Width, skillRect.Height));

            // If we have more than one skill level in queue, we need to redraw them only every (24h / width in pixels)
            if (e.Index == 1)
                m_nextRepainting = DateTime.Now.AddSeconds((24 * 3600) / Width);
        }

        #endregion

        #region Local events

        /// <summary>
        /// Handles the MouseWheel event of the lbSkillsQueue control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.MouseEventArgs"/> instance containing the event data.</param>
        private void lbSkillsQueue_MouseWheel(object sender, MouseEventArgs e)
        {
            // Update the drawing based upon the mouse wheel scrolling.
            int numberOfItemLinesToMove = e.Delta * SystemInformation.MouseWheelScrollLines / 120;
            int lines = numberOfItemLinesToMove;
            if (lines == 0)
                return;

            // Compute the number of lines to move
            int direction = lines/Math.Abs(lines);
            var numberOfPixelsToMove = new int[lines*direction];
            for (int i = 1; i <= Math.Abs(lines); i++)
            {
                object item = null;

                // Going up
                if (direction == Math.Abs(direction))
                {
                    // Retrieve the next top item
                    if (lbSkillsQueue.TopIndex - i >= 0)
                        item = lbSkillsQueue.Items[lbSkillsQueue.TopIndex - i];
                }
                    // Going down
                else
                {
                    // Compute the height of the items from current the topindex (included)
                    int height = 0;
                    for (int j = lbSkillsQueue.TopIndex + i - 1; j < lbSkillsQueue.Items.Count; j++)
                    {
                        height += GetItemHeight;
                    }

                    // Retrieve the next bottom item
                    if (height > lbSkillsQueue.ClientSize.Height)
                        item = lbSkillsQueue.Items[lbSkillsQueue.TopIndex + i - 1];
                }

                // If found a new item as top or bottom
                if (item != null)
                {
                    numberOfPixelsToMove[i - 1] = GetItemHeight * direction;
                }
                else
                {
                    lines -= direction;
                }
            }

            // Scroll 
            if (lines != 0)
                lbSkillsQueue.Invalidate();
        }

        /// <summary>
        /// On a mouse down event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbSkills_MouseDown(object sender, MouseEventArgs e)
        {
            // Retrieve the item at the given point and quit if none
            int index = lbSkillsQueue.IndexFromPoint(e.X, e.Y);
            if (index < 0 || index >= lbSkillsQueue.Items.Count)
                return;

            // Beware, this last index may actually means a click in the whitespace at the bottom
            // Let's deal with this special case
            if (index == lbSkillsQueue.Items.Count - 1)
            {
                Rectangle itemRect = lbSkillsQueue.GetItemRectangle(index);
                if (!itemRect.Contains(e.Location))
                    return;
            }

            // Right click for skills below lv5 : we display a context menu to plan higher levels.
            m_item = lbSkillsQueue.Items[index] as QueuedSkill;
            var skill = m_item.Skill;
            if (skill != null)
            {
                if (e.Button == MouseButtons.Right)
                {
                    // "Show in Skill Explorer" menu item
                    var tmSkillExplorer = new ToolStripMenuItem("Show In Skill Explorer", CommonProperties.Resources.LeadsTo);
                    tmSkillExplorer.Click += tmSkillExplorer_Click;
                    tmSkillExplorer.Tag = skill;

                    // Add to the context menu
                    contextMenuStripPlanPopup.Items.Clear();
                    contextMenuStripPlanPopup.Items.Add(tmSkillExplorer);

                    if (skill.Level < 5)
                    {
                        // Reset the menu.
                        var tm = new ToolStripMenuItem(String.Format(CultureConstants.DefaultCulture, "Add {0}", skill.Name));

                        // Build the level options.
                        int nextLevel = Math.Min(5, skill.Level + 1);
                        for (var level = nextLevel; level < 6; level++)
                        {
                            var menuLevel = new ToolStripMenuItem(String.Format(CultureConstants.DefaultCulture,
                                                                                "Level {0} to", Skill.GetRomanForInt(level)));
                            tm.DropDownItems.Add(menuLevel);

                            int level1 = level;
                            Character.Plans.AddTo(menuLevel.DropDownItems, (menuPlanItem, plan) =>
                                                    {
                                                        menuPlanItem.Click += menuPlanItem_Click;
                                                        menuPlanItem.Tag = new Pair<Plan, SkillLevel>(
                                                                                            plan, new SkillLevel(skill, level1));
                                                    });
                        }

                        // Add to the context menu
                        contextMenuStripPlanPopup.Items.Add(new ToolStripSeparator());
                        contextMenuStripPlanPopup.Items.Add(tm);
                    }

                    // Display the context menu
                    contextMenuStripPlanPopup.Show((Control) sender, new Point(e.X, e.Y));
                    return;
                }
            }

            // Non-right click or already lv5, display the tooltip
            DisplayTooltip(m_item);
        }

        /// <summary>
        /// On mouse move, we hide the tooltip.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbSkills_MouseMove(object sender, MouseEventArgs e)
        {
            for (int i = 0; i < lbSkillsQueue.Items.Count; i++)
            {
                // Skip until we found the mouse location
                Rectangle rect = lbSkillsQueue.GetItemRectangle(i);
                if (!rect.Contains(e.Location))
                    continue;

                // Updates the tooltip
                m_item = lbSkillsQueue.Items[i] as QueuedSkill;
                DisplayTooltip(m_item);
                return;
            }

            // If we went so far, we're not over anything.
            m_lastTooltipItem = null;
            ttToolTip.Active = false;
        }

        /// <summary>
        /// Displays the tooltip for the given skill.
        /// </summary>
        /// <param name="item"></param>
        private void DisplayTooltip(QueuedSkill item)
        {
            if (ttToolTip.Active && m_lastTooltipItem != null && m_lastTooltipItem == item)
                return;

            m_lastTooltipItem = item;

            ttToolTip.Active = false;
            ttToolTip.SetToolTip(lbSkillsQueue, GetTooltip(item));
            ttToolTip.Active = true;
        }

        /// <summary>
        /// Gets the tooltip text for the given skill
        /// </summary>
        /// <param name="skill"></param>
        private string GetTooltip(QueuedSkill skill)
        {
            if (skill.Skill == null)
                return String.Empty;

            int sp = skill.Skill.SkillPoints;
            int nextLevel = Math.Min(5, skill.Level);
            double percentCompleted = 0;
            if (skill.Level == skill.Skill.Level + 1)
                percentCompleted = skill.Skill.PercentCompleted;

            if (skill.Level > skill.Skill.Level + 1)
                sp = skill.CurrentSP;

            int nextLevelSP = skill.Skill.StaticData.GetPointsRequiredForLevel(nextLevel);
            int pointsLeft = nextLevelSP - sp;
            TimeSpan timeSpanFromPoints = skill.Skill.GetTimeSpanForPoints(pointsLeft);
            string remainingTimeText = timeSpanFromPoints.ToDescriptiveText(
                DescriptiveTextOptions.IncludeCommas | DescriptiveTextOptions.UppercaseText);

            if (sp < skill.Skill.StaticData.GetPointsRequiredForLevel(1))
            {
                // Training hasn't got past level 1 yet
                var untrainedToolTip = new StringBuilder();
                untrainedToolTip.AppendFormat(CultureConstants.DefaultCulture, "Not yet trained to Level I ({0}%)\n",
                                              Math.Floor(percentCompleted));
                untrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                              "Next level I: {0:#,##0} skill points remaining\n", pointsLeft);
                untrainedToolTip.AppendFormat(CultureConstants.DefaultCulture, "Training time remaining: {0}",
                                              remainingTimeText);
                AddSkillBoilerPlate(untrainedToolTip, skill.Skill);
                return untrainedToolTip.ToString();
            }

            // So, it's a left click on a skill, we display the tool tip
            // Currently training skill?
            if (skill.Skill.IsTraining && percentCompleted != 0)
            {
                var partiallyTrainedToolTip = new StringBuilder();
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture, "Partially Completed ({0}%)\n",
                                                     Math.Floor(percentCompleted));
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                                     "Training to level {0}: {1:#,##0} skill points remaining\n",
                                                     Skill.GetRomanForInt(nextLevel), pointsLeft);
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture, "Training time remaining: {0}",
                                                     remainingTimeText);
                AddSkillBoilerPlate(partiallyTrainedToolTip, skill.Skill);
                return partiallyTrainedToolTip.ToString();
            }

            // Currently training skill but next queued level?
            if (skill.Skill.IsTraining && percentCompleted == 0)
            {
                var partiallyTrainedToolTip = new StringBuilder();
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                                     "Previous level not yet completed\n");
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                                     "Queued to level {0}: {1:#,##0} skill points remaining\n",
                                                     Skill.GetRomanForInt(nextLevel), pointsLeft);
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture, "Training time to next level: {0}",
                                                     remainingTimeText);
                AddSkillBoilerPlate(partiallyTrainedToolTip, skill.Skill);
                return partiallyTrainedToolTip.ToString();
            }

            // Partially trained skill and not in training?
            if (skill.Skill.IsPartiallyTrained && !skill.Skill.IsTraining)
            {
                var partiallyTrainedToolTip = new StringBuilder();
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture, "Partially Completed ({0}%)\n",
                                                     Math.Floor(percentCompleted));
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                                     "Queued to level {0}: {1:#,##0} skill points remaining\n",
                                                     Skill.GetRomanForInt(nextLevel), pointsLeft);
                partiallyTrainedToolTip.AppendFormat(CultureConstants.DefaultCulture, "Training time remaining: {0}",
                                                     remainingTimeText);
                AddSkillBoilerPlate(partiallyTrainedToolTip, skill.Skill);
                return partiallyTrainedToolTip.ToString();
            }

            // We've completed all the skill points for the current level
            if (!skill.Skill.IsPartiallyTrained && skill.Level != 5)
            {
                var levelCompleteToolTip = new StringBuilder();
                levelCompleteToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                                  "Completed Level {0}: {1:#,##0}/{2:#,##0}\n",
                                                  Skill.GetRomanForInt(skill.Level - 1), sp, nextLevelSP);
                levelCompleteToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                                  "Queued level {0}: {1:#,##0} skill points required\n",
                                                  Skill.GetRomanForInt(nextLevel), pointsLeft);
                levelCompleteToolTip.AppendFormat(CultureConstants.DefaultCulture, "Training time to next level: {0}",
                                                  remainingTimeText);
                AddSkillBoilerPlate(levelCompleteToolTip, skill.Skill);
                return levelCompleteToolTip.ToString();
            }

            // Error in calculating SkillPoints
            var calculationErrorToolTip = new StringBuilder();
            calculationErrorToolTip.AppendLine("Partially Trained (Could not calculate all skill details)");
            calculationErrorToolTip.AppendFormat(CultureConstants.DefaultCulture,
                                                 "Next level {0}: {1:#,##0} skill points remaining\n", nextLevel,
                                                 pointsLeft);
            calculationErrorToolTip.AppendFormat(CultureConstants.DefaultCulture, "Training time remaining: {0}",
                                                 remainingTimeText);
            AddSkillBoilerPlate(calculationErrorToolTip, skill.Skill);
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
        /// Handler for a plan item click on the plan's context menu.
        /// Add a skill to the plan.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void menuPlanItem_Click(object sender, EventArgs e)
        {
            var planItem = (ToolStripMenuItem) sender;
            var tag = (Pair<Plan, SkillLevel>) planItem.Tag;

            IPlanOperation operation = tag.A.TryPlanTo(tag.B.Skill, tag.B.Level);
            PlanHelper.SelectPerform(operation);
        }

        /// <summary>
        /// Shows the selected skill in Skill Explorer
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tmSkillExplorer_Click(object sender, EventArgs e)
        {
            var item = (ToolStripMenuItem) sender;
            var skill = (Skill) item.Tag;

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
        private void EveClient_TimerTick(object sender, EventArgs e)
        {
            if (Character.IsTraining && Visible)
            {
                // Retrieves the trained skill for update but quit if the skill is null (was not in our datafiles)
                QueuedSkill trainingSkill = Character.CurrentlyTrainingSkill;
                if (trainingSkill == null)
                    return;

                // Invalidate the currently training skill level row
                int index = lbSkillsQueue.Items.IndexOf(trainingSkill);
                if (index == 0)
                    lbSkillsQueue.Invalidate(lbSkillsQueue.GetItemRectangle(index));

                // When there are more than one skill level rows in queue, we invalidate them on a timer
                if (lbSkillsQueue.Items.Count > 1 && DateTime.Now > m_nextRepainting)
                    lbSkillsQueue.Invalidate();

                UpdateContent();
            }
        }

        /// <summary>
        /// When the character changed, we refresh the content
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            if (e.Character != Character)
                return;

            UpdateContent();
        }

        /// <summary>
        /// When the queue changed, we refresh the content
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_QueuedSkillsCompleted(object sender, QueuedSkillsEventArgs e)
        {
            UpdateContent();
        }

        /// <summary>
        /// When the settings changed, we refresh the content
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            UpdateContent();
        }

        #endregion
    }
}