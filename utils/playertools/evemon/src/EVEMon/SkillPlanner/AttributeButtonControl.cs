using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.ComponentModel;
using System.Drawing.Drawing2D;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Represents a button for changing the value of an attribute.
    /// It is not changes value of <see cref="AttributeBarControl"/> itself. 
    /// Instead this button provides associated <see cref="AttributeBarControl"/> and change amount for parent control.
    /// Parent will perform changes itself, based on unassignet attribute pool and parameters of a button.
    /// </summary>
    public class AttributeButtonControl : Control
    {
        private Pen borderPen = Pens.Black;
        private Pen borderPenInactive = Pens.Gray;

        private Brush backgroundBrush = Brushes.LightGray;
        private Brush backgroundBrushHighlighted = Brushes.WhiteSmoke;
        private Brush backgroundBrushPressed = Brushes.DarkGray;

        private GraphicsPath borderPath;

        private int valueChange;

        /// <summary>
        /// Gets ot sets the change of the atribute value.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(0)]
        public int ValueChange
        {
            get { return valueChange; }
            set { valueChange = value; }
        }

        private AttributeBarControl attributeBar;

        /// <summary>
        /// Gets or sets <see cref="AttributeBarControl"/> associated with this button.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(null)]
        public AttributeBarControl AttributeBar
        {
            get { return attributeBar; }
            set { attributeBar = value; }
        }

        /// <summary>
        /// Initializes new instance of <see cref="AttributeButtonControl"/>.
        /// </summary>
        public AttributeButtonControl()
            : base()
        {
            SetStyle(ControlStyles.SupportsTransparentBackColor, true);
            SetStyle(ControlStyles.StandardDoubleClick, false);
            borderPath = CreateBorderPath();
        }

        protected override void OnForeColorChanged(EventArgs e)
        {
            base.OnForeColorChanged(e);
            borderPen = new Pen(this.ForeColor);
        }

        private bool hover = false;
        private bool pressed = false;

        protected override void OnMouseEnter(EventArgs e)
        {
            base.OnMouseEnter(e);

            hover = true;
            Invalidate();
        }

        protected override void OnMouseLeave(EventArgs e)
        {
            base.OnMouseLeave(e);

            hover = false;
            Invalidate();
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseDown(e);

            if (e.Button == MouseButtons.Left)
                pressed = true;

            Invalidate();
        }

        protected override void OnMouseUp(MouseEventArgs e)
        {
            base.OnMouseUp(e);

            if (e.Button == MouseButtons.Left)
                pressed = false;

            Invalidate();
        }

        protected override void OnResize(EventArgs e)
        {
            if (borderPath != null)
                borderPath.Dispose();

            borderPath = CreateBorderPath();

            base.OnResize(e);
        }

        /// <summary>
        /// Creates a path in form of rounded rectangle. This path is used as shape of the button.
        /// </summary>
        /// <returns>Created path</returns>
        private GraphicsPath CreateBorderPath()
        {
            GraphicsPath borderPath = new GraphicsPath();

            const int radius = 3;
            int h = 1;
            int v = 1;
            int width = this.Width - 2;
            int height = this.Height - 2;
            borderPath.AddLine(h + radius, v, h + width - (radius * 2), v);
            borderPath.AddArc(h + width - (radius * 2), v, radius * 2, radius * 2, 270, 90);
            borderPath.AddLine(h + width, v + radius, h + width, v + height - (radius * 2));
            borderPath.AddArc(h + width - (radius * 2), v + height - (radius * 2), radius * 2, radius * 2, 0, 90);
            borderPath.AddLine(h + width - (radius * 2), v + height, h + radius, v + height);
            borderPath.AddArc(h, v + height - (radius * 2), radius * 2, radius * 2, 90, 90);
            borderPath.AddLine(h, v + height - (radius * 2), h, v + radius);
            borderPath.AddArc(h, v, radius * 2, radius * 2, 180, 90);
            borderPath.CloseFigure();

            return borderPath;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            using (Graphics g = e.Graphics)
            {
                g.SmoothingMode = SmoothingMode.HighQuality;

                Pen pen = this.Enabled ? borderPen : borderPenInactive;
                Brush brush;
                if (pressed)
                    brush = backgroundBrushPressed;
                else
                    if (hover)
                        brush = backgroundBrushHighlighted;
                    else
                        brush = backgroundBrush;

                e.Graphics.FillPath(brush, borderPath);
                e.Graphics.DrawPath(pen, borderPath);

                e.Graphics.DrawLine(pen, 4, this.Height / 2, this.Width - 4, this.Height / 2);

                if (this.valueChange >= 0)
                    e.Graphics.DrawLine(pen, this.Width / 2, 4, this.Width / 2, this.Height - 4);
            }
        }
    }
}
