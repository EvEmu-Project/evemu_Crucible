using System.Drawing;
using System.Windows.Forms;

namespace EVEMon.Controls
{
    /// <summary>
    /// This control draws a border around its children.
    /// Unfortunately, you need to adjust the padding and such to prevents the top and left lines to be hidden by your controls.
    /// There is probably some way to change the client rectangle area but I didn't find it.
    /// Did I mention I hate winforms very much ?
    /// </summary>
    public partial class BorderPanel : Panel
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        public BorderPanel()
        {
            InitializeComponent();
            this.BackColor = SystemColors.Window;
            this.ForeColor = Color.Gray;
            this.Padding = new Padding(0);
            this.Margin = new Padding(0);
            this.SetStyle(ControlStyles.UserPaint, true);
            this.SetStyle(ControlStyles.ResizeRedraw, true);
            this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            this.SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
        }

        /// <summary>
        /// Paints the border.
        /// </summary>
        /// <param name="pe"></param>
        protected override void OnPaint(PaintEventArgs pe)
        {
            base.OnPaint(pe);
            var rect = this.ClientRectangle;
            rect.Inflate(-1, -1);

            using (var brush = new SolidBrush(this.ForeColor))
            {
                using (var pen = new Pen(brush, 1.0f))
                {
                    pe.Graphics.DrawRectangle(pen, rect);
                }
            }
        }
    }
}
