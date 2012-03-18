using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Represents the method that will handle changing of the attribute's value.
    /// </summary>
    /// <param name="sender">Source control</param>
    /// <param name="deltaValue">Change of the value. Can be adjusted in a handler.</param>
    public delegate void ValueChangingHandler(AttributeBarControl sender, ref int deltaValue);

    /// <summary>
    /// Represents the method that will handle change of the attribute value.
    /// </summary>
    /// <param name="sender">Source control</param>
    public delegate void ValueChangedHandler(AttributeBarControl sender);

    /// <summary>
    /// Represents the method that will handle highlighting of a cell.
    /// </summary>
    /// <param name="sender">Source control</param>
    /// <param name="highlightValue">Cell index</param>
    public delegate void HighlightingHandler(AttributeBarControl sender, ref int highlightValue);

    /// <summary>
    /// This control shows the value of an attribute in form of cells.
    /// Also it allows to change value by clicking on a cell.
    /// </summary>
    public partial class AttributeBarControl : Control
    {
        private Pen m_borderPen = Pens.Black;

        /// <summary>
        /// Gets or sets the color of the border between cells.
        /// </summary>
        [Category("Appearance")]
        [DefaultValue(typeof(Color), "Black")]
        public Color BorderColor
        {
            get { return m_borderPen.Color; }
            set
            {
                m_borderPen = new Pen(value);
                Invalidate();
            }
        }

        private Pen m_outerBorderPen = Pens.LightGray;

        /// <summary>
        /// Gets or sets the outer border color.
        /// </summary>
        [Category("Appearance")]
        [DefaultValue(typeof(Color), "LightGray")]
        public Color OuterBorderColor
        {
            get { return m_outerBorderPen.Color; }
            set
            {
                m_outerBorderPen = new Pen(value);
                Invalidate();
            }
        }

        private SolidBrush m_inactiveBrush = new SolidBrush(Color.DimGray);

        /// <summary>
        /// Gets or sets the color of an inactive cell.
        /// </summary>
        [Category("Appearance")]
        [DefaultValue(typeof(Color), "DimGray")]
        public Color InactiveColor
        {
            get { return m_inactiveBrush.Color; }
            set
            {
                m_inactiveBrush = new SolidBrush(value);
                Invalidate();
            }
        }

        private SolidBrush m_basePointBrush = new SolidBrush(Color.LightGray);

        /// <summary>
        /// Gets or sets the color of a cell for base point.
        /// </summary>
        [Category("Appearance")]
        [DefaultValue(typeof(Color), "LightGray")]
        public Color BasePointColor
        {
            get { return m_basePointBrush.Color; }
            set
            {
                m_basePointBrush = new SolidBrush(value);
                Invalidate();
            }
        }

        private SolidBrush m_spentPointBrush = new SolidBrush(Color.LimeGreen);

        /// <summary>
        /// Gets or sets the color of a cell for spent point.
        /// </summary>
        [Category("Appearance")]
        [DefaultValue(typeof(Color), "LimeGreen")]
        public Color SpentPointColor
        {
            get { return m_spentPointBrush.Color; }
            set
            {
                m_spentPointBrush = new SolidBrush(value);
                Invalidate();
            }
        }

        int m_points = 5;

        /// <summary>
        /// Gets or sets the maximum number of points (cells).
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(5)]
        public int MaxPoints
        {
            get { return m_points; }
            set
            {
                if (m_points == value)
                    return;

                m_points = value;
                int width = m_tileWidth * m_points + 3;
                this.Size = new Size(width, this.Height);
            }
        }

        int m_baseValue = 0;

        /// <summary>
        /// Gets or sets the base value for the attribute.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(0)]
        public int BaseValue
        {
            get { return m_baseValue; }
            set
            {
                if (m_baseValue == value)
                    return;

                m_baseValue = value;
                Invalidate();
            }
        }

        int m_value = 0;

        /// <summary>
        /// Gets or sets the value of the attribute.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(0)]
        public int Value
        {
            get { return m_value; }
            set
            {
                if (m_value == value)
                    return;

                m_value = value;
                Invalidate();
            }
        }

        private int m_tileWidth = 6;
        private int m_tileHeight = 20;

        private int m_highlightedItem = -1;

        [Category("Behavior")]
        public event ValueChangingHandler ValueChanging;

        [Category("Behavior")]
        public event ValueChangedHandler ValueChanged;

        [Category("Behavior")]
        public event HighlightingHandler Highlighting;

        /// <summary>
        /// Initializes a new instance of <see cref="AttributeBarControl"/>.
        /// </summary>
        public AttributeBarControl()
            : base()
        {
            this.SetStyle(
                ControlStyles.AllPaintingInWmPaint |
                ControlStyles.Opaque |
                ControlStyles.UserPaint,
                true);
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);

            m_tileHeight = this.Height - 4;
            // Calculate tile width
            m_tileWidth = (this.Width - 3) / m_points;
            // Calculate width of control from width of tiles
            int width = m_tileWidth * m_points + 3;
            if (this.Width != width)
                this.Size = new Size(width, this.Height);
        }

        /// <summary>
        /// Returns the index of a tile at specified point.
        /// </summary>
        /// <param name="location">Location to check</param>
        /// <returns>Tile index</returns>
        private int GetValueAt(Point location)
        {
            if (location.Y == 0 || location.Y == this.Height - 1)
                return -1;

            if (location.X == 0 || location.X == this.Width - 1)
                return -1;

            // Return not more than the number of cells
            return Math.Min((location.X - 1) / m_tileWidth + 1, m_points);
        }

        /// <summary>
        /// Changes highlighted tile and causes invalidation of it's region.
        /// </summary>
        /// <param name="newHighlight">Index of the new highlighted tile. Can be -1 if no one is highlighted.</param>
        private void ChangeHighlight(int newHighlight)
        {
            int previousHighlighted = m_highlightedItem;
            m_highlightedItem = newHighlight;

            if (previousHighlighted < 0 && m_highlightedItem < 0)
                return;

            // Invalidate changed areas
            if (m_highlightedItem >= 0)
            {
                Invalidate(new Rectangle(
                    m_highlightedItem * m_tileWidth + 1,
                    2,
                    m_tileWidth,
                    m_tileHeight
                ));
            }

            if (previousHighlighted >= 0)
            {
                Invalidate(new Rectangle(
                    previousHighlighted * m_tileWidth + 1,
                    2,
                    m_tileWidth,
                    m_tileHeight
                ));
            }
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);

            if (!Enabled)
                return;

            // Change cursor if required
            int value = GetValueAt(e.Location);
            Cursor = (value == -1) ? Cursors.Arrow : Cursors.Hand;

            int newHighlight = value;
            if (newHighlight >= 0 && newHighlight < m_baseValue)
                newHighlight = m_baseValue;

            if (Highlighting != null)
                Highlighting(this, ref newHighlight);

            // To zero-based value
            newHighlight--;

            if (m_highlightedItem == newHighlight)
                return;

            ChangeHighlight(newHighlight);
        }

        protected override void OnMouseLeave(EventArgs e)
        {
            base.OnMouseLeave(e);

            ChangeHighlight(-1);
        }

        protected override void OnMouseClick(MouseEventArgs e)
        {
            base.OnMouseClick(e);

            if (!Enabled)
                return;

            int newValue = GetValueAt(e.Location);

            if (newValue == -1)
                return;

            if (newValue < m_baseValue)
                newValue = m_baseValue;

            int deltaValue = newValue - m_value;

            if (deltaValue == 0)
                return;

            if (ValueChanging != null)
                ValueChanging(this, ref deltaValue);

            if (deltaValue == 0)
                return;

            this.Value += deltaValue;
            if (ValueChanged != null)
                ValueChanged(this);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            using (Graphics g = e.Graphics)
            {
                // Draw the borders
                g.DrawRectangle(m_outerBorderPen, 0, 0, this.Width - 1, this.Height - 1);
                g.DrawRectangle(m_borderPen, 1, 1, this.Width - 3, this.Height - 3);

                // Draw the tiles
                for (int iTile = 0; iTile < m_points; iTile++)
                {
                    DrawTile(g, iTile);
                }
            }
        }

        /// <summary>
        /// Draws a tile.
        /// </summary>
        /// <param name="g">A <see cref="System.Drawing.Graphics"/> object for drawing</param>
        /// <param name="iTile">Index of the tile</param>
        protected virtual void DrawTile(Graphics g, int iTile)
        {
            // Select brush
            SolidBrush brush;
            if (iTile >= m_value)
                brush = m_inactiveBrush;
            else
                if (iTile >= m_baseValue - 1)
                    brush = m_spentPointBrush;
                else
                    brush = m_basePointBrush;

            if (iTile == m_highlightedItem)
            {
                // Highlight cell color
                const int shift = 50;
                brush = new SolidBrush(ShiftColor(brush.Color, shift));
            }

            int x = 1 + iTile * m_tileWidth; ;
            // Draw the tile
            g.FillRectangle(brush, x, 2, m_tileWidth, m_tileHeight);

            if (iTile == m_highlightedItem)
                brush.Dispose();

            // Draw the tile's border
            g.DrawLine(m_borderPen, x, 2, x, this.Height - 2);
        }

        /// <summary>
        /// Makes a color lighter or darker.
        /// </summary>
        /// <param name="color">Source color</param>
        /// <param name="shift">Color shift</param>
        /// <returns></returns>
        protected Color ShiftColor(Color color, int shift)
        {
            return Color.FromArgb(
                    Math.Min(color.R + shift, 255),
                    Math.Min(color.G + shift, 255),
                    Math.Min(color.B + shift, 255)
                    );
        }
    }
}
