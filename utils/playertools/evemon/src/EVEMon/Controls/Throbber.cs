using System;
using System.Drawing;
using System.Windows.Forms;

using CommonProperties = EVEMon.Common.Properties;

namespace EVEMon.Controls
{
    public enum ThrobberState
    {
        Stopped,
        Rotating,
        Strobing
    }

    /// <summary>
    /// The little "flower" displayed on the top right of the characters monitors.
    /// </summary>
    public partial class Throbber : PictureBox
    {
        // Static members
        private static int m_runners;
        private static Timer m_timer;
        private static Image m_strobeFrame = null;
        private static Image[] m_movingFrames = null;

        // Instance members
        private ThrobberState m_state = ThrobberState.Stopped;
        private bool m_running;
        private int m_ticks;

        /// <summary>
        /// Constructor
        /// </summary>
        public Throbber()
        {
            InitializeComponent();

            // Initializes the common images
            if (m_strobeFrame == null)
            {
                InitImages();
            }

            // Initializes the common timer
            if (m_timer == null)
            {
                m_timer = new Timer();
                m_timer.Interval = 100;
            }

            // Always subscribed to the timer (ridiculous CPU overhead, less work for the GC with no subscriptions/unsubscriptions, cleaner code)
            m_timer.Tick += m_timer_Tick;

            // Forces the control to be 24*24
            this.MinimumSize = new Size(24, 24);
            this.MaximumSize = new Size(24, 24);
        }

        /// <summary>
        /// Gets or sets the throbber's state
        /// </summary>
        public ThrobberState State
        {
            get { return m_state; }
            set
            {
                // Is the state unchanged ? 
                if (value == m_state) return;
                m_state = value;

                // Leave it stopped if not visible
                if(!this.Visible) return;

                // Start or stop
                switch (m_state)
                {
                    case ThrobberState.Rotating:
                    case ThrobberState.Strobing:
                        Start();
                        Refresh();
                        break;

                    default:
                        Stop();
                        Invalidate();
                        break;
                }
            }
        }

        /// <summary>
        /// Start animating this throbber
        /// </summary>
        private void Start()
        {
            // Always refresh the ticks since we're changing the state or becoming visible
            m_ticks = 0;

            // Is it already running ?
            if (m_running) return;
            m_running = true;

            // Start
            m_runners++;
            if (m_runners == 1) m_timer.Start();
        }

        /// <summary>
        /// Stop animating this throbber
        /// </summary>
        private void Stop()
        {
            // Is it already stopped ?
            if (!m_running) return;
            m_running = false;

            // Stop
            m_runners--;
            if (m_runners == 0) m_timer.Stop();
        }

        /// <summary>
        /// occurs 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_timer_Tick(object sender, EventArgs e)
        {
            // Invalidates the control
            Refresh();
            m_ticks++;
        }

        /// <summary>
        /// Handles the painting
        /// </summary>
        /// <param name="pe"></param>
        protected override void OnPaint(PaintEventArgs pe)
        {
            Image frame = m_strobeFrame;

            // Select the frame to display
            switch (m_state)
            {
                case ThrobberState.Rotating:
                    frame = m_movingFrames[m_ticks % m_movingFrames.Length];
                    break;

                case ThrobberState.Strobing:
                    if ((m_ticks % 10) >= 5) return;
                    break;

                default:
                    break;
            }

            // Draw the selected image
            pe.Graphics.DrawImage(frame, 0, 0);

            // Calling the base method
            base.OnPaint(pe);
        }

        /// <summary>
        /// Any time the visibility change, we may start or stop the timer.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            // When not visible, stop
            if (!this.Visible)
            {
                this.Stop();
                return;
            }

            // When animated and visible, restart
            if (m_state != ThrobberState.Stopped)
            {
                Start();
            }
        }

        /// <summary>
        /// Initialize the images shared by those controls.
        /// </summary>
        private static void InitImages()
        {
            int width = 24;
            int height = 24;
            Image b = CommonProperties.Resources.Throbber;

            //Make the stopped Image
            m_strobeFrame = new Bitmap(width, height);
            using (Graphics g = Graphics.FromImage(m_strobeFrame))
            {
                g.DrawImage(b, new Rectangle(0, 0, width, height), new Rectangle(0, 0, width, height), GraphicsUnit.Pixel);
            }

            //Make the moving Images
            m_movingFrames = new Image[8];
            for (int i = 1; i < 9; i++)
            {
                Bitmap ib = new Bitmap(width, height);
                using (Graphics g = Graphics.FromImage(ib))
                {
                    g.DrawImage(b, new Rectangle(0, 0, width, height), new Rectangle(i * width, 0, width, height), GraphicsUnit.Pixel);
                }
                m_movingFrames[i - 1] = ib;
            }
        }
    }
}
