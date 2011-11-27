using System.Windows.Forms;

namespace EVEMon.Controls
{
    public class NoFlickerPanel : Panel
    {
        public NoFlickerPanel()
        {
            DoubleBuffered = true;
            SetStyle(ControlStyles.DoubleBuffer, true);
            SetStyle(ControlStyles.UserPaint, true);
            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            UpdateStyles();
        }
    }
}

