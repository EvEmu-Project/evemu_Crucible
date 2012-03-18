using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Drawing.Text;

using EVEMon.Common;

using lgLcdClassLibrary;

namespace EVEMon.LogitechG15
{
    public class Lcdisplay : IDisposable
    {
        internal static int G15Width = (int)LCDInterface.LGLCD_BMP_WIDTH;
        internal static int G15Height = (int)LCDInterface.LGLCD_BMP_HEIGHT;
        internal const float G15DpiX = 46;
        internal const float G15DpiY = 46;

        private static Lcdisplay s_singleInstance;

        private Font m_defaultFont;
        private Bitmap m_bmpLCD;
        private Bitmap m_bmpLCDX;
        private Graphics m_lcdCanvas;
        private Graphics m_lcdOverlay;
        private int m_currentCharacterIndex;
        private int m_oldButtonState;
        private DateTime m_buttonStateHld;
        private DateTime m_paintTime;
        private DateTime m_holdTime;
        private LcdState m_state;
        private DateTime m_cycleTime;
        private DateTime m_cycleQueueInfoTime;
        private bool m_showingCycledQueueInfo;
        private int m_completedSkills;
        private bool m_disposed;
        private ButtonDelegate m_buttonDelegate;

        private List<LineProcess> m_lcdLines = new List<LineProcess>();


        #region Delegates and Events

        public delegate void CharacterHandler(Character character);
        public delegate void CharAutoCycleHandler(bool cycle);

        /// <summary>
        /// Fired whenever a button has been pressed which require EVEMon to requery the API for the specified character.
        /// </summary>
        public static event CharacterHandler APIUpdateRequested;

        /// <summary>
        /// Fired whenever the current character changed (because of a button press).
        /// </summary>
        public static event CharacterHandler CurrentCharacterChanged;

        /// <summary>
        /// Fired whenever the auto cycle should change (because of a button press).
        /// </summary>
        public static event CharAutoCycleHandler AutoCycleChanged;
        
        #endregion


        #region Instantiation

        /// <summary>
        /// Initializes a new instance of the <see cref="Lcdisplay"/> class.
        /// </summary>
        private Lcdisplay()
        {
            m_defaultFont = FontFactory.GetFont("Microsoft Sans Serif", 13.5f, FontStyle.Regular, GraphicsUnit.Point);

            m_bmpLCD = new Bitmap(G15Width, G15Height, PixelFormat.Format24bppRgb);
            m_bmpLCD.SetResolution(G15DpiX, G15DpiY);
            m_lcdCanvas = Graphics.FromImage(m_bmpLCD);
            m_lcdCanvas.TextRenderingHint = TextRenderingHint.SingleBitPerPixelGridFit;

            m_bmpLCDX = new Bitmap(G15Width, G15Height, PixelFormat.Format24bppRgb);
            m_bmpLCDX.SetResolution(G15DpiX, G15DpiY);
            m_lcdOverlay = Graphics.FromImage(m_bmpLCDX);
            m_lcdOverlay.TextRenderingHint = TextRenderingHint.SingleBitPerPixelGridFit;

            m_buttonStateHld = DateTime.Now;
            m_cycleTime = DateTime.Now.AddSeconds(10);
            m_cycleQueueInfoTime = DateTime.Now.AddSeconds(5);
            m_showingCycledQueueInfo = false;

            Cycle = false;
            ShowSystemTime = false;
            CycleSkillQueueTime = false;
            FirstSkillCompletionRemaingTime = TimeSpan.FromTicks(DateTime.Now.Ticks);

            m_buttonDelegate = new ButtonDelegate(OnButtonsPressed);
            LCDInterface.AssignButtonDelegate(m_buttonDelegate);
            LCDInterface.Open("EVEMon", false);
        }

        /// <summary>
        /// Instances this instance.
        /// </summary>
        /// <returns></returns>
        public static Lcdisplay Instance()
        {
            if (s_singleInstance == null)
                s_singleInstance = new Lcdisplay();

            return s_singleInstance;
        }
        
        #endregion


        #region Properties

        public bool Cycle { get; set; }

        public int CycleInterval { get; set; }

        public bool ShowSystemTime { get; set; }

        public bool ShowEVETime { get; set; }

        public bool CycleSkillQueueTime { get; set; }

        public int CycleCompletionInterval { get; set; }

        public Character RefreshCharacter { get; set; }

        public Character FirstCharacterToCompleteSkill { get; set; }

        public TimeSpan FirstSkillCompletionRemaingTime { get; set; }

        public CCPCharacter[] Characters { get; set; }

        public int CurrentCharacterIndex
        {
            get { return m_currentCharacterIndex; }
        }

        public string CurrentCharacterName
        {
            get
            {
                if (m_currentCharacterIndex >= Characters.Length)
                    m_currentCharacterIndex = Characters.Length - 1;

                if (m_currentCharacterIndex < 0 || Characters == null)
                    return "No CCP Character";

                return Characters[m_currentCharacterIndex].AdornedName;
            }
        }

        public CCPCharacter CurrentCharacter
        {
            get
            {
                if (m_currentCharacterIndex < 0 || Characters == null || m_currentCharacterIndex >= Characters.Length)
                    return null;

                return Characters[m_currentCharacterIndex];
            }
            set
            {
                m_currentCharacterIndex = Array.IndexOf(Characters, value);

                if (m_currentCharacterIndex == -1)
                    m_currentCharacterIndex = 0;
            }
        }
        
        #endregion


        #region Cleanup

        /// <summary>
        /// Releases unmanaged resources and performs other cleanup operations before the
        /// <see cref="Lcdisplay"/> is reclaimed by garbage collection.
        /// </summary>
        ~Lcdisplay()
        {
            Dispose(false);
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases unmanaged and - optionally - managed resources
        /// </summary>
        /// <param name="isDisposing">
        /// <c>true</c> to release both managed and unmanaged resources;
        /// <c>false</c> to release only unmanaged resources.
        /// </param>
        private void Dispose(bool isDisposing)
        {
            if (!m_disposed)
            {
                if (isDisposing || s_singleInstance != null)
                    LCDInterface.Close();

                s_singleInstance = null;
            }
            m_disposed = true;
        }

        #endregion


        #region Painting

        /// <summary>
        /// Performs the repainting of the screen.
        /// </summary>
        public void Paint()
        {
            TimeSpan test = TimeSpan.FromTicks(m_paintTime.Ticks - DateTime.Now.Ticks);
            if (test.TotalMilliseconds > 0)
                return;

            TimeSpan now = TimeSpan.FromTicks(DateTime.Now.Ticks - m_holdTime.Ticks);
            if (m_state == LcdState.SplashScreen && now.TotalSeconds > 4)
                SwitchState(LcdState.Character);

            if (m_state == LcdState.CharacterList && now.TotalMilliseconds > 2000)
                SwitchState(LcdState.Character);

            if (m_state == LcdState.SkillComplete && now.TotalSeconds > 14)
                SwitchState(LcdState.Character);

            if ((m_state == LcdState.CycleSettings || m_state == LcdState.Refreshing) && now.TotalSeconds > 2)
                SwitchState(LcdState.Character);

            switch (m_state)
            {
                case LcdState.SplashScreen:
                    m_paintTime = m_paintTime.AddSeconds(2);
                    PaintSplash();
                    return;
                case LcdState.Character:
                    if (Cycle)
                    {
                        if (TimeSpan.FromTicks(DateTime.Now.Ticks - m_cycleTime.Ticks).TotalSeconds > CycleInterval)
                        {
                            m_cycleTime = DateTime.Now;
                            MoveToNextChar();

                            // When moving to next character
                            // we reset the queue info timer
                            m_cycleQueueInfoTime = DateTime.Now;
                            m_showingCycledQueueInfo = false;
                        }
                    }
                    m_paintTime = m_paintTime.AddSeconds(1);
                    PaintsCharacter();
                    return;
                case LcdState.CharacterList:
                    m_paintTime = m_paintTime.AddSeconds(2);
                    PaintCharactersList();
                    return;
                case LcdState.SkillComplete:
                    m_paintTime = m_paintTime.AddMilliseconds(800);
                    PaintSkillCompletionMessage();
                    return;
                case LcdState.CycleSettings:
                    m_paintTime = m_paintTime.AddSeconds(2);
                    PaintCycleSettings();
                    return;
                case LcdState.Refreshing:
                    if (RefreshCharacter != null)
                    {
                        m_paintTime = m_paintTime.AddSeconds(2);
                        PaintRefreshingMessage();
                    }
                    return;
                default:
                    return;
            }
        }

        /// <summary>
        /// Paints the current character's training informations, this is the regular painting operation.
        /// </summary>
        private void PaintsCharacter()
        {
            m_lcdLines.Clear();

            if (CurrentCharacter == null)
            {
                m_lcdLines.Add(new LineProcess("No CCP Characters To Display", m_defaultFont));
                RenderLines();
                UpdateLcdisplay();
                return;
            }

            if (CycleSkillQueueTime)
            {
                if (TimeSpan.FromTicks(DateTime.Now.Ticks - m_cycleQueueInfoTime.Ticks).TotalSeconds > CycleCompletionInterval)
                {
                    m_cycleQueueInfoTime = DateTime.Now;
                    m_showingCycledQueueInfo = !m_showingCycledQueueInfo;
                }
            }

            m_lcdLines.Add(new LineProcess(CurrentCharacter.AdornedName, m_defaultFont));

            var skill = CurrentCharacter.SkillQueue.CurrentlyTraining;
            if (CurrentCharacter.SkillQueue.IsTraining)
            {
                var skillQueueEndTime = CurrentCharacter.SkillQueue.EndTime;
                if (m_showingCycledQueueInfo)
                {
                    bool freeTime = skillQueueEndTime < DateTime.UtcNow.AddHours(24);

                    if (freeTime)
                    {
                        // Place holder for skill queue free room rendering
                        m_lcdLines.Add(new LineProcess(" ", m_defaultFont));
                    }
                    else if (CurrentCharacter.SkillQueue.Count > 1)
                    {
                        // If more then one skill is in queue, show queue finish time
                        var time = skillQueueEndTime.Subtract(DateTime.UtcNow).ToDescriptiveText(
                            DescriptiveTextOptions.SpaceBetween, true);
                        m_lcdLines.Add(new LineProcess(
                            String.Format(CultureConstants.DefaultCulture, "Queue finishes in: {0}", time), m_defaultFont));
                    }
                    else
                    {
                        // Show the skill in training
                        m_lcdLines.Add(new LineProcess(skill.ToString(), m_defaultFont));
                    }
                }
                else
                {
                    // Show the skill in training
                    m_lcdLines.Add(new LineProcess(skill.ToString(), m_defaultFont));
                }

                m_lcdLines.Add(new LineProcess(skill.EndTime.Subtract(DateTime.UtcNow).ToDescriptiveText(
                    DescriptiveTextOptions.SpaceBetween, true).TrimStart(' '), m_defaultFont));
            }
            else
            {
                if (CurrentCharacter.SkillQueue.IsPaused)
                {
                    m_lcdLines.Add(new LineProcess(skill.ToString(), m_defaultFont));
                    m_lcdLines.Add(new LineProcess("Skill Training Is Paused", m_defaultFont));
                }
                else
                {
                    m_lcdLines.Add(new LineProcess("No Skill In Training", m_defaultFont));
                    m_lcdLines.Add(new LineProcess("Skill Queue Is Empty", m_defaultFont));
                }
            }

            m_lcdLines.Add(new LineProcess((skill != null ? skill.FractionCompleted : 0), m_defaultFont));

            RenderLines();
            RenderWalletBalance();
            RenderSkillQueueInfo();
            RenderCompletionTime();
            RenderEVETime();
            RenderSystemTime();

            UpdateLcdisplay();
        }

        /// <summary>
        /// Renders the lines.
        /// </summary>
        private void RenderLines()
        {
            ClearGraphics();

            float offset = 0;

            foreach (var lcdLine in m_lcdLines)
            {
                lcdLine.Render(m_lcdCanvas, m_lcdOverlay, offset);
                offset += lcdLine.Height;
            }
        }

        /// <summary>
        /// Renders the wallet balance.
        /// </summary>
        private void RenderWalletBalance()
        {
            decimal balance = CurrentCharacter.Balance;
            string walletBalance = String.Format(CultureConstants.DefaultCulture, "{0} ISK", balance.ToString("#,##0.#0"));
            SizeF size = m_lcdCanvas.MeasureString(walletBalance, m_defaultFont);
            SizeF charNameSize = m_lcdCanvas.MeasureString(CurrentCharacter.AdornedName, m_defaultFont);
            float availableWidth = (G15Width - charNameSize.Width);

            if (availableWidth < size.Width)
            {
                walletBalance = AbbreviationFormat(balance, availableWidth);
                size = m_lcdCanvas.MeasureString(walletBalance, m_defaultFont);
            }

            RectangleF line = new RectangleF(new PointF(G15Width - size.Width, 0f), size);
            line.Offset(0f, 0f);
            m_lcdCanvas.DrawString(walletBalance, m_defaultFont, new SolidBrush(Color.Black), line);
        }

        /// <summary>
        /// Renders the skill queue info.
        /// </summary>
        private void RenderSkillQueueInfo()
        {
            bool freeTime = CurrentCharacter.SkillQueue.EndTime < DateTime.UtcNow.AddHours(24);

            if (CurrentCharacter.IsTraining && m_showingCycledQueueInfo && freeTime)
                UpdateSkillQueueFreeRoom();
        }

        /// <summary>
        /// Renders the completion time.
        /// </summary>
        private void RenderCompletionTime()
        {
            if (CurrentCharacter.IsTraining)
            {
                DateTime completionDateTime = CurrentCharacter.CurrentlyTrainingSkill.EndTime.ToLocalTime();
                string completionDateTimeText = String.Format(CultureConstants.DefaultCulture, "{0}  {1}",
                completionDateTime.ToShortDateString(), completionDateTime.ToCustomShortTimeString());
                SizeF size = m_lcdCanvas.MeasureString(completionDateTimeText, m_defaultFont);
                RectangleF timeLine = new RectangleF(new PointF(G15Width - size.Width, 0f), size);
                timeLine.Offset(0f, 22f);
                m_lcdCanvas.DrawString(completionDateTimeText, m_defaultFont, new SolidBrush(Color.Black), timeLine);
            }
        }

        /// <summary>
        /// Renders the EVE time.
        /// </summary>
        private void RenderEVETime()
        {
            if (!ShowEVETime)
                return;

            string curEVETime = DateTime.UtcNow.ToString("HH:mm");
            SizeF size = m_lcdCanvas.MeasureString(curEVETime, m_defaultFont);
            RectangleF timeLine = new RectangleF(new PointF(0f, 0f), size);
            timeLine.Offset(0f, 32f);
            m_lcdCanvas.DrawString(curEVETime, m_defaultFont, new SolidBrush(Color.Black), timeLine);
        }

        /// <summary>
        /// Renders the system time.
        /// </summary>
        private void RenderSystemTime()
        {
            if (!ShowSystemTime)
                return;

            string curTime = DateTime.Now.ToCustomShortTimeString();
            SizeF size = m_lcdCanvas.MeasureString(curTime, m_defaultFont);
            RectangleF timeLine = new RectangleF(new PointF(G15Width - size.Width, 0f), size);
            timeLine.Offset(0f, 32f);
            m_lcdCanvas.DrawString(curTime, m_defaultFont, new SolidBrush(Color.Black), timeLine);
        }

        /// <summary>
        /// Paints a message for skill completion.
        /// </summary>
        private void PaintSkillCompletionMessage()
        {
            if (CurrentCharacter.SkillQueue.LastCompleted == null)
                return;

            m_lcdLines.Clear();

            if (CurrentCharacter == null)
            {
                m_lcdLines.Add(new LineProcess("No CCP Characters To Display", m_defaultFont));
                RenderLines();
                UpdateLcdisplay();
                return;
            }

            m_lcdLines.Add(new LineProcess(CurrentCharacter.AdornedName, m_defaultFont));
            m_lcdLines.Add(new LineProcess("has finished training", m_defaultFont));

            if (m_completedSkills > 1)
            {
                m_lcdLines.Add(new LineProcess(String.Format(CultureConstants.DefaultCulture, "{0} skills", m_completedSkills), m_defaultFont));
            }
            else
            {
                m_lcdLines.Add(new LineProcess(CurrentCharacter.SkillQueue.LastCompleted.ToString(), m_defaultFont));
            }

            var skillCount = CurrentCharacter.SkillQueue.Count;

            if (skillCount == 0)
            {
                m_lcdLines.Add(new LineProcess("NO SKILLS IN QUEUE", m_defaultFont));
            }
            else
            {
                m_lcdLines.Add(new LineProcess(String.Format(CultureConstants.DefaultCulture, "{0} more skill{1} in queue", skillCount, skillCount == 1 ? String.Empty : "s"), m_defaultFont));
            }

            RenderLines();
            UpdateLcdDisplay(LCDInterface.LGLCD_PRIORITY_ALERT);
        }

        /// <summary>
        /// Paints the characters list.
        /// </summary>
        private void PaintCharactersList()
        {
            m_lcdLines.Clear();

            if (Characters.Length == 0)
            {
                m_lcdLines.Add(new LineProcess("No CCP Characters To Display", m_defaultFont));
                RenderLines();
                UpdateLcdisplay();
                return;
            }

            // Creates a reordered list with the selected character on top
            List<CCPCharacter> charList = new List<CCPCharacter>();

            for (int i = m_currentCharacterIndex; i < Characters.Length; i++)
            {
                charList.Add(Characters[i]);
            }
            for (int i = 0; i < m_currentCharacterIndex; i++)
            {
                charList.Add(Characters[i]);
            }

            // Perform the painting
            ClearGraphics();

            foreach (var character in charList)
            {
                m_lcdLines.Add(new LineProcess(character.AdornedName, m_defaultFont));
            }

            RenderLines();
            RenderSelector();
            UpdateLcdisplay();
        }

        /// <summary>
        /// Renders the selector.
        /// </summary>
        private void RenderSelector()
        {
            m_lcdOverlay.FillRectangle(new SolidBrush(Color.Black), 0, 0, G15Width, 11);
        }

        /// <summary>
        /// Paints the cycling settings on the screen.
        /// </summary>
        private void PaintCycleSettings()
        {
            ClearGraphics();
            m_lcdLines.Clear();

            string status = Cycle ? "on" : "off";
            string statusMsg = String.Format(CultureConstants.DefaultCulture, "Autocycle is now {0}", status);
            m_lcdLines.Add(new LineProcess(statusMsg, m_defaultFont));

            string cycleMsg = String.Format(CultureConstants.DefaultCulture, "Cycle Time is: {0}s", CycleInterval);
            m_lcdLines.Add(new LineProcess(cycleMsg, m_defaultFont));

            RenderLines();
            UpdateLcdisplay();
        }

        /// <summary>
        /// Paints a waiting message while we're updating the characters.
        /// </summary>
        private void PaintRefreshingMessage()
        {
            ClearGraphics();
            m_lcdLines.Clear();

            m_lcdLines.Add(new LineProcess("Refreshing Character Information", m_defaultFont));
            m_lcdLines.Add(new LineProcess("of", m_defaultFont));
            m_lcdLines.Add(new LineProcess(RefreshCharacter.AdornedName, m_defaultFont));

            RenderLines();
            UpdateLcdisplay();
        }

        /// <summary>
        /// Paints the EVEMon icon at the initialization of the LCD screen.
        /// </summary>
        private void PaintSplash()
        {
            // Load the icon
            Bitmap splashLogo = new Bitmap(Properties.Resources.LCDSplash);

            // Clear the graphics
            ClearGraphics();

            // Display the splash logo
            int left = (int)((G15Width / 2) - (splashLogo.Width / 2));
            int top = (int)((G15Height / 2) - (splashLogo.Height / 2));
            m_lcdCanvas.DrawImage(splashLogo, new Rectangle(left, top, splashLogo.Width, splashLogo.Height));
            UpdateLcdDisplay(LCDInterface.LGLCD_PRIORITY_ALERT);
        }

        /// <summary>
        /// Clears the graphics.
        /// </summary>
        private void ClearGraphics()
        {
            m_lcdCanvas.Clear(Color.White);
            m_lcdOverlay.Clear(Color.White);
        }

        /// <summary>
        /// Updates the skill queue free room info.
        /// </summary>
        private void UpdateSkillQueueFreeRoom()
        {
            TimeSpan timeLeft = DateTime.UtcNow.AddHours(24) - CurrentCharacter.SkillQueue.EndTime;
            string timeLeftText;

            // Prevents the "(none)" text from being displayed
            if (timeLeft < TimeSpan.FromSeconds(1))
                return;

            // Less than minute ? Display seconds
            if (timeLeft < TimeSpan.FromMinutes(1))
            {
                timeLeftText = timeLeft.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);
            }
            // Display time without seconds
            else
            {
                timeLeftText = timeLeft.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas, false);
            }

            string skillQueueFreemRoom = String.Format(CultureConstants.DefaultCulture, "{0} free room in skill queue", timeLeftText);
            SizeF size = m_lcdCanvas.MeasureString(skillQueueFreemRoom, m_defaultFont);
            RectangleF line = new RectangleF(new PointF(0f, 0f), size);
            line.Offset(0f, 11f);
            m_lcdCanvas.FillRectangle(new SolidBrush(Color.Black), 0, 13, 160, 10);
            m_lcdOverlay.DrawString(skillQueueFreemRoom, m_defaultFont, new SolidBrush(Color.Black), line);
        }

        /// <summary>
        /// Fetches the content of the <see cref="Graphics"/> object to the G15 screen with a <see cref="LcdisplayPriority.Normal"/> priority.
        /// </summary>
        private void UpdateLcdisplay()
        {
            UpdateLcdDisplay(LCDInterface.LGLCD_PRIORITY_NORMAL);
        }

        /// <summary>
        /// Fetches the content of the <see cref="Graphics"/> object to the G15 screen.
        /// </summary>
        /// <param name="priority"></param>
        private unsafe void UpdateLcdDisplay(uint priority)
        {
            // Locking should not be necesseray but i'll keep it here
            lock (m_bmpLCD)
            {
                int width = m_bmpLCD.Width;
                int height = m_bmpLCD.Height;
                byte[] buffer = new byte[width * height];
                Rectangle rect = new Rectangle(0, 0, width, height);

                BitmapData bmData = m_bmpLCD.LockBits(rect, ImageLockMode.ReadOnly, m_bmpLCD.PixelFormat);
                try
                {
                    BitmapData bmDataX = m_bmpLCDX.LockBits(rect, ImageLockMode.ReadOnly, m_bmpLCDX.PixelFormat);
                    try
                    {
                        // Extract bits per pixel and length infos
                        int stride = bmData.Stride;
                        int bpp = stride / width;
                        int length = stride * height;

                        // Copy the content of the bitmap to our buffers 
                        // Unsafe code removes the boundaries checks - a lot faster.
                        fixed (byte* buffer0 = buffer)
                        {
                            byte* output = buffer0;
                            byte* inputX = (byte*)bmDataX.Scan0.ToPointer();
                            byte* input = (byte*)bmData.Scan0.ToPointer(); ;

                            for (int i = 0; i < height; i++)
                            {
                                for (int j = 0; j < width; j++)
                                {
                                    *output = (byte)((*input) ^ (*inputX));
                                    inputX += bpp;
                                    input += bpp;
                                    output++;
                                }
                            }
                        }
                    }
                    finally
                    {
                        m_bmpLCDX.UnlockBits(bmDataX);
                    }
                }
                finally
                {
                    m_bmpLCD.UnlockBits(bmData);
                }

                // Fetches the buffer to the LCD screen
                LCDInterface.DisplayBitmap(ref buffer, priority);
            }
        }
        #endregion


        #region Helper Methods

        /// <summary>
        /// Formats the wallet balance value in an abbreviation form
        /// </summary>
        /// <param name="value"></param>
        /// <param name="width"></param>
        /// <returns>Abbriaviated balance value</returns>
        private string AbbreviationFormat(decimal value, float width)
        {
            string balance;
            int suffixIndex = 0;
            float newWidth;

            do
            {
                value /= 1000;
                suffixIndex++;

                switch (suffixIndex)
                {
                    case 1:
                        balance = String.Format(CultureConstants.DefaultCulture, "{0} K ISK", value.ToString("#,###.#0"));
                        break;
                    case 2:
                        balance = String.Format(CultureConstants.DefaultCulture, "{0} M ISK", value.ToString("#,###.#0"));
                        break;
                    case 3:
                        balance = String.Format(CultureConstants.DefaultCulture, "{0} B ISK", value.ToString("#,###.#0"));
                        break;
                    // We have no room to show the wallet balance
                    default:
                        balance = String.Empty;
                        break;
                }

                SizeF size = m_lcdCanvas.MeasureString(balance, m_defaultFont);
                newWidth = size.Width;
            }
            while (width < newWidth);

            return balance;
        }

        #endregion


        #region Controlling logic

        /// <summary>
        /// Occurs when some of the G15 screen buttons are pressed.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="dwButtons"></param>
        /// <param name="pContext"></param>
        /// <returns></returns>
        private int OnButtonsPressed(int device, int dwButtons, IntPtr pContext)
         {
            if (m_oldButtonState == dwButtons)
                return 0;

            // Gets all buttons who haven't been pressed last time
            int press = (m_oldButtonState ^ dwButtons) & dwButtons;

            // Displays the characters' list or move to the next char if the list is already displayed.
            if ((press & LCDInterface.LGLCDBUTTON_BUTTON0) != 0)
                DisplayCharactersList();

            // Move to the first character to complete his training
            if ((press & LCDInterface.LGLCDBUTTON_BUTTON1) != 0)
            {
                // Select next skill ready char
                if (Characters == null)
                    return 0;

                CurrentCharacter = FirstCharacterToCompleteSkill as CCPCharacter;

                if (AutoCycleChanged != null)
                    AutoCycleChanged(false);

                SwitchState(LcdState.Character);
            }

            // Forces a refresh from CCP
            if ((press & LCDInterface.LGLCDBUTTON_BUTTON2) != 0)
            {
                if (m_state == LcdState.Character || m_state == LcdState.CharacterList)
                {
                    RefreshCharacter = CurrentCharacter;
                    if (APIUpdateRequested != null)
                        APIUpdateRequested(RefreshCharacter);

                    SwitchState(LcdState.Refreshing);
                }
            }

            // Switch autocycle ON/OFF
            if ((press & LCDInterface.LGLCDBUTTON_BUTTON3) != 0)
            {
                // Switch autocycle on/off
                SwitchCycle();

                if (AutoCycleChanged != null)
                    AutoCycleChanged(Cycle);

                SwitchState(LcdState.CycleSettings);
                m_cycleTime = DateTime.Now;
            }

            m_oldButtonState = dwButtons;
            return 0;
        }

        /// <summary>
        /// Moves the selection to the next character.
        /// </summary>
        /// <param name="nextchar"></param>
        private void MoveToNextChar()
        {
            if (Characters == null)
                return;

            // Move to next char
            m_currentCharacterIndex++;
            if (m_currentCharacterIndex >= Characters.Length)
                m_currentCharacterIndex = 0;

            // Requests new data
            if (CurrentCharacterChanged != null)
                CurrentCharacterChanged(CurrentCharacter);
        }

        /// <summary>
        /// Switches the state and updates some of the internal times variables.
        /// </summary>
        /// <param name="state"></param>
        public void SwitchState(LcdState newState)
        {
            m_state = newState;
            m_paintTime = DateTime.Now;
            m_holdTime = DateTime.Now;
        }

        /// <summary>
        /// Updates the characters' list. First call displays the list, the second one moves the selection
        /// </summary>
        public void DisplayCharactersList()
        {
            TimeSpan now = TimeSpan.FromTicks(DateTime.Now.Ticks - m_buttonStateHld.Ticks);
            m_buttonStateHld = DateTime.Now;

            if (now.TotalMilliseconds < 2000)
            {
                Cycle = false;
                MoveToNextChar();
            }

            SwitchState(LcdState.CharacterList);
        }

        /// <summary>
        /// On skill completion, switch to the display of the proper message.
        /// </summary>
        public void SkillCompleted(Character character)
        {
            CurrentCharacter = character as CCPCharacter;
            SwitchState(LcdState.SkillComplete);
        }

        /// <summary>
        /// On skill completion, switch to the display of the proper message.
        /// </summary>
        public void SkillCompleted(Character character, int skillCount)
        {
            CurrentCharacter = character as CCPCharacter;
            m_completedSkills = skillCount;
            SwitchState(LcdState.SkillComplete);
        }

        /// <summary>
        /// Switches the cycling setting.
        /// </summary>
        public void SwitchCycle()
        {
            Cycle = !Cycle;
        }

        #endregion
    }
}
