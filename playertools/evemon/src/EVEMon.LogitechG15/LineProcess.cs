using System;
using System.Drawing;

using EVEMon.Common;

namespace EVEMon.LogitechG15
{
    public class LineProcess
    {
        private double m_percentage;
        private string m_text;
        private Font m_font;

        #region Constructor
        /// <summary>
        /// Initializes a new instance of the <see cref="LineProcess"/> class.
        /// </summary>
        /// <param name="font">The font.</param>
        public LineProcess(Font font)
        {
            m_font = font;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="LineProcess"/> class.
        /// </summary>
        /// <param name="percentage">The percentage.</param>
        /// <param name="font">The font.</param>
        public LineProcess(double percentage, Font font)
            : this(font)
        {
            m_percentage = percentage;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="LineProcess"/> class.
        /// </summary>
        /// <param name="text">The text.</param>
        /// <param name="font">The font.</param>
        public LineProcess(string text, Font font)
            : this(font)
        {
            m_text = text;
        }
        #endregion


        #region Methods
        /// <summary>
        /// Gets or sets the height.
        /// </summary>
        /// <value>The height.</value>
        internal float Height
        {
            get;
            private set;
        }

        /// <summary>
        /// Renders the specified canvas.
        /// </summary>
        /// <param name="canvas">The canvas.</param>
        /// <param name="overlay">The overlay.</param>
        /// <param name="offset">The offset.</param>
        internal void Render(Graphics canvas, Graphics overlay, float offset)
        {
            if (m_text == null)
            {
                RenderProgressLine(canvas, overlay, offset);
            }
            else
            {
                RenderTextLine(canvas, overlay, offset);
            }
        }

        /// <summary>
        /// Renders the text line.
        /// </summary>
        /// <param name="canvas">The canvas.</param>
        /// <param name="overlay">The overlay.</param>
        /// <param name="offset">The offset.</param>
        private void RenderTextLine(Graphics canvas, Graphics overlay, float offset)
        {
            RectangleF lineRect = new RectangleF(new PointF(0f, offset), canvas.MeasureString(m_text, m_font));
            canvas.DrawString(m_text, m_font, new SolidBrush(Color.Black), lineRect);
            Height = lineRect.Height;
        }

        /// <summary>
        /// Renders the progress line.
        /// </summary>
        /// <param name="canvas">The canvas.</param>
        /// <param name="overlay">The overlay.</param>
        /// <param name="offset">The offset.</param>
        private void RenderProgressLine(Graphics canvas, Graphics overlay, float offset)
        {
            string text = m_percentage.ToString("P2");
            SizeF textSize = canvas.MeasureString(text, m_font);
            int left = 0;
            int size = Lcdisplay.G15Width;
            int pad = 2;

            if (Settings.G15.ShowEVETime)
            {
                string EVETime = DateTime.UtcNow.ToString("HH:mm");
                SizeF EVETimeSize = canvas.MeasureString(EVETime, m_font);
                left = (int)EVETimeSize.Width + pad;
                size = Lcdisplay.G15Width - left - pad;
            }

            if (Settings.G15.ShowSystemTime)
            {
                string SystemTime = DateTime.Now.ToCustomShortTimeString();
                SizeF SystemTimeSize = canvas.MeasureString(SystemTime, m_font);

                size = size - (int)SystemTimeSize.Width;
            }


            RectangleF barRect = new RectangleF(new PointF(left, offset + 1), new SizeF(size - 1, textSize.Height - 2));
            float textLeft = (barRect.Width / 2) - (textSize.Width / 2);
            RectangleF textRect = new RectangleF(new PointF(left + textLeft, offset), canvas.MeasureString(text, m_font));

            int barFill = Convert.ToInt16(m_percentage * size - 2);

            canvas.DrawRectangle(new Pen(Color.Black), barRect.Left, barRect.Top, barRect.Width, barRect.Height);
            canvas.FillRectangle(new SolidBrush(Color.Black), barRect.Left + 1, barRect.Top + 1, barFill, barRect.Height - 2);
            overlay.DrawString(text, m_font, new SolidBrush(Color.Black), textRect);
            Height = barRect.Height + 1;
        }
        #endregion
    }
}
