using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common.Controls
{
    /// <summary>
    /// This base class provides the common icon shared by all of our forms, along with an optional position storing service.
    /// </summary>
    public partial class EVEMonForm : Form
    {
        protected const int MaxTitleLength = 259;

        private delegate void OnLayoutCallback(LayoutEventArgs levent);

        private bool m_loaded;

        /// <summary>
        /// Constructor
        /// </summary>
        public EVEMonForm()
        {
            InitializeComponent();
            Font = FontFactory.GetFont("Tahoma", 8.25F, FontStyle.Regular);
        }

        /// <summary>
        /// Gets or sets a key used to store and restore the position and size of the window. When null or empty, the position won't be persisted.
        /// </summary>
        [Category("Behavior")]
        [Description("A key used to store and restore the position and size of the window. When null or empty, the position won't be persisted.")]
        public string RememberPositionKey { get; set; }

        /// <summary>
        /// On load, restores the window rectangle from the settings.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            RestoreLocation();
            m_loaded = true;
            SaveLocation();
        }

        /// <summary>
        /// On closing, stores the window rect.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            SaveLocation();
            base.OnFormClosed(e);
        }

        /// <summary>
        /// When the size changes, stores the window rect.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnSizeChanged(EventArgs e)
        {
            SaveLocation();
            base.OnSizeChanged(e);
        }

        /// <summary>
        /// When the location changes, stores the window rect.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLocationChanged(EventArgs e)
        {
            SaveLocation();
            base.OnLocationChanged(e);
        }

        /// <summary>
        /// Saves the window rect to the settings when the key is not null or empty.
        /// </summary>
        private void SaveLocation()
        {
            if (!m_loaded)
                return;

            if (String.IsNullOrEmpty(RememberPositionKey))
                return;

            Rectangle r = new Rectangle(Location, Size);
            if (WindowState == FormWindowState.Normal && VerifyValidWindowLocation(r) == r)
            {
                Settings.UI.WindowLocations[RememberPositionKey] = 
                    (SerializableRectangle)new Rectangle(Location, Size);
            }
        }

        /// <summary>
        /// Restores the window rect from the settings when the key is not null or empty.
        /// </summary>
        private void RestoreLocation()
        {
            if (String.IsNullOrEmpty(RememberPositionKey))
                return;

            List<Form> formList = new List<Form>();
            foreach (Form form in Application.OpenForms)
            {
                if (form.GetType() == this.GetType())
                    formList.Add(form);
            }

            if (Settings.UI.WindowLocations.ContainsKey(RememberPositionKey))
            {
                Rectangle r = (Rectangle)Settings.UI.WindowLocations[RememberPositionKey];
                if (formList.Count > 1)
                {
                    Point pfl = formList[formList.Count - 2].Location;
                    r.Location = new Point(pfl.X + 20, pfl.Y + 20);
                }

                r = VerifyValidWindowLocation(r);
                SetBounds(r.X, r.Y, r.Width, r.Height);
            }
        }

        /// <summary>
        /// Verify the window location is valid and resets it when necessary.
        /// </summary>
        /// <param name="inRect">The proposed rectangle.</param>
        /// <returns>The corrected rectangle.</returns>
        private Rectangle VerifyValidWindowLocation(Rectangle inRect)
        {
            Point p = inRect.Location;
            Size s = inRect.Size;
            s.Width = Math.Max(s.Width, MinimumSize.Width);
            s.Height = Math.Max(s.Height, MinimumSize.Height);

            foreach (Screen ts in Screen.AllScreens)
            {
                if (ts.WorkingArea.Contains(inRect))
                    return inRect;

                if (ts.WorkingArea.Contains(p))
                {
                    p.X = ts.WorkingArea.Left + 50;
                    p.Y = ts.WorkingArea.Top + 100;
                    return new Rectangle(p, s);
                }
            }

            p.X = Screen.PrimaryScreen.WorkingArea.X + 5;
            p.Y = Screen.PrimaryScreen.WorkingArea.Y + 5;
            return new Rectangle(p, s);
        }
    }
}
