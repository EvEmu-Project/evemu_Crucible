using System;
using System.Drawing;
using System.Windows.Forms;

namespace EVEMon.SkillPlanner
{
    public partial class LineDrawControl : UserControl
    {
        public LineDrawControl()
        {
            InitializeComponent();
            SetStyle(ControlStyles.SupportsTransparentBackColor, true);
            SetStyle(ControlStyles.ResizeRedraw, true);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            //e.Graphics.FillRectangle(Brushes.Red, e.ClipRectangle);
            using (Pen p = new Pen(Color.White, 5))
            {
                e.Graphics.DrawLine(p, m_lineFrom, m_lineTo);
            }
        }

        private Point m_pointA = Point.Empty;
        private Point m_pointB = Point.Empty;
        private Point m_lineFrom = Point.Empty;
        private Point m_lineTo = Point.Empty;

        internal void SetPointA(int x, int y)
        {
            m_pointA = new Point(x, y);
            TryPosition();
        }

        internal void SetPointB(int x, int y)
        {
            m_pointB = new Point(x, y);
            TryPosition();
        }

        private void TryPosition()
        {
            if (m_pointA != Point.Empty && m_pointB != Point.Empty)
            {
                int left = Math.Min(m_pointA.X, m_pointB.X);
                int top = Math.Min(m_pointA.Y, m_pointB.Y);
                int right = Math.Max(m_pointA.X, m_pointB.X);
                int bottom = Math.Max(m_pointA.Y, m_pointB.Y);
                int width = right - left;
                int height = bottom - top;

                m_lineFrom = new Point(m_pointA.X - left, m_pointA.Y - top);
                m_lineTo = new Point(m_pointB.X - left, m_pointB.Y - top);

                if (width == 0)
                {
                    left -= 5;
                    width = 10;
                }
                if (height == 0)
                {
                    top -= 5;
                    height = 10;
                }

                this.SetBounds(left, top, width, height);
            }
        }
    }
}