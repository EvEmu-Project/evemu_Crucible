using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Design;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using System.Windows.Forms.Design;

namespace EVEMon.Controls
{
    public class ExpandablePanelControl : NoFlickerPanel
    {
        // Settings
        private PanelStatus m_status;
        protected AnimationSpeed animationSpeed;
        protected Direction expandDirection = Direction.Up;
        private int m_animationStep;
        private int m_expandedHeight;
        private bool m_beginExpanded;
        
        // Header
        protected NoFlickerPanel nfpHeader;
        private string m_headerText = "Header Text";

        // ContextMenu
        protected ContextMenuStrip contextMenuStrip;
        protected ToolStripMenuItem tsmiExpandCollapse;
        protected ToolStripMenuItem tsmiSelectAnim;
        protected ToolStripSeparator tsmiSeparator;
        protected ToolStripMenuItem tsmiNoAnim;
        protected ToolStripMenuItem tsmiHighAnim;
        protected ToolStripMenuItem tsmiMedAnim;
        protected ToolStripMenuItem tsmiLowAnim;

        private bool m_enableContextMenu;

        // Graphics variables
        private StringFormat hCenteredStringFormat;
        private Bitmap headerImage;
        private Bitmap expandImage;
        private Bitmap collapseImage;
        private Pen pen;
        private Brush brush;
        private int m_offset;
        private int m_pad = 6;


        #region Constructor

        /// <summary>
        /// Cunstructor.
        /// </summary>
        public ExpandablePanelControl()
            : base()
        {
            // Header
            CreateHeader();

            // ContextMenu
            CreateContextMenu();

            // Event handlers
            nfpHeader.Paint += new PaintEventHandler(nfpHeader_Paint);
        }

        /// <summary>
        /// Gets true if the panel is expanded.
        /// </summary>
        internal bool IsExpanded
        {
            get { return m_status == PanelStatus.Expanded; }
        }

        /// <summary>
        /// Gets the headerof the panel.
        /// </summary>
        internal NoFlickerPanel Header
        {
            get { return nfpHeader; }
        }

        /// <summary>
        /// Gets or sets the header text.
        /// </summary>
        internal string HeaderText
        {
            get { return m_headerText; }
            set { m_headerText = value; }
        }

        /// <summary>
        /// Gets or sets the expanded height of the  panel.
        /// </summary>
        internal int ExpandedHeight
        {
            get { return m_expandedHeight; }
            set
            {
                m_expandedHeight = value;

                // If we start collapsed we don't have to update the height
                if (!IsExpanded && !m_beginExpanded)
                    return;

                Height = m_expandedHeight;
                Invalidate();
                Update();
            }
        }

        #endregion


        #region Control Creation Methods

        /// <summary>
        /// Creates the header.
        /// </summary>
        private void CreateHeader()
        {
            nfpHeader = new NoFlickerPanel();
            nfpHeader.Width = Width;
            nfpHeader.Height = 30;
            nfpHeader.BackColor = Color.Transparent;
            Controls.Add(nfpHeader);
        }

        /// <summary>
        /// Creates the context menu.
        /// </summary>
        private void CreateContextMenu()
        {
            contextMenuStrip = new ContextMenuStrip();
            tsmiExpandCollapse = new ToolStripMenuItem();
            tsmiSelectAnim = new ToolStripMenuItem();
            tsmiNoAnim = new ToolStripMenuItem();
            tsmiHighAnim = new ToolStripMenuItem();
            tsmiMedAnim = new ToolStripMenuItem();
            tsmiLowAnim = new ToolStripMenuItem();
            tsmiSeparator = new ToolStripSeparator();
            
            // Add menu items
            contextMenuStrip.Items.Add(tsmiExpandCollapse);
            contextMenuStrip.Items.Add(tsmiSeparator);
            contextMenuStrip.Items.Add(tsmiSelectAnim);
            tsmiSelectAnim.DropDownItems.AddRange(new ToolStripItem[]
            {
                tsmiNoAnim,
                tsmiHighAnim,
                tsmiMedAnim,
                tsmiLowAnim
            });
            
            // Apply properties
            tsmiSelectAnim.Text = "Animation Speed";
            tsmiNoAnim.Text = "None";
            tsmiHighAnim.Text = "High";
            tsmiMedAnim.Text = "Medium";
            tsmiLowAnim.Text = "Low";

            // Subscribe events
            tsmiExpandCollapse.Click += new EventHandler(tsmiExpandCollapse_Click);
            tsmiNoAnim.Click += new EventHandler(animationSpeedSelect_Click);
            tsmiHighAnim.Click += new EventHandler(animationSpeedSelect_Click);
            tsmiMedAnim.Click += new EventHandler(animationSpeedSelect_Click);
            tsmiLowAnim.Click += new EventHandler(animationSpeedSelect_Click);
        }

        #endregion


        #region Graphics Methods

        /// <summary>
        /// Draws the main panel.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnPaint(PaintEventArgs e)
        {
            Graphics gr = e.Graphics;
            gr.SmoothingMode = SmoothingMode.AntiAlias;
            pen = new Pen(SystemBrushes.ControlDark, 1);

            gr.DrawLine(pen, 0, 0, 0, Height);
            gr.DrawLine(pen, 0, Height - 1, Width - 1, Height - 1);
            gr.DrawLine(pen, Width - 1, Height - 1, Width - 1, 0);
            gr.DrawLine(pen, Width - 1, 0, 0, 0);

            int height = (expandDirection == Direction.Up ? Height - nfpHeader.Height : 0);
            nfpHeader.Location = new Point(0, height);
            
            base.OnPaint(e);
        }

        /// <summary>
        /// Draws the header panel.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nfpHeader_Paint(object sender, PaintEventArgs e)
        {
            Graphics gr = e.Graphics;
            gr.SmoothingMode = SmoothingMode.AntiAlias;

            hCenteredStringFormat = new StringFormat();
            hCenteredStringFormat.LineAlignment = StringAlignment.Center;

            brush = Brushes.Black;
            nfpHeader.Width = Width;
            headerImage = (IsExpanded ? collapseImage : expandImage);

            if (headerImage != null)
            {
                m_offset = headerImage.Width + m_pad;
                gr.DrawImage(headerImage, new Rectangle(m_pad, nfpHeader.Height / 2 - headerImage.Height / 2, headerImage.Width, headerImage.Height));
            }

            gr.DrawString(HeaderText, Font, brush, new RectangleF(m_pad + m_offset, 0, nfpHeader.Width - m_pad * 4, nfpHeader.Height), hCenteredStringFormat);
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets or sets the image shown in the header when Panel is collapsed. Height must be less than HeaderHeight - 4 pixels. Null to disable it.
        /// </summary>
        [Description("The image used in the header when the Panel is collapsed.")]
        public Bitmap ImageExpand
        {
            get { return expandImage; }
            set
            {
                if (value != null)
                {
                    if (value.Height > nfpHeader.Height - 4)
                    {
                        throw new ArgumentException("HeaderIcon: Height must be less than HeaderHeight - 4 pixels.");
                    }
                }
                expandImage = value;
                nfpHeader.Invalidate();
                nfpHeader.Update();
            }
        }

        /// <summary>
        /// Gets or sets the image shown in the header when Panel is expanded. Height must be less than HeaderHeight - 4 pixels. Null to disable it.
        /// </summary>
        [Description("The image used in the header when the Panel is expanded.")]
        public Bitmap ImageCollapse
        {
            get { return collapseImage; }
            set
            {
                if (value != null)
                {
                    if (value.Height > nfpHeader.Height - 4)
                    {
                        throw new ArgumentException("HeaderIcon: Height must be less than HeaderHeight - 4 pixels.");
                    }
                }
                collapseImage = value;
                nfpHeader.Invalidate();
                nfpHeader.Update();
            }
        }

        /// <summary>
        /// Gets or sets the Expand/Collapse Speed.
        /// </summary>
        [Description("Set the Expand/Collapse Speed.")]
        public AnimationSpeed AnimationSpeed
        {
            get { return animationSpeed; }
            set { animationSpeed = value; }
        }

        /// <summary>
        /// Gets or sets the Header's height of the Panel. Height must be between 30 and 50.
        /// </summary>
        [Description("Set the Header's height of the Panel. Height must be between 30 and 50.")]
        public int HeaderHeight
        {
            get { return nfpHeader.Height; }
            set
            {
                if (value < 30 || value > 50)
                    throw new ArgumentException("Height must be between 30 and 50.");

                nfpHeader.Height = value;
            }
        }

        /// <summary>
        /// Gets or sets the direction of the Panel expansion.
        /// </summary>
        [Description("Set the direction of the Panel expansion.")]
        public Direction ExpandDirection
        {
            get { return expandDirection; }
            set { expandDirection = value; }
        }

        /// <summary>
        /// Gets or sets  whether the Panel should start expanded.
        /// </summary>
        [Description("Indicates whether the Panel should start expanded.")]
        public bool ExpandedOnStartup
        {
            get { return m_beginExpanded; }
            set { m_beginExpanded = value; }
        }

        /// <summary>
        /// Gets or sets whether the contextMenu should be shown by right-clicking on the header.
        /// </summary>
        [Description("Indicates whether the contextMenu should be shown by right-clicking on the header.")]
        public bool EnableContextMenu
        {
            get { return m_enableContextMenu; }
            set { m_enableContextMenu = value; }
        }

        #endregion


        #region Expand/Collapse Methods

        /// <summary>
        /// Expands the panel.
        /// </summary>
        private void ExpandPanel()
        {
            while (AnimationSpeed != AnimationSpeed.NoAnimation && Height < m_expandedHeight - m_animationStep)
            {
                Height += m_animationStep;
                this.Invalidate();
                this.Update();
            }

            Height = m_expandedHeight;
            headerImage = collapseImage;
            m_status = PanelStatus.Expanded;

            this.Invalidate();
            this.Update();

            // Clear memory
            GC.Collect();
        }

        /// <summary>
        /// Collapses the panel.
        /// </summary>
        private void CollapsePanel()
        {
            while (AnimationSpeed != AnimationSpeed.NoAnimation && Height > nfpHeader.Height + m_animationStep)
            {
                Height -= m_animationStep;
                this.Invalidate();
                this.Update();
            }

            Height = nfpHeader.Height;
            headerImage = expandImage;
            m_status = PanelStatus.Collapsed;

            this.Invalidate();
            this.Update();

            // Clear memory
            GC.Collect();
        }

        /// <summary>
        /// Triggers the panel to expand or collapse.
        /// </summary>
        private void SwitchStatus()
        {
            switch (m_status)
            {
                case PanelStatus.Collapsed:
                    ExpandPanel();
                    break;
                case PanelStatus.Expanded:
                    CollapsePanel();
                    break;
            }

            UpdateContextMenu();
        }

        /// <summary>
        /// Updates the context menu text.
        /// </summary>
        private void UpdateContextMenu()
        {
            tsmiExpandCollapse.Text = (IsExpanded ? "Collapse Panel" : "Expand Panel");
        }

        /// <summary>
        /// Updates the animation speed.
        /// </summary>
        private void UpdateAnimationSpeed()
        {
            switch (animationSpeed)
            {
                case AnimationSpeed.NoAnimation:
                    m_animationStep = ExpandedHeight;
                    break;
                case AnimationSpeed.High:
                    m_animationStep = (int)(ExpandedHeight / 4);
                    break;
                case AnimationSpeed.Medium:
                    m_animationStep = (int)(ExpandedHeight / 20);
                    break;
                case AnimationSpeed.Low:
                    m_animationStep = 1;
                    break;
            }
        }

        #endregion


        #region ContextMenu Events

        /// <summary>
        /// Occurs when we click on an animation choice context menu item.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void animationSpeedSelect_Click(object sender, EventArgs e)
        {
            string choice = ((ToolStripItem)sender).Text;

            switch (choice)
            {
                case "None":
                    AnimationSpeed = AnimationSpeed.NoAnimation;
                    break;
                case "High":
                    AnimationSpeed = AnimationSpeed.High;
                    break;
                case "Medium":
                    AnimationSpeed = AnimationSpeed.Medium;
                    break;
                case "Low":
                    AnimationSpeed = AnimationSpeed.Low;
                    break;
            }

            UpdateAnimationSpeed();
        }

        /// <summary>
        /// Occurs when we click the Expand/Collapse context menu item.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmiExpandCollapse_Click(object sender, EventArgs e)
        {
            SwitchStatus();
        }

        #endregion


        #region Event Handlers

        /// <summary>
        /// Updates the controls settings.
        /// </summary>
        protected override void OnCreateControl()
        {
            if (DesignMode)
                return;

            // Set the expanded height of the panel according to the height set in the designer
            // It can be set to a manual height by replacing "Height" with the number of your choice
            m_expandedHeight = Height;

            // Set the panel status for startup
            m_animationStep = m_expandedHeight;
            m_status = (m_beginExpanded ? PanelStatus.Collapsed : PanelStatus.Expanded);
            SwitchStatus();

            // Set the animation speed
            UpdateAnimationSpeed();

            base.OnCreateControl();
        }

        /// <summary>
        /// Forces the panel to redraw.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnSizeChanged(EventArgs e)
        {
            this.Invalidate();
            base.OnSizeChanged(e);
        }

        /// <summary>
        /// Occurs on a mouse click in the main panel.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        internal void expandablePanelControl_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right && !m_enableContextMenu)
                return;

            if (e.Button == MouseButtons.Right)
            {
                int height = (ExpandDirection == Direction.Up ? Height - nfpHeader.Height + e.Y : e.Y);
                contextMenuStrip.Enabled = m_enableContextMenu;
                contextMenuStrip.Show(this, new Point(e.X, height));
                contextMenuStrip.BringToFront();
                return;
            }

            SwitchStatus();
        }

        #endregion

    }

    #region Enumerations

    /// <summary>
    /// Enumerator for the status of the panel.
    /// </summary>
    public enum PanelStatus
    {
        Expanded,
        Collapsed
    }

    /// <summary>
    /// Enumerator for the speed of the Expand/Collapse animation.
    /// </summary>
    public enum AnimationSpeed
    {
        NoAnimation,
        High,
        Medium,
        Low
    }

    /// <summary>
    /// Enumerator for the direction of the expansion.
    /// </summary>
    public enum Direction
    {
        Up,
        Down
    }

    #endregion
}
