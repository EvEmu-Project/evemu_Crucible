using System.Globalization;
using System.Windows.Forms;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Derived from TreeView class
    /// <para>Overrides standard node double click behaviour to prevent node expand / collapse actions</para>
    /// </summary>
    internal class ReqSkillsTreeView : System.Windows.Forms.TreeView
    {
        private const int WM_LBUTTONDBLCLK = 0x203;

        protected override void WndProc(ref Message m)
        {
            if (m.Msg == WM_LBUTTONDBLCLK)
            {
                handleDoubleClick(ref m);
            }
            else { base.WndProc(ref m); };
        }

        private void handleDoubleClick(ref Message m)
        {
            // Get mouse location from message.lparam
            // x is low order word, y is high order word
            string lparam = m.LParam.ToString("X08");
            int x = int.Parse(lparam.Substring(4, 4), NumberStyles.HexNumber);
            int y = int.Parse(lparam.Substring(0, 4), NumberStyles.HexNumber);
            // Test for a treenode at this location
            TreeViewHitTestInfo info = this.HitTest(x, y);
            if (info.Node != null)
            {
                // Raise NodeMouseDoubleClick event
                TreeNodeMouseClickEventArgs e = new TreeNodeMouseClickEventArgs(info.Node, MouseButtons.Left, 2, x, y);
                this.OnNodeMouseDoubleClick(e);
            }
        }

        protected override void CreateHandle()
        {
            if (!this.IsDisposed)
                base.CreateHandle();
        }
    }
}
