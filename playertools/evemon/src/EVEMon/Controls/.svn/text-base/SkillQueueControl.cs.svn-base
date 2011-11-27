using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

using EVEMon.Common;

using DescriptionAttribute = System.ComponentModel.DescriptionAttribute;

namespace EVEMon.Controls
{
    public partial class SkillQueueControl : Control
    {
        private DateTime m_nextRepainting = DateTime.MinValue;
        private static DateTime m_paintTime = DateTime.UtcNow;


        #region Constructors, disposing, global events

        /// <summary>
        /// Creates the skill queue control without an associates skill queue.
        /// </summary>
        public SkillQueueControl()
        {
            InitializeComponent();

            m_toolTip = new SkillQueueToolTip(this);

            this.Disposed += new EventHandler(OnDisposed);
            EveClient.TimerTick += new EventHandler(EveClient_TimerTick);
            EveClient.SettingsChanged += new EventHandler(EveClient_SettingsChanged);
            EveClient.CharacterChanged += new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterChanged);
        }

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            this.Disposed -= new EventHandler(OnDisposed);
            EveClient.TimerTick -= new EventHandler(EveClient_TimerTick);
            EveClient.SettingsChanged -= new EventHandler(EveClient_SettingsChanged);
            EveClient.CharacterChanged -= new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterChanged);
            m_toolTip.Dispose();
        }

        /// <summary>
        /// Every second, we checks whether we should update the display.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_TimerTick(object sender, EventArgs e)
        {
            if (DateTime.Now > m_nextRepainting)
                Invalidate();
        }

        /// <summary>
        /// When the settings changed, the "SafeForWork" propety which affects our color schemes may have changed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_SettingsChanged(object sender, EventArgs e)
        {
            this.Invalidate();
        }

        /// <summary>
        /// When the character changes, we invalidate the repainting.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            var ccpCharacter = e.Character as CCPCharacter;

            // Current character isn't a CCP character, so can't have a Queue.
            if (ccpCharacter == null)
                return;

            if (m_skillQueue == null || ccpCharacter.SkillQueue != m_skillQueue)
                return;

            this.Invalidate();
        }
        #endregion


        #region Public Properties
        private SkillQueue m_skillQueue;

        /// <summary>
        /// Skill Queue to be rendered.
        /// </summary>
        [Category("Data")]
        [Description("Skill queue to render on the control canvas")]
        public SkillQueue SkillQueue
        {
            get
            {
                return m_skillQueue;
            }
            set
            {
                m_skillQueue = value;
                this.Invalidate();
            }
        }

        private Color m_firstColor = Color.LightBlue;
        
        /// <summary>
        /// The first of two colors to be used in the queue.
        /// </summary>
        [Category("Appearance")]
        [Description("First color of the component")]
        public Color FirstColor 
        {
            get
            {
                return m_firstColor;
            }

            set
            {
                m_firstColor = value;
                this.Invalidate();
            }
        }

        private Color m_secondColor = Color.DarkBlue;

        /// <summary>
        /// The second of two colours to be used in the queue.
        /// </summary>
        [Category("Appearance")]
        [Description("Second color of the component")]
        public Color SecondColor
        {
            get
            {
                return m_secondColor;
            }
            set
            {
                m_secondColor = value;
                this.Invalidate();
            }
        }

        private Color m_emptyColor = Color.DimGray;

        /// <summary>
        /// The color used for the free space at the end of the queue.
        /// </summary>
        [Category("Appearance")]
        [Description("Color used for the free space at the end of the queue when there are less than 24h of training queued.")]
        public Color EmptyColor
        {
            get
            {
                return m_emptyColor;
            }
            set
            {
                m_emptyColor = value;
                this.Invalidate();
            }
        }

        private Color m_borderColor = Color.Gray;

        /// <summary>
        /// The color used for the border of the queue.
        /// </summary>
        [Category("Appearance")]
        [Description("Color used for the border of the queue.")]
        public Color BorderColor
        {
            get
            {
                return m_borderColor;
            }
            set
            {
                m_borderColor = value;
                this.Invalidate();
            }
        }
        #endregion

        
        #region Overridden Methods
        /// <summary>
        /// Paint the skill queue to the control surface.
        /// </summary>
        /// <remarks>
        /// pe.Graphics is control suface. Width and Height are
        /// derived from the control itself not pe.ClipRectangle 
        /// which could point to part of the control
        /// </remarks>
        /// <param name="pe">Paint Event</param>
        protected override void OnPaint(PaintEventArgs pe)
        {
            base.OnPaint(pe);

            Graphics g = pe.Graphics;
            int width = this.Width;
            int height = this.Height;

            // If we are in DesignMode we just paint a dummy queue
            if (DesignMode)
            {
                PaintDesignerQueue(g, width, height);
            }
            else
            {
                PaintQueue(g, width, height);
            }

            // We need to update the painting only every (24h / width in pixels)
            m_nextRepainting = DateTime.Now.AddSeconds((24 * 3600) / width);
        }
        #endregion


        #region Private Methods
        SkillQueueToolTip m_toolTip;

        /// <summary>
        /// Get the first of the two alternating colours.
        /// </summary>
        /// <remarks>
        /// Implements safe for work functionality
        /// </remarks>
        /// <returns>First Colour property, or dark Gray if in safe for work mode</returns>
        private Color GetFirstColor()
        {
            if (Settings.UI.SafeForWork)
            {
                return Color.DarkGray;
            }
            else
            {
                return m_firstColor;
            }
        }

        /// <summary>
        /// Gets the second of the two alternating colours.
        /// </summary>
        /// <remarks>
        /// Implements safe for work functionality
        /// </remarks>
        /// <returns>Second Colour property, or gray if in safe for work mode</returns>
        private Color GetSecondColor()
        {
            if (Settings.UI.SafeForWork)
            {
                return Color.Gray;
            }
            else
            {
                return m_secondColor;
            }
        }

        /// <summary>
        /// Gets the color for the free time.
        /// </summary>
        /// <remarks>
        /// Implements safe for work functionality
        /// </remarks>
        /// <returns>Second Colour property, or dim gray if in safe for work mode</returns>
        private Color GetEmptyColor()
        {
            if (Settings.UI.SafeForWork)
            {
                return Color.DimGray;
            }
            else
            {
                return m_emptyColor;
            }
        }

        /// <summary>
        /// Gets the border color.
        /// </summary>
        /// <remarks>
        /// Implements safe for work functionality
        /// </remarks>
        /// <returns>Second Colour property, or black if in safe for work mode</returns>
        private Color GetBorderColor()
        {
            if (Settings.UI.SafeForWork)
            {
                return Color.Black;
            }
            else
            {
                return m_borderColor;
            }
        }

        /// <summary>
        /// Paints the point (right pointing arrow) on the canvas.
        /// </summary>
        /// <remarks>
        /// Actually paints an inverse triangle
        /// </remarks>
        /// <param name="g">Graphics canvas</param>
        /// <param name="width">Width of the canvas</param>
        /// <param name="height">Height of the canvas</param>
        private void PaintPoint(Graphics g, int width, int height)
        {
            using(Brush background = new SolidBrush(this.BackColor))
            {
                using(Pen pen = new Pen(GetBorderColor(), 1.0f))
                {
                    int halfHeight = (height / 2);
                    int pointWidth = (height / 2) + 1;

                    // Top triangle
                    PointF topTopLeft = new PointF(width - pointWidth, 0);
                    PointF topTopRight = new PointF(width, 0);
                    PointF topBottomRight = new PointF(width, halfHeight + 1);

                    PointF[] topTriangle = { topTopLeft, topTopRight, topBottomRight };

                    g.FillPolygon(background, topTriangle);

                    // Bottom triangle
                    PointF bottomTopRight = new PointF(width, halfHeight - 1);
                    PointF bottomBottomLeft = new PointF(width - pointWidth, height);
                    PointF bottomBottomRight = new PointF(width, height);

                    PointF[] bottomTriangle = { bottomTopRight, bottomBottomLeft, bottomBottomRight };

                    g.FillPolygon(background, bottomTriangle);

                    // Border (point)
                    g.DrawLine(pen, width - pointWidth, 0, width, halfHeight + 1);
                    g.DrawLine(pen, width, halfHeight - 1, width - pointWidth, height);

                    // Border (top, left, bottom lines)
                    g.DrawLine(pen, 0, 0, width - pointWidth, 0);
                    g.DrawLine(pen, 0, 0, 0, height);
                    g.DrawLine(pen, 0, height - 1, width - pointWidth, height - 1);
                }
            }
        }

        /// <summary>
        /// Paints the first 24 hours of the skill queue including the
        /// point if the queue has more than 24 hours contained within it.
        /// </summary>
        /// <param name="g">Graphics canvas</param>
        /// <param name="width">Width of the canvas</param>
        /// <param name="height">Height of the canvas</param>
        private void PaintQueue(Graphics g, int width, int height)
        {
            Brush[] brushes = new Brush[2];
            brushes[0] = new SolidBrush(GetFirstColor());
            brushes[1] = new SolidBrush(GetSecondColor());
            try
            {
                int brushNumber = 0;
                if (m_skillQueue == null)
                    return;

                int lastX = 0;
                foreach (QueuedSkill skill in m_skillQueue)
                {
                    // find the rectangle for the skill and paint it
                    Rectangle skillRect = GetSkillRect(skill, width, height);
                    g.FillRectangle(brushes[brushNumber], skillRect);
                    lastX = skillRect.Right;

                    // Rotate the brush
                    brushNumber = (brushNumber + 1) % brushes.Length;
                }

                // If there are more than 24 hours in the queue show the point
                if (m_skillQueue.EndTime > DateTime.UtcNow.AddHours(24))
                {
                    PaintPoint(g, width, height);
                }
                // Else, draw a dark region at the end and the border
                else
                {
                    // Empty region
                    var emptyRect = new Rectangle(lastX, 0, this.Width - lastX, this.Height);
                    using (var brush = new SolidBrush(GetEmptyColor()))
                    {
                        g.FillRectangle(brush, emptyRect);
                    }

                    // Then the border
                    using(Pen pen = new Pen(GetBorderColor(), 1.0f))
                    {
                        g.DrawRectangle(pen, 0, 0, width - 1, height - 1);
                    }
                }
            }
            finally
            {
                brushes[0].Dispose();
                brushes[1].Dispose();
            }
        }

        /// <summary>
        /// Gets the rectangle a skill rendes in within a specified rectange.
        /// </summary>
        /// <param name="skill">Skill that exists within the queue</param>
        /// <param name="width">Width of the canvas</param>
        /// <param name="height">Height of the canvas</param>
        /// <returns>
        /// Rectangle representing the area within the visual
        /// queue the skill occupies.
        /// </returns>
        internal static Rectangle GetSkillRect(QueuedSkill skill, int width, int height)
        {
            TimeSpan relativeStart;
            TimeSpan relativeFinish;

            // Character is training ? we update the timespan
            if (skill.Owner.IsTraining)
            {
                relativeStart = skill.StartTime.Subtract(DateTime.UtcNow);
                relativeFinish = skill.EndTime.Subtract(DateTime.UtcNow);

            }
            // Timespan is stable
            else
            {
                relativeStart = skill.StartTime.Subtract(m_paintTime);
                relativeFinish = skill.EndTime.Subtract(m_paintTime);
            }

            int TotalSeconds = (int)TimeSpan.FromHours(24).TotalSeconds;
            
            double Start = Math.Floor((relativeStart.TotalSeconds / TotalSeconds) * width);
            double Finish = Math.Floor((relativeFinish.TotalSeconds / TotalSeconds) * width);

            // If the start time is before now set it to zero
            if (Start < 0)
                Start = 0;

            return new Rectangle((int)Start, 0, (int)(Finish - Start), height);
        }

        private Point m_lastLocation = new Point(-1, -1);

        /// <summary>
        /// Triggers when the mouse is moved displays skill.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseMove(MouseEventArgs e)
        {
            if (m_skillQueue == null)
                return;

            // Prevent rapid triggering of event when the mouse hasn't moved
            if (e.Location == m_lastLocation)
                return;

            m_lastLocation = e.Location;

            int lastX = 0;
            foreach (QueuedSkill skill in m_skillQueue)
            {
                // Find the rectangle for the skill and paint it
                Rectangle skillRect = GetSkillRect(skill, this.Width, this.Height);
                lastX = skillRect.Right;

                if (skillRect.Contains(e.Location))
                {
                    Point tipPoint = new Point((Math.Min(skillRect.Right, this.Width) + skillRect.Left) / 2, e.Location.Y);
                    m_toolTip.Display(skill, tipPoint);
                    base.OnMouseMove(e);
                    return;
                }
            }

            // Are we in the empty space ?
            var emptyRect = new Rectangle(lastX, 0, this.Width - lastX, this.Height);
            if (emptyRect.Contains(e.Location))
            {
                var leftTime = (m_skillQueue.IsPaused ? m_paintTime : DateTime.UtcNow).AddHours(24) - m_skillQueue.EndTime;
                var text = String.Format(CultureConstants.DefaultCulture, "Free room:{0}", leftTime.ToDescriptiveText(DescriptiveTextOptions.SpaceBetween, false));
                Point tipPoint = new Point((emptyRect.Right + emptyRect.Left) / 2, e.Location.Y);
                m_toolTip.Display(text, tipPoint);
                base.OnMouseMove(e);
                return;
            }

            m_toolTip.Hide();
            base.OnMouseMove(e);
        }

        /// <summary>
        /// Hide tooltip when mouse moves out of skill queue.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseLeave(EventArgs e)
        {
            m_toolTip.Hide();
            base.OnMouseLeave(e);
        }

        /// <summary>
        /// Spit out a static skill queue for the designer.
        /// </summary>
        /// <param name="g"></param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        private void PaintDesignerQueue(Graphics g, int width, int height)
        {
            Brush lightBrush = new SolidBrush(GetFirstColor());
            Brush darkBrush = new SolidBrush(GetSecondColor());

            g.FillRectangle(lightBrush, new Rectangle(0, 0, (width / 5) * 2, height));
            g.FillRectangle(darkBrush, new Rectangle((width / 5) * 2, 0, (width / 5) * 2, height));
            g.FillRectangle(lightBrush, new Rectangle((width / 5) * 4, 0, width / 5, height));

            PaintPoint(g, width, height);
        }
        #endregion

    }
}
