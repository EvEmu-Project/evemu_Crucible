using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.PieChart;
using System.Collections;
using EVEMon.Common.Controls;
using EVEMon.Common.Serialization;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common.Data;
using System.Globalization;

namespace EVEMon
{
    public partial class SkillsPieChart : EVEMonForm
    {
        private readonly Character m_character = null;

        #region Construction, loading, closing
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="character"></param>
        public SkillsPieChart(Character character)
        {
            InitializeComponent();
            this.RememberPositionKey = "SkillsPieChart";

            // Fields
            this.m_character = character;
            this.Text = "Skillgroup chart for " + character.Name;

            // Layout
            skillPieChartControl.LeftMargin = 20F;
            skillPieChartControl.TopMargin = 15F;
            skillPieChartControl.RightMargin = 20F;
            skillPieChartControl.BottomMargin = 15F;
            skillPieChartControl.FitChart = true;
            skillPieChartControl.SliceRelativeHeight = Settings.UI.SkillPieChart.SliceHeight;
            skillPieChartControl.InitialAngle = Settings.UI.SkillPieChart.InitialAngle;
            skillPieChartControl.ShadowStyle = ShadowStyle.GradualShadow;
            skillPieChartControl.EdgeColorType = EVEMon.PieChart.EdgeColorType.DarkerThanSurface;
            skillPieChartControl.EdgeLineWidth = 1F;
        }

        /// <summary>
        /// On loading
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            if (this.DesignMode) return;

            // Events
            skillPieChartControl.AngleChange += new EventHandler(skillPieChartControl_AngleChange);

            // Read settings
            sortBySizeCheck.Checked = Settings.UI.SkillPieChart.SortBySize;
            mergeMinorCheck.Checked = Settings.UI.SkillPieChart.MergeMinorGroups;
            pieHeight.Value = (decimal)Settings.UI.SkillPieChart.SliceHeight;
            pieAngle.Value = (decimal)Settings.UI.SkillPieChart.InitialAngle;

            // Check there are enough colors or create them
            if (Settings.UI.SkillPieChart.Colors.Count < m_character.SkillGroups.Count)
            {
                int alpha = 125;
                List<Color> newColors = new List<Color>();
                while (newColors.Count < m_character.SkillGroups.Count)
                {
                    newColors.Add(Color.FromArgb(alpha, Color.Red));
                    newColors.Add(Color.FromArgb(alpha, Color.Green));
                    newColors.Add(Color.FromArgb(alpha, Color.Blue));
                }
                skillPieChartControl.Colors = newColors.ToArray();
            }
            else
            {
                List<Color> loadedColors = new List<Color>();
                foreach (SerializableColor c in Settings.UI.SkillPieChart.Colors)
                {
                    loadedColors.Add((Color)c);
                }
                skillPieChartControl.Colors = loadedColors.ToArray();
            }

            // Initialize plans combox Box                        
            planSelector.SelectedIndex = 0;
            foreach (var plan in m_character.Plans)
            {
                planSelector.Items.Add(plan.Name);
            }

            // Update the display
            UpdatePieChart();
        }

        /// <summary>
        /// On closing, store settings and unsubscribe events
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);

            // Store colors to settings
            Settings.UI.SkillPieChart.Colors.Clear();
            foreach (Color c in skillPieChartControl.Colors)
            {
                Settings.UI.SkillPieChart.Colors.Add((SerializableColor)c);
            }

            // Store other settings
            Settings.UI.SkillPieChart.SliceHeight = (float)pieHeight.Value;
            Settings.UI.SkillPieChart.InitialAngle = (float)pieAngle.Value;
            Settings.UI.SkillPieChart.MergeMinorGroups = mergeMinorCheck.Checked;
            Settings.UI.SkillPieChart.SortBySize = sortBySizeCheck.Checked;

            // Events
            skillPieChartControl.AngleChange -= new EventHandler(skillPieChartControl_AngleChange);
        }
        #endregion


        #region Fetching data to the pie chart
        /// <summary>
        /// Updates the pie chart display
        /// </summary>
        private void UpdatePieChart()
        {
            // Prevents updating before OnLoad() completed.
            if (planSelector.SelectedIndex < 0) return;

            int groupCount = StaticSkills.AllGroups.Count();
            CharacterScratchpad scratchpad = new CharacterScratchpad(m_character);

            // Retrieve the selected Plan
            Plan plan = null;
            if (planSelector.SelectedIndex > 0)
            {
                plan = m_character.Plans[planSelector.SelectedIndex - 1];

                // Updates the scratchpad
                foreach (var entry in plan) scratchpad.Train(entry);
            }

            // Get group to index map and groups list
            List<SkillGroup> groups = new List<SkillGroup>();
            Dictionary<StaticSkillGroup, int> indices = new Dictionary<StaticSkillGroup, int>();
            foreach (var group in StaticSkills.AllGroups) 
            {
                indices[group] = groups.Count;
                groups.Add(m_character.SkillGroups[group.Name]);
            }

            // Get start SP, before plan
            decimal[] srcSkillPoints = new decimal[groupCount];
            foreach(var skillGroup in groups)
            {
                int groupIndex = indices[skillGroup.StaticData];
                srcSkillPoints[groupIndex] = skillGroup.TotalSP;
            }

            // Get target SP and skills count, after plan
            int[] skillCounts = new int[groupCount];
            decimal[] targetSkillPoints = new decimal[groupCount];
            foreach (StaticSkill skill in StaticSkills.AllSkills)
            {
                int sp = scratchpad.GetSkillPoints(skill);
                int groupIndex = indices[skill.Group];

                targetSkillPoints[groupIndex] += sp;
                if (sp != 0) skillCounts[groupIndex]++;
            }

            // Get groups names and descriptions
            string[] names = new string[groupCount];
            string[] descriptions = new string[groupCount];
            for(int i=0; i<srcSkillPoints.Length; i++)
            {
                names[i] = groups[i].Name;
                descriptions[i] = groups[i].Name;

                decimal srcSP = srcSkillPoints[i];
                decimal destSP = targetSkillPoints[i];

                StringBuilder description = new StringBuilder();
                description.AppendFormat(CultureConstants.DefaultCulture, "{0} ({1} skills, {2:#,###} skillpoints", names[i], skillCounts[i], srcSP);
                if (srcSP != destSP)
                {
                    description.AppendFormat(CultureConstants.DefaultCulture, " / {0:#,###} after plan completion", destSP);
                }
                description.Append(")");
                descriptions[i] = description.ToString();
            }

            // Merge minor groups
            if (mergeMinorCheck.Checked)
            {
                Merge(ref targetSkillPoints, ref names, ref descriptions);
            }

            // Compute the slices displacements
            int tinyGroups = 0;
            float[] slicesDiscplacements = new float[targetSkillPoints.Length];
            for(int i=0; i < targetSkillPoints.Length; i++)
            {
                slicesDiscplacements[i] = (targetSkillPoints[i] < 100000) ? 0.06F + (0.008F * ++tinyGroups) : 0.05F;  
            }

            // Assign and sort
            skillPieChartControl.Values = targetSkillPoints;
            skillPieChartControl.Texts = names;
            skillPieChartControl.ToolTips = descriptions;
            skillPieChartControl.SliceRelativeDisplacements = slicesDiscplacements;
            skillPieChartControl.OrderSlices(sortBySizeCheck.Checked);
        }

        /// <summary>
        /// Performs the merge
        /// </summary>
        /// <param name="targetSkillPoints"></param>
        /// <param name="names"></param>
        /// <param name="descriptions"></param>
        private static void Merge(ref decimal[] targetSkillPoints, ref string[] names, ref string[] descriptions)
        {
            // Gets total SP and threshold (1% of total SP)
            decimal totalSP = 0;
            for (int i = 0; i < targetSkillPoints.Length; i++) totalSP += targetSkillPoints[i];
            decimal threshold = totalSP / 100;

            // Gathers group indices to merge
            var mergedGroupIndices = new List<int>();
            for (int i = 0; i < targetSkillPoints.Length; i++)
            {
                if (targetSkillPoints[i] < threshold) mergedGroupIndices.Add(i);
            }

            // Prepare the merging lists
            var newTargetSkillPoints = new List<decimal>();
            var newDescriptions = new List<string>();
            var newNames = new List<string>();

            if (mergedGroupIndices.Count != 0)
            {
                newTargetSkillPoints.Add(0);
                newDescriptions.Add("");
                newNames.Add("Other");
            }

            // Merge
            bool isFirstMerged = true;
            for (int i = 0; i < targetSkillPoints.Length; i++)
            {
                // Is Merged ?
                if (mergedGroupIndices.Contains(i))
                {
                    if (!isFirstMerged) newDescriptions[0] += "\n";
                    isFirstMerged = false;

                    newTargetSkillPoints[0] += targetSkillPoints[i];
                    newDescriptions[0] += descriptions[i];
                }
                // Not merged
                else
                {
                    newTargetSkillPoints.Add(targetSkillPoints[i]);
                    newDescriptions.Add(descriptions[i]);
                    newNames.Add(names[i]);
                }
            }

            // Replace the old arrays
            targetSkillPoints = newTargetSkillPoints.ToArray();
            descriptions = newDescriptions.ToArray();
            names = newNames.ToArray();
        }
        #endregion


        #region Events for the controls
        /// <summary>
        /// When the user rotates the pie chart, we update the numeric box
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void skillPieChartControl_AngleChange(object sender, EventArgs e)
        {
            pieAngle.Value = (decimal) (e as AngleChangeEventArgs).NewAngle;
        }

        /// <summary>
        /// When the user changes the numeric box for the pie height, we update the pie chart control.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void pieHeight_ValueChanged(object sender, EventArgs e)
        {
            skillPieChartControl.SliceRelativeHeight = (float)pieHeight.Value;
        }

        /// <summary>
        /// When the user changes the numeric box for the angle, we update the pie chart control.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void pieAngle_ValueChanged(object sender, EventArgs e)
        {
            skillPieChartControl.InitialAngle = (float)pieAngle.Value;
        }

        /// <summary>
        /// Close button click. Close.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void closeButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// When the user double click the control, we allow him to change the clicked group's color
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void skillPieChartControl_DoubleClick(object sender, EventArgs e)
        {
            // Retrieve the clicked segment
            MouseEventArgs ev = (MouseEventArgs)e;
            PieChart3D m_pieChart = skillPieChartControl.PieChart;
            int index = m_pieChart.FindPieSliceUnderPoint(new PointF(ev.X, ev.Y));

            // If none clicked, we return. Otherwise we open the color picker.
            if (index == -1) return;
            if (m_colorDialog.ShowDialog() != DialogResult.OK) return;

            // The user picked a new color, we update our colors list.
            if (sortBySizeCheck.Checked)
            {
                int realIndex = skillPieChartControl.GetIndex(index);
                skillPieChartControl.Colors[realIndex] = Color.FromArgb(125, m_colorDialog.Color);
            }
            else
            {
                skillPieChartControl.Colors[index] = Color.FromArgb(125, m_colorDialog.Color);
            }

            // Forces an update of the control
            skillPieChartControl.OrderSlices(sortBySizeCheck.Checked);
        }

        /// <summary>
        /// Save button. Export to PNG.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void saveButton_Click(object sender, EventArgs e)
        {
            Bitmap pie = new Bitmap(skillPieChartControl.Width, skillPieChartControl.Height);
            Rectangle bounds = new Rectangle(0, 0, skillPieChartControl.Width, skillPieChartControl.Height);
            skillPieChartControl.DrawToBitmap(pie, bounds);

            DialogResult savePieResult = savePieDialog.ShowDialog();
            if (savePieResult == DialogResult.OK)
            {
                pie.Save(savePieDialog.FileName, System.Drawing.Imaging.ImageFormat.Png);
            }
        }

        /// <summary>
        /// Forces an update when the sort by size combo box state changes.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void sortBySizeCheck_CheckedChanged(object sender, EventArgs e)
        {
            UpdatePieChart();
        }

        /// <summary>
        /// When the selected plan change, we refresh all the data.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void planSelector_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdatePieChart();
        }

        /// <summary>
        /// When the merge minor option changes, we refresh all the data.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mergeMinorCheck_CheckedChanged(object sender, EventArgs e)
        {
            UpdatePieChart();
        }
        #endregion
    }
}
