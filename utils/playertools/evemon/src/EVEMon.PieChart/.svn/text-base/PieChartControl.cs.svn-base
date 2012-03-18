using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;

namespace EVEMon.PieChart
{
    /// <summary>
    /// Summary description for PieChartControl.
    /// </summary>
    public class PieChartControl : System.Windows.Forms.Panel
    {
        /// <summary>
        ///   Initializes the <c>PieChartControl</c>.
        /// </summary>
        public PieChartControl()
            : base()
        {
            this.SetStyle(ControlStyles.UserPaint, true);
            this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            this.SetStyle(ControlStyles.DoubleBuffer, true);
            this.SetStyle(ControlStyles.ResizeRedraw, true);
            m_toolTip = new ToolTip();
        }

        public PieChart3D PieChart
        {
            get
            {
                return m_pieChart;
            }
        }

        /// <summary>
        ///   Sets the left margin for the chart.
        /// </summary>
        public float LeftMargin
        {
            set
            {
                Debug.Assert(value >= 0);
                m_leftMargin = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Sets the right margin for the chart.
        /// </summary>
        public float RightMargin
        {
            set
            {
                Debug.Assert(value >= 0);
                m_rightMargin = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Sets the top margin for the chart.
        /// </summary>
        public float TopMargin
        {
            set
            {
                Debug.Assert(value >= 0);
                m_topMargin = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Sets the bottom margin for the chart.
        /// </summary>
        public float BottomMargin
        {
            set
            {
                Debug.Assert(value >= 0);
                m_bottomMargin = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Sets the indicator if chart should fit the bounding rectangle
        ///   exactly.
        /// </summary>
        public bool FitChart
        {
            set
            {
                m_fitChart = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Sets values to be represented by the chart.
        /// </summary>
        public decimal[] Values
        {
            set
            {
                m_values = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Sets colors to be used for rendering pie slices.
        /// </summary>
        public Color[] Colors
        {
            set
            {
                m_colors = value;
                Invalidate();
            }
            get
            {
                return m_colors;
            }
        }

        /// <summary>
        ///   Sets values for slice displacements.
        /// </summary>
        public float[] SliceRelativeDisplacements
        {
            set
            {
                m_relativeSliceDisplacements = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Gets or sets tooltip texts.
        /// </summary>
        public string[] ToolTips
        {
            set { m_toolTipTexts = value; }
            get { return m_toolTipTexts; }
        }

        /// <summary>
        ///   Sets texts appearing by each pie slice.
        /// </summary>
        public string[] Texts
        {
            set { m_texts = value; }
        }

        /// <summary>
        ///   Sets pie slice reative height.
        /// </summary>
        public float SliceRelativeHeight
        {
            set
            {
                m_sliceRelativeHeight = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Sets the shadow style.
        /// </summary>
        public ShadowStyle ShadowStyle
        {
            set
            {
                m_shadowStyle = value;
                Invalidate();
            }
        }

        /// <summary>
        ///  Sets the edge color type.
        /// </summary>
        public EdgeColorType EdgeColorType
        {
            set
            {
                m_edgeColorType = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Sets the edge lines width.
        /// </summary>
        public float EdgeLineWidth
        {
            set
            {
                m_edgeLineWidth = value;
                Invalidate();
            }
        }

        /// <summary>
        ///   Sets the initial angle from which pies are drawn.
        /// </summary>
        public float InitialAngle
        {
            set
            {
                float newAngle = value;

                if(newAngle > 360.0f)
                    newAngle -= 360.0f;
                if(newAngle < 0.0f)
                    newAngle += 360.0f;

                OnAngleChange(new AngleChangeEventArgs(m_initialAngle, newAngle));

                m_initialAngle = newAngle;
                Invalidate();
            }
        }

        protected bool mouseDown = false;

        /// <summary>
        ///   Handles <c>OnPaint</c> event.
        /// </summary>
        /// <param name="args">
        ///   <c>PaintEventArgs</c> object.
        /// </param>
        protected override void OnPaint(PaintEventArgs args)
        {
            base.OnPaint(args);
            if (HasAnyValue)
            {
                DoDraw(args.Graphics);
            }
        }

        /// <summary>
        ///   Sets values for the chart and draws them.
        /// </summary>
        /// <param name="graphics">
        ///   Graphics object used for drawing.
        /// </param>
        protected void DoDraw(Graphics graphics)
        {
            if (m_drawValues != null && m_drawValues.Length > 0)
            {
                graphics.SmoothingMode = SmoothingMode.AntiAlias;
                float width = ClientSize.Width - m_leftMargin - m_rightMargin;
                float height = ClientSize.Height - m_topMargin - m_bottomMargin;
                // if the width or height if <=0 an exception would be thrown -> exit method..
                if (width <= 0 || height <= 0)
                    return;
                if (m_pieChart != null)
                    m_pieChart.Dispose();
                if (m_drawColors != null && m_drawColors.Length > 0)
                    m_pieChart = new PieChart3D(m_leftMargin, m_topMargin, width, height, m_drawValues, m_drawColors, m_sliceRelativeHeight, m_drawTexts);
                else
                    m_pieChart = new PieChart3D(m_leftMargin, m_topMargin, width, height, m_drawValues, m_sliceRelativeHeight, m_drawTexts);
                m_pieChart.FitToBoundingRectangle = m_fitChart;
                m_pieChart.InitialAngle = m_initialAngle;
                m_pieChart.SliceRelativeDisplacements = m_drawRelativeSliceDisplacements;
                m_pieChart.EdgeColorType = m_edgeColorType;
                m_pieChart.EdgeLineWidth = m_edgeLineWidth;
                m_pieChart.ShadowStyle = m_shadowStyle;
                m_pieChart.HighlightedIndex = m_highlightedIndex;
                m_pieChart.Draw(graphics);
                m_pieChart.Font = this.Font;
                m_pieChart.ForeColor = this.ForeColor;
                m_pieChart.PlaceTexts(graphics);
            }
        }

        /// <summary>
        ///   Handles <c>MouseEnter</c> event to activate the tooltip.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseEnter(System.EventArgs e)
        {
            base.OnMouseEnter(e);
            m_defaultToolTipAutoPopDelay = m_toolTip.AutoPopDelay;
            m_toolTip.AutoPopDelay = Int16.MaxValue;
        }

        /// <summary>
        ///   Handles <c>MouseLeave</c> event to disable tooltip.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseLeave(System.EventArgs e)
        {
            base.OnMouseLeave(e);
            m_toolTip.RemoveAll();
            m_toolTip.AutoPopDelay = m_defaultToolTipAutoPopDelay;
            m_highlightedIndex = -1;
            Refresh();
        }

        /// <summary>
        /// Handles <c>MouseDown</c> event
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseDown(e);

            this.mouseDown = true;
        }

        /// <summary>
        /// Handles <c>MouseUp</c> event
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseUp(MouseEventArgs e)
        {
            base.OnMouseUp(e);

            this.mouseDown = false;
        }


        /// <summary>
        ///   Handles <c>MouseMove</c> event to display tooltip for the pie
        ///   slice under pointer and to display slice in highlighted color.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseMove(System.Windows.Forms.MouseEventArgs e)
        {
            base.OnMouseMove(e);
            if (m_pieChart != null && m_values != null && m_values.Length > 0)
            {
                if (e.X == m_lastX && e.Y == m_lastY) return;

                if (this.mouseDown)
                {
                    this.InitialAngle = m_initialAngle - (e.X - m_lastX);
                }
                else
                {
                    int index = m_pieChart.FindPieSliceUnderPoint(new PointF(e.X, e.Y));

                    if (index != m_highlightedIndex)
                    {
                        m_highlightedIndex = index;
                        Refresh();
                    }
                    if (m_highlightedIndex != -1)
                    {
                        if (m_drawToolTipTexts == null || m_drawToolTipTexts.Length <= m_highlightedIndex || m_drawToolTipTexts[m_highlightedIndex].Length == 0)
                        {
                            m_toolTip.SetToolTip(this, m_values[m_highlightedIndex].ToString());
                        }
                        else
                        {
                            m_toolTip.SetToolTip(this, m_drawToolTipTexts[m_highlightedIndex]);
                        }
                    }
                    else
                    {
                        m_toolTip.RemoveAll();
                    }
                }

                m_lastX = e.X;
                m_lastY = e.Y;
            }
        }

        /// <summary>
        ///   Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (!m_disposed)
            {
                try
                {
                    if (disposing)
                    {
                        m_pieChart.Dispose();
                        m_toolTip.Dispose();
                    }
                    m_disposed = true;
                }
                finally
                {
                    base.Dispose(disposing);
                }
            }
        }

        /// <summary>
        ///   Gets a flag indicating if at least one value is nonzero.
        /// </summary>
        private bool HasAnyValue
        {
            get
            {
                if (m_values == null)
                    return false;
                foreach (decimal angle in m_values)
                {
                    if (angle != 0)
                    {
                        return true;
                    }
                }
                return false;
            }
        }

        public int GetIndex(int sliceIndex)
        {
            return this.m_sortOrder[sliceIndex];
        }

        /// <summary>
        /// Event for when the graph angle changes
        /// </summary>
        public event EventHandler AngleChange;

        /// <summary>
        /// Event for when the graph angle changes
        /// </summary>
        /// <param name="e"></param>
        protected virtual void OnAngleChange(AngleChangeEventArgs e)
        {
            if(AngleChange != null)
                AngleChange(this, e);
        }

        /// <summary>
        /// Will copy the original data to the vars used for drawing
        /// The original is needed to use for ordering
        /// </summary>
        public void CopyDataToDrawVars()
        {
            this.m_drawValues = (decimal[])this.m_values.Clone();
            this.m_drawColors = (Color[])this.m_colors.Clone();
            this.m_drawRelativeSliceDisplacements = (float[])this.m_relativeSliceDisplacements.Clone();
            this.m_drawToolTipTexts = (string[])this.m_toolTipTexts.Clone();
            this.m_drawTexts = (string[])this.m_texts.Clone();


            // fill the sort order to default:
            this.m_sortOrder = new int[this.m_values.Length];
            for (int i = 0; i < this.m_values.Length; i++)
            {
                this.m_sortOrder[i] = i;
            }
        }

        public void OrderSlices(bool orderBySize)
        {
            if (orderBySize == true && this.m_values != null)
            {
                // prefill the draw vars
                this.CopyDataToDrawVars();

                // take a copy of the original values
                // then use it to do the calculations
                decimal[] values = (decimal[])this.m_values.Clone();
                Color[] colours = (Color[])this.m_colors.Clone();
                float[] displacements = (float[])this.m_relativeSliceDisplacements.Clone();
                string[] tooltips = (string[])this.m_toolTipTexts.Clone();
                string[] texts = (string[])this.m_texts.Clone();

                // reordering the slices
                for (int num = 0; num < values.Length; num++)
                {
                    decimal tempsp = decimal.MinValue;
                    int biggest = -1;
                    for (int y = 0; y < values.Length; y++)
                    {
                        if (biggest == -1)
                        {
                            tempsp = values[y];
                            biggest = y;
                        }
                        if (values[y] > tempsp && values[y] > 0)
                        {
                            tempsp = values[y];
                            biggest = y;
                        }

                    }

                    this.m_drawValues[num] = values[biggest];
                    this.m_drawTexts[num] = texts[biggest];
                    this.m_drawRelativeSliceDisplacements[num] = displacements[biggest];
                    this.m_drawToolTipTexts[num] = tooltips[biggest];
                    this.m_drawColors[num] = colours[biggest];
                    this.m_sortOrder[num] = biggest;
                    values[biggest] = 0;
                }
            }
            else
            {
                if (this.m_values != null)
                {
                    this.CopyDataToDrawVars();
                }
            }

            this.Refresh();
        }

        private PieChart3D m_pieChart = null;
        private float m_leftMargin;
        private float m_topMargin;
        private float m_rightMargin;
        private float m_bottomMargin;
        private bool m_fitChart = false;

        private decimal[] m_values = null;
        private Color[] m_colors = null;
        private float m_sliceRelativeHeight;
        private float[] m_relativeSliceDisplacements = new float[] { 0F };
        private string[] m_texts = null;
        private string[] m_toolTipTexts = null;
        private ShadowStyle m_shadowStyle = ShadowStyle.GradualShadow;
        private EdgeColorType m_edgeColorType = EdgeColorType.SystemColor;
        private float m_edgeLineWidth = 1F;
        private float m_initialAngle;
        private int m_highlightedIndex = -1;
        private ToolTip m_toolTip = null;

        private int m_lastX = -1;
        private int m_lastY = -1;

        // These are used for the actual drawing. They are modified depending
        // on wether sorting by size is on or off
        private decimal[] m_drawValues = null;
        private Color[] m_drawColors = null;
        private float[] m_drawRelativeSliceDisplacements = new float[] { 0F };
        private string[] m_drawToolTipTexts = null;
        private string[] m_drawTexts = null;
        private int[] m_sortOrder = null;

        /// <summary>
        ///   Default AutoPopDelay of the ToolTip control.
        /// </summary>
        private int m_defaultToolTipAutoPopDelay;
        /// <summary>
        ///   Flag indicating that object has been disposed.
        /// </summary>
        private bool m_disposed = false;
    }

    public class AngleChangeEventArgs : EventArgs
    {
        private float oldValue, newValue;

        public AngleChangeEventArgs(float oldValue, float newValue)
        {
            this.oldValue = oldValue;
            this.newValue = newValue;
        }

        public float NewAngle
        {
            get { return newValue; }
        }

        public float OldAngle
        {
            get { return oldValue; }
        }
    }
}

