using System.Drawing;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System;

namespace EVEMon.Controls
{
    public class NoFlickerListBox : ListBox
    {
        protected override void OnDrawItem(DrawItemEventArgs e)
        {
            Rectangle newBounds = new Rectangle(0, 0, e.Bounds.Width, e.Bounds.Height);

            if (newBounds.Width == 0 || newBounds.Height == 0) 
                return;

            // stacked using blocks to avoid indentation, don't need to call IDisposable.Dispose explicitly
            using (BufferedGraphicsContext currentContext = BufferedGraphicsManager.Current)
            using (BufferedGraphics bufferedGraphics = currentContext.Allocate(e.Graphics, newBounds))
            {
                DrawItemEventArgs newArgs = new DrawItemEventArgs(
                    bufferedGraphics.Graphics, e.Font, newBounds, e.Index, e.State, e.ForeColor, e.BackColor);

                // Supply the real DrawItem with the off-screen graphics context
                base.OnDrawItem(newArgs);

                NativeMethods.CopyGraphics(e.Graphics, e.Bounds, bufferedGraphics.Graphics, new Point(0, 0));
            }
        }

        private enum WM
        {
            WM_NULL = 0x0000,
            WM_ERASEBKGND = 0x0014
        }

        protected override void WndProc(ref Message m)
        {
            switch (m.Msg)
            {
                case (int)WM.WM_ERASEBKGND:
                    PaintNonItemRegion();
                    m.Msg = (int)WM.WM_NULL;
                    break;
            }
            base.WndProc(ref m);
        }

        private void PaintNonItemRegion()
        {
            using (Graphics g = Graphics.FromHwnd(this.Handle))
            using (Region r = new Region(this.ClientRectangle))
            {
                for (int i = 0; i < this.Items.Count; i++)
                {
                    Rectangle itemRect = this.GetItemRectangle(i);
                    r.Exclude(itemRect);
                }
                using (var brush = new SolidBrush(this.BackColor))
                {
                    g.FillRegion(SystemBrushes.Window, r);
                }
            }
        }
    }
}