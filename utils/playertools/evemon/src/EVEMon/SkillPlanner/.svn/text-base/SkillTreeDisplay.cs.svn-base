using System;
using System.Linq;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using EVEMon.Common;
using System.Text;

namespace EVEMon.SkillPlanner
{

    #region SkillTreeDisplay
    /// <summary>
    /// Represents the diagram displayed on the skill browser.
    /// </summary>
    public partial class SkillTreeDisplay : UserControl
    {
        #region Row
        /// <summary>
        /// Represents a cells' row
        /// </summary>
        private sealed class Row : List<Cell>
        {
            private SkillTreeDisplay m_std = new SkillTreeDisplay();

            /// <summary>
            /// Constructor
            /// </summary>
            public Row(Cell cell)
            {
                Add(cell);
            }

            /// <summary>
            /// Gets the row's width
            /// </summary>
            public int Width
            {
                get { return Count * m_std.CellWidth + (Count - 1) * SkillboxMarginLr; }
            }

            /// <summary>
            /// Gets true if two cells overlaps or if their space is lesser than the regular spacing.
            /// </summary>
            /// <param name="leftIndex"></param>
            /// <param name="rightIndex"></param>
            /// <param name="space">The space between the two cells, including the margins. Negative when boxes are overlapping.</param>
            /// <returns></returns>
            public bool AreOverlapping(int leftIndex, int rightIndex, out int space)
            {
                space = 0;
                if (leftIndex == rightIndex || rightIndex >= Count || leftIndex >= Count || rightIndex < -1 || leftIndex < -1)
                    return false;

                var left = this[Math.Min(leftIndex, rightIndex)];
                var right = this[Math.Max(leftIndex, rightIndex)];

                space = right.Rectangle.Left - (left.Rectangle.Right + SkillboxMarginLr);
                return (space < 0);
            }
        }
        #endregion

        #region Cell
        /// <summary>
        /// Helper class to store layout informations about a skill
        /// </summary>
        private sealed class Cell
        {
            public Skill Skill;
            public List<Cell> Cells = new List<Cell>();
            public Rectangle Rectangle = Rectangle.Empty;
            public int RequiredLevel = -1;

            private SkillTreeDisplay m_std = new SkillTreeDisplay();

            /// <summary>
            /// Constructor for root
            /// </summary>
            /// <param name="skill"></param>
            public Cell(Skill skill)
            {
                Skill = skill;

                // Create the top row
                var rows = new List<Row>();
                rows.Add(new Row(this));

                // Create the children
                foreach (var prereq in skill.Prerequisites)
                {
                    // Ignore recursive prereqs, happens with non-public skills like Polaris
                    if (prereq.Skill == skill)
                        continue;

                    Cells.Add(new Cell(prereq, rows, 1));
                }

                // Perform the layout
                FirstPassLayout(0, 0);
                SecondPassLayout(rows, 0);
            }

            /// <summary>
            /// Constructor for prerequisites
            /// </summary>
            /// <param name="rows"></param>
            /// <param name="level"></param>
            /// <param name="prereq"></param>
            private Cell(SkillLevel prereq, List<Row> rows, int level)
            {
                Skill = prereq.Skill;
                RequiredLevel = prereq.Level;

                // Put on the appropriate row
                if (rows.Count == level)
                {
                    rows.Add(new Row(this));
                }
                else
                {
                    rows[level].Add(this);
                }

                // Create the children
                foreach (var childPrereq in prereq.Skill.Prerequisites)
                {
                    // Ignore recursive prereqs, happens with non-public skills like Polaris
                    if (childPrereq.Skill == prereq.Skill)
                        continue;

                    Cells.Add(new Cell(childPrereq, rows, level + 1));
                }
            }

            /// <summary>
            /// Arrange cells in a hierarchical order matching prerequisites, the first one being centered on x = 0.
            /// </summary>
            /// <param name="left"></param>
            /// <param name="top"></param>
            /// <returns></returns>
            private void FirstPassLayout(int left, int top)
            {
                // Layout this cell
                Rectangle = new Rectangle(left - m_std.CellWidth / 2, top, m_std.CellWidth, m_std.CellHeight);

                // Layout the children
                int childrenTop = top + m_std.CellHeight + SkillboxMarginUd;
                int childrenWidth = Cells.Count * m_std.CellWidth +(Cells.Count - 1) * SkillboxMarginLr;

                left += (m_std.CellWidth - childrenWidth) / 2;
                foreach (var cell in Cells)
                {
                    cell.FirstPassLayout(left, childrenTop);
                    left += m_std.CellWidth + SkillboxMarginLr;
                }
            }

            /// <summary>
            /// The first pass may have created overlapping rectangles, so we check every row and shift boxes when required.
            /// </summary>
            /// <param name="rows"></param>
            /// <param name="level"></param>
            private void SecondPassLayout(List<Row> rows, int level)
            {
                // Gets the row for this level
                if (level == rows.Count)
                    return;

                var row = rows[level];

                // Scan every cell and, when there is a conflict, shift all the other cells
                for (int i = 0; i < row.Count - 1; i++)
                {
                    int space;
                    if (!row.AreOverlapping(i, i + 1, out space))
                        continue;

                    // Shift the two cells
                    int shift = (-space) >> 1;
                    row[i].Offset(-shift, 0);
                    row[i + 1].Offset(shift, 0);

                    // Shift boxes on the left of "left"
                    for (int j = i - 1; j >= 0; j--)
                    {
                        if (!row.AreOverlapping(j, j + 1, out space))
                            break;

                        row[j].Offset(space, 0);
                    }
                    // Shift boxes on the right of "right"
                    for (int j = i + 2; j < row.Count; j++)
                    {
                        if (!row.AreOverlapping(j, j - 1, out space))
                            break;

                        row[j].Offset(-space, 0);
                    }
                }

                // Next level
                SecondPassLayout(rows, level + 1);
            }

            /// <summary>
            /// Offsets this cell and all its children.
            /// </summary>
            /// <param name="x"></param>
            /// <param name="y"></param>
            public void Offset(int x, int y)
            {
                Rectangle.Offset(x, y);
                foreach (var cell in Cells)
                {
                    cell.Offset(x, y);
                }
            }

            /// <summary>
            /// Arranges the whole graph into the given surface. Centers the cells and then return the global bounding box.
            /// </summary>
            /// <param name="size"></param>
            /// <returns>The graph bounds, centered into the given surface (or aligned on top left when there is overlap.</returns>
            public Rectangle Arrange(Size size)
            {
                // Compute the global rect
                int left = Rectangle.Left, right = Rectangle.Right, top = Rectangle.Top, bottom = Rectangle.Bottom;
                foreach (var cell in AllCells)
                {
                    var rect = cell.Rectangle;
                    left = Math.Min(left, rect.Left);
                    right = Math.Max(right, rect.Right);
                    bottom = Math.Max(bottom, rect.Bottom);
                    top = Math.Min(top, rect.Top);
                }

                // Compute the new origins
                int xOrigin = Math.Max(10, size.Width - (right - left)) >> 1;
                int yOrigin = Math.Max(10, size.Height - (bottom - top)) >> 1;

                // Offset the cell's rectangles
                foreach (var cell in AllCells)
                {
                    cell.Rectangle.Offset(xOrigin - left, yOrigin - top);
                }

                // Return the global rect
                return new Rectangle(xOrigin, yOrigin, 20 + right - left, 20 + bottom - top);
            }

            /// <summary>
            /// Gets this cell and all its descendants.
            /// </summary>
            public IEnumerable<Cell> AllCells
            {
                get
                {
                    yield return this;
                    foreach (var child in Cells)
                    {
                        foreach (var cell in child.AllCells)
                        {
                            yield return cell;
                        }
                    }
                }
            }
        }
        #endregion


        private const int SkillboxMarginUd = 20;
        private const int SkillboxMarginLr = 10;
        private const DescriptiveTextOptions TimeFormat = DescriptiveTextOptions.UppercaseText | DescriptiveTextOptions.IncludeCommas;

        public event SkillClickedHandler SkillClicked;

        private Plan m_plan;
        private Skill m_rootSkill;
        private Cell m_rootCell;
        private Rectangle m_graphBounds = new Rectangle(0, 0, 10, 10);


        /// <summary>
        /// Constructor
        /// </summary>
        public SkillTreeDisplay()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Subscribe events on load.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            SetStyle(ControlStyles.Opaque, true);
            SetStyle(ControlStyles.ResizeRedraw, true);

            EveClient.SettingsChanged += EveClient_SettingsChanged;
            EveClient.CharacterChanged += EveClient_CharacterChanged;
            EveClient.PlanChanged += EveClient_PlanChanged;
            Disposed += OnDisposed;
        }

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.SettingsChanged -= EveClient_SettingsChanged;
            EveClient.CharacterChanged -= EveClient_CharacterChanged;
            EveClient.PlanChanged -= EveClient_PlanChanged;
            Disposed -= OnDisposed;
        }

        /// <summary>
        /// Gets the cell width according to dpi (for font scaling support)
        /// </summary>
        public int CellWidth
        {
            get
            {
                Graphics g = Graphics.FromHwnd(Handle);
                float dpi = g.DpiX;

                if (dpi > 125)
                    return 353;

                if (dpi > 96)
                    return 295;

                return 235;
            }
        }

        /// <summary>
        /// Gets the cell height according to dpi (for font scaling support)
        /// </summary>
        public int CellHeight
        {
            get
            {
                Graphics g = Graphics.FromHwnd(Handle);
                float dpi = g.DpiX;

                if (dpi > 125)
                    return 110;

                if (dpi > 96)
                    return 92;

                return 73;
            }
        }
        
        /// <summary>
        /// Gets or sets the plan this control is bound to
        /// </summary>
        public Plan Plan
        {
            get { return m_plan; }
            set 
            { 
                m_plan = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Gets or sets the root skill.
        /// </summary>
        public Skill RootSkill
        {
            get { return m_rootSkill; }
            set
            {
                if (m_rootSkill != value)
                {
                    m_rootSkill = value;
                    UpdateLayout();
                }
            }
        }

        #region Layout and painting
        /// <summary>
        /// Checks training data
        /// </summary>
        private void CheckTraining()
        {
            if (m_rootSkill == null)
                return;

            tmrSkillTick.Enabled = m_rootSkill.AllPrerequisites.Any(x => x.Skill.IsTraining);
        }

        /// <summary>
        /// Build the layout
        /// </summary>
        private void UpdateLayout()
        {
            if (m_rootSkill == null)
                return;

            m_rootCell = new Cell(m_rootSkill);
            ArrangeGraph();
        }

        /// <summary>
        /// Arranges the graph position
        /// </summary>
        private void ArrangeGraph()
        {
            if (m_rootSkill == null)
                return;

            m_graphBounds = m_rootCell.Arrange(Size);
            AutoScrollMinSize = m_graphBounds.Size;
            Invalidate();
        }

        /// <summary>
        /// Performs the painting
        /// </summary>
        /// <param name="e"></param>
        protected override void OnPaint(PaintEventArgs e)
        {
            var g = e.Graphics;

            // Draws the background (solid or gradient, depending on safe mode or not)
            if (!Settings.UI.SafeForWork)
            {
                using (Brush b = new LinearGradientBrush(ClientRectangle, Color.LightBlue, Color.DarkBlue, 90.0F))
                {
                    g.FillRectangle(b, e.ClipRectangle);
                }
            }
            else
            {
                g.FillRectangle(SystemBrushes.Control, e.ClipRectangle);
            }

            // Returns when no root skill
            if (m_rootSkill == null)
                return;

            // Compute offset caused by scrollers
            int ofsLeft = AutoScrollPosition.X;
            int ofsTop = AutoScrollPosition.Y;

            // Draw the lines
            using(var linePen = new Pen((Settings.UI.SafeForWork ? SystemColors.ControlText : Color.White), 5.0F))
            {
                foreach(var cell in m_rootCell.Cells)
                {
                    DrawLines(g, m_rootCell, cell, linePen, ofsLeft, ofsTop);
                }
            }

            // Draw the cells
            using (Font boldFont = FontFactory.GetFont(Font, FontStyle.Bold))
            {
                foreach (var cell in m_rootCell.AllCells)
                {
                    DrawCell(g, cell, boldFont, ofsLeft, ofsTop);
                }
            }
        }

        /// <summary>
        /// Draws the line between the given cells and recursively repeats the operation for the children cell.
        /// </summary>
        /// <param name="g"></param>
        /// <param name="startCell"></param>
        /// <param name="endCell"></param>
        /// <param name="pen"></param>
        /// <param name="ofsLeft"></param>
        /// <param name="ofsTop"></param>
        private void DrawLines(Graphics g, Cell startCell, Cell endCell, Pen pen, int ofsLeft, int ofsTop)
        {
            var startRect = startCell.Rectangle;
            var endRect = endCell.Rectangle;

            g.DrawLine(pen,
                startRect.Location.X + ofsLeft + (startRect.Width >> 1), 
                startRect.Location.Y + ofsTop + (startRect.Height >> 1),
                endRect.Location.X + ofsLeft + (startRect.Width >> 1), 
                endRect.Location.Y + ofsTop + (startRect.Height >> 1));

            foreach (var child in endCell.Cells)
            {
                DrawLines(g, endCell, child, pen, ofsLeft, ofsTop);
            }
        }

        /// <summary>
        /// Paints the provided cell and recursively repeats the operation for the children cell.
        /// </summary>
        /// <param name="g"></param>
        /// <param name="cell"></param>
        /// <param name="boldFont"></param>
        /// <param name="ofsLeft"></param>
        /// <param name="ofsTop"></param>
        private void DrawCell(Graphics g, Cell cell, Font boldFont, int ofsLeft, int ofsTop)
        {
            Rectangle rect = cell.Rectangle;
            rect.Offset(ofsLeft, ofsTop);

            Color stdTextColor = !Settings.UI.SafeForWork ? Color.Black : SystemColors.ControlText;
            Color reqTextColor = !Settings.UI.SafeForWork ? Color.Red : SystemColors.GrayText;
            Color prTextColor = !Settings.UI.SafeForWork ? Color.Yellow : SystemColors.ControlText;

            Brush fillBrush = null;
            try
            {
                StringBuilder currentLevelText = new StringBuilder();
                
                // Retrieves the output of the second line : "Current Level : II (Planned to IV)"
                currentLevelText.AppendFormat(CultureConstants.DefaultCulture,
                    "Current Level: {0}",
                    Skill.GetRomanForInt(cell.Skill.Level));

                if (m_plan.GetPlannedLevel(cell.Skill) > 0)
                {
                    currentLevelText.AppendFormat(CultureConstants.DefaultCulture,
                        " (Planned To: {0})",
                        Skill.GetRomanForInt(m_plan.GetPlannedLevel(cell.Skill)));
                }

                // Retrieves the output and colors for the lower lines
                string thisRequiredTime = null;
                string requiredLevel = null;
                string prereqTime = null;
                if (cell.RequiredLevel > 0)
                {
                    // Third line : "Required Level : V"
                    requiredLevel = String.Format(CultureConstants.DefaultCulture, "Required Level: {0}",
                        Skill.GetRomanForInt(cell.RequiredLevel));

                    if (cell.RequiredLevel > cell.Skill.Level)
                    {
                        // Fourth line : "This Time : 9H, 26M, 42S"
                        TimeSpan ts = cell.Skill.GetLeftTrainingTimeToLevel(cell.RequiredLevel);
                        thisRequiredTime = String.Format(CultureConstants.DefaultCulture, "This Time: {0}",
                            ts.ToDescriptiveText(TimeFormat));
                        reqTextColor = !Settings.UI.SafeForWork ? Color.Yellow : SystemColors.GrayText;

                        if (cell.Skill.ArePrerequisitesMet)
                        {
                            fillBrush = new LinearGradientBrush(rect, Color.LightPink, Color.DarkRed, 90.0F);
                        }
                        else
                        {
                            fillBrush = new LinearGradientBrush(rect, Color.Red, Color.Black, 90.0F);
                            stdTextColor = !Settings.UI.SafeForWork ? Color.White : SystemColors.ControlText;
                        }
                    }
                    // Required level already met
                    else
                    {
                        reqTextColor = !Settings.UI.SafeForWork ? Color.Black : SystemColors.ControlText;
                        fillBrush = new LinearGradientBrush(rect, Color.LightSeaGreen, Color.DarkGreen, 90.0F);
                    }
                }
                // Skill at level 0, prerequisites met
                else if (cell.Skill.ArePrerequisitesMet)
                {
                    fillBrush = new LinearGradientBrush(rect, Color.LightBlue, Color.Blue, 90.0F);
                }
                // Skill unknown, not trainable
                else
                {
                    fillBrush = new LinearGradientBrush(rect, Color.Blue, Color.Black, 90.0F);
                    stdTextColor = !Settings.UI.SafeForWork ? Color.White : SystemColors.ControlText;
                }

                // Last line : prerequisites time
                if (!cell.Skill.ArePrerequisitesMet)
                {
                    TimeSpan pts = cell.Skill.Character.GetTrainingTimeToMultipleSkills(cell.Skill.Prerequisites);
                    prereqTime = String.Format(CultureConstants.DefaultCulture, "Prerequisite: {0}", pts.ToDescriptiveText(TimeFormat));
                }



                // Fill the background
                if (Settings.UI.SafeForWork)
                {
                    fillBrush.Dispose();
                    fillBrush = new SolidBrush(SystemColors.Control);
                }
                g.FillRectangle(fillBrush, rect);


                // Draw text (two to five lines)
                Point drawPoint = new Point(rect.Left + 5, rect.Top + 5);
                Size sz = MeasureAndDrawText(g, cell.Skill.Name, boldFont, drawPoint, stdTextColor);
                drawPoint.Y += sz.Height;

                sz = MeasureAndDrawText(g, currentLevelText.ToString(), Font, drawPoint, stdTextColor);
                drawPoint.Y += sz.Height;

                if (!String.IsNullOrEmpty(requiredLevel))
                {
                    sz = MeasureAndDrawText(g, requiredLevel, Font, drawPoint, reqTextColor);
                    drawPoint.Y += sz.Height;
                }
                if (!String.IsNullOrEmpty(thisRequiredTime))
                {
                    sz = MeasureAndDrawText(g, thisRequiredTime, Font, drawPoint, reqTextColor);
                    drawPoint.Y += sz.Height;
                }
                if (!String.IsNullOrEmpty(prereqTime))
                {
                    sz = MeasureAndDrawText(g, prereqTime, Font, drawPoint, prTextColor);
                    drawPoint.Y += sz.Height;
                }

                // Draw border
                g.DrawRectangle(!Settings.UI.SafeForWork ? Pens.Black : SystemPens.ControlDarkDark, rect);
            }
            finally
            {
                if (fillBrush != null)
                    fillBrush.Dispose();
            }
        }

        /// <summary>
        /// Simultaneously measures and renders the text.
        /// </summary>
        /// <param name="g"></param>
        /// <param name="text"></param>
        /// <param name="f"></param>
        /// <param name="p"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        private Size MeasureAndDrawText(Graphics g, string text, Font f, Point p, Color c)
        {
            Size res = TextRenderer.MeasureText(g, text, f);

            TextRenderer.DrawText(g, text, f,
                                  new Rectangle(p.X, p.Y, res.Width, res.Height), c, Color.Transparent,
                                  TextFormatFlags.Default);
            return res;
        }

        /// <summary>
        /// Once the size changed, we rearrange the graph and update scroll bars.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnSizeChanged(EventArgs e)
        {
            ArrangeGraph();
            base.OnSizeChanged(e);
        }
        #endregion


        #region Other events
        /// <summary>
        /// When the root skill or one of its prerequisites is in training , every 30s we invalidate the display.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tmrSkillTick_Tick(object sender, EventArgs e)
        {
            Invalidate();
            CheckTraining();
        }

        /// <summary>
        /// On click, we detect which skill is under the mouse location
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseClick(MouseEventArgs e)
        {
            base.OnMouseClick(e);

            // Computes the offsets caused by scrollers
            int ofsLeft = -AutoScrollPosition.X;
            int ofsTop = -AutoScrollPosition.Y;

            // Checks every cell
            Skill skill = null;
            var mouseLocation = e.Location;
            mouseLocation.Offset(ofsLeft, ofsTop);
            foreach (var cell in m_rootCell.AllCells.Where(x => x.Rectangle.Contains(mouseLocation)))
            {
                skill = cell.Skill;
            }

            // Fires the event when skill not null
            if (skill == null)
                return;

            if (SkillClicked != null)
            {
                SkillClickedEventArgs se = new SkillClickedEventArgs(skill, e.Button, mouseLocation);
                SkillClicked(this, se);
            }
        }

        /// <summary>
        /// On scrolling, we invalidate the display
        /// </summary>
        /// <param name="se"></param>
        protected override void OnScroll(ScrollEventArgs se)
        {
            base.OnScroll(se);
            Invalidate();
        }

        /// <summary>
        /// On mouse wheel, the scroll changes, so we invalidate
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseWheel(MouseEventArgs e)
        {
            base.OnMouseWheel(e);
            Invalidate();
        }

        /// <summary>
        /// On settings change, we invalidate the drawing
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            Invalidate();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            if (m_plan == null) 
                return;

            if (e.Character != m_plan.Character) 
                return;

            Invalidate();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_PlanChanged(object sender, PlanChangedEventArgs e)
        {
            Invalidate();
        }
        #endregion
    }
    #endregion


    #region SkillClickedHandler
    /// <summary>
    /// Handler for the <see cref="SkillTreeDisplay.SkillClicked"/> event.
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>
    public delegate void SkillClickedHandler(object sender, SkillClickedEventArgs e);

    /// <summary>
    /// Arguments for the skill clicked event args
    /// </summary>
    public class SkillClickedEventArgs : EventArgs
    {
        private Skill m_skill;
        private MouseButtons m_button;
        private Point m_location;

        internal SkillClickedEventArgs(Skill skill, MouseButtons button, Point location)
        {
            m_skill = skill;
            m_button = button;
            m_location = location;
        }

        public Skill Skill
        {
            get { return m_skill; }
        }

        public MouseButtons Button
        {
            get { return m_button; }
        }

        public Point Location
        {
            get { return m_location; }
        }

        public int X
        {
            get { return m_location.X; }
        }

        public int Y
        {
            get { return m_location.Y; }
        }
    }
    #endregion

}