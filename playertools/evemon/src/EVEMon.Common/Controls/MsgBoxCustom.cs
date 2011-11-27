using System;
using System.Drawing;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace EVEMon.Common.Controls
{
    /// <summary>
    /// Class for a message box with integrated checkbox.
    /// </summary>
    public partial class MessageBoxCustom : Form
    {
        private DialogResult m_dialogResult;
        public bool cbUnchecked = true;

        /// <summary>
        /// Creates a new instance of <see cref="MessageBoxCustom"/>.
        /// </summary>
        public MessageBoxCustom()
        {
            InitializeComponent();

            msgText.Font = FontFactory.GetFont("Segoe UI", 9f, FontStyle.Regular, GraphicsUnit.Point);
            cbOption.Font = FontFactory.GetFont("Segoe UI", 9f, FontStyle.Regular, GraphicsUnit.Point);
            button1.Font = FontFactory.GetFont("Segoe UI", 9f, FontStyle.Regular, GraphicsUnit.Point);
            button2.Font = FontFactory.GetFont("Segoe UI", 9f, FontStyle.Regular, GraphicsUnit.Point);
            button3.Font = FontFactory.GetFont("Segoe UI", 9f, FontStyle.Regular, GraphicsUnit.Point);
        }


        /// <summary>
        /// Displays a message box.
        /// </summary>
        /// <param name="owner">Owner window.</param>
        /// <param name="text">Text to display.</param>
        /// <param name="caption">Text to display in the title bar.</param>
        /// <param name="cdText">Text to display near check box.</param>
        /// <param name="buttons">Buttons to display in the message box.</param>
        /// <param name="icon">Icon to display in the mesage box.</param>
        /// <returns>One of the <see cref="DialogResult"/> values.</returns>
        public DialogResult Show(IWin32Window owner, string text, string caption, string cdText, MessageBoxButtons buttons, MessageBoxIcon icon)
        {
            button1.Click += new EventHandler(OnButtonClick);
            button2.Click += new EventHandler(OnButtonClick);
            button3.Click += new EventHandler(OnButtonClick);

            this.Text = caption;
            msgText.Text = text;
            cbOption.Text = cdText;

            if (Environment.OSVersion.Platform == PlatformID.Win32NT)
            {
                NativeMethods.EnableMenuItem(NativeMethods.GetSystemMenu(this.Handle, false), 
                    NativeMethods.SC_CLOSE, NativeMethods.MF_BYCOMMAND | NativeMethods.MF_GRAYED);
            }
            else this.ControlBox = false;

            SetButtonsToDisplay(buttons);

            SetIconToDisplay(icon);
            
            MessageBeep(icon);

            ShowDialog(owner);

            return m_dialogResult;
        }

        /// <summary>
        /// Displays a message box.
        /// </summary>
        /// <param name="owner">Owner window.</param>
        /// <param name="text">Text to display.</param>
        /// <param name="caption">Text to display in the title bar.</param>
        /// <param name="cdText">Text to display near check box.</param>
        /// <param name="buttons">Buttons to display in the message box.</param>
        /// <returns>One of the <see cref="DialogResult"/> values.</returns>
        public DialogResult Show(IWin32Window owner, string text, string caption, string cbText, MessageBoxButtons buttons)
        {
            return Show(owner, text, caption, cbText, buttons, MessageBoxIcon.None);
        }

        /// <summary>
        /// Displays a message box.
        /// </summary>
        /// <param name="owner">Owner window.</param>
        /// <param name="text">Text to display.</param>
        /// <param name="caption">Text to display in the title bar.</param>
        /// <param name="cdText">Text to display near check box.</param>
        /// <returns>One of the <see cref="DialogResult"/> values.</returns>
        public DialogResult Show(IWin32Window owner, string text, string caption, string cbText)
        {
            return Show(owner, text, caption, cbText, MessageBoxButtons.OK, MessageBoxIcon.None);
        }

        /// <summary>
        /// Displays a message box.
        /// </summary>
        /// <param name="owner">Owner window.</param>
        /// <param name="text">Text to display.</param>
        /// <param name="caption">Text to display in the title bar.</param>
        /// <returns>One of the <see cref="DialogResult"/> values.</returns>
        public DialogResult Show(IWin32Window owner, string text, string caption)
        {
            return Show(owner, text, caption, String.Empty, MessageBoxButtons.OK, MessageBoxIcon.None);
        }

        /// <summary>
        /// Displays a message box.
        /// </summary>
        /// <param name="owner">Owner window.</param>
        /// <param name="text">Text to display.</param>
        /// <returns>One of the <see cref="DialogResult"/> values.</returns>
        public DialogResult Show(IWin32Window owner, string text)
        {
            return Show(owner, text, String.Empty, String.Empty, MessageBoxButtons.OK, MessageBoxIcon.None);
        }

        # region EventHandlers

        private void OnButtonClick(object sender, EventArgs e)
        {
            string buttonText = ((ButtonBase)sender).Text;
            m_dialogResult = GetDialogResult(buttonText);
            this.Close();
        }

        private void cbOption_CheckedChanged(object sender, EventArgs e)
        {
            cbUnchecked = !cbOption.Checked;
        }

        # endregion

        # region Help Methods

        /// <summary>
        /// Sets buttons for the message box.
        /// </summary>
        /// <param name="icon">Which buttons to display.</param>
        private void SetButtonsToDisplay(MessageBoxButtons buttons)
        {
            switch (buttons)
            {
                case (MessageBoxButtons.AbortRetryIgnore):
                    button3.Text = "Ignore";
                    button2.Text = "Retry";
                    button1.Text = "Abort";
                    break;
                case (MessageBoxButtons.OK):
                    button3.Visible = false;
                    button2.Visible = false;
                    button1.Text = "OK";
                    AcceptButton = button1;
                    break;
                case (MessageBoxButtons.OKCancel):
                    button3.Visible = false;
                    button2.Text = "OK";
                    button1.Text = "Cancel";
                    break;
                case (MessageBoxButtons.RetryCancel):
                    button3.Visible = false;
                    button2.Text = "Retry";
                    button1.Text = "Cancel";
                    break;
                case (MessageBoxButtons.YesNo):
                    button3.Visible = false;
                    button2.Text = "Yes";
                    button1.Text = "No";
                    break;
                case (MessageBoxButtons.YesNoCancel):
                    button3.Text = "Yes";
                    button2.Text = "No";
                    button1.Text = "Cancel";
                    AcceptButton = button3;
                    break;
            }
        }

        /// <summary>
        /// Sets icon for the message box.
        /// </summary>
        /// <param name="icon">Icon type.</param>
        private void SetIconToDisplay(MessageBoxIcon icon)
        {
            switch (icon.GetHashCode())
            {
                case 0:
                    break;
                case 16:
                    msgIcon.Image = SystemIcons.Hand.ToBitmap();
                    break;
                case 32:
                    msgIcon.Image = SystemIcons.Question.ToBitmap();
                    break;
                case 48:
                    msgIcon.Image = SystemIcons.Exclamation.ToBitmap();
                    break;
                case 64:
                    msgIcon.Image = SystemIcons.Asterisk.ToBitmap();
                    break;
            }
        }

        /// <summary>
        /// Plays one of the system message beeps.
        /// </summary>
        /// <param name="icon">Sound type to play.</param>
        private void MessageBeep(MessageBoxIcon icon)
        {
            switch (icon.GetHashCode())
            {
                
                 case 0:
                    System.Media.SystemSounds.Beep.Play();
                    break;
               
                case 16:
                    System.Media.SystemSounds.Hand.Play();
                    break;

                case 32:
                    System.Media.SystemSounds.Question.Play();
                    break;

                case 48:
                    System.Media.SystemSounds.Exclamation.Play();
                    break;
                
                case 64:
                    System.Media.SystemSounds.Asterisk.Play();
                    break;

            }
        }

        /// <summary>
        /// Returns dialog result based on button text.
        /// </summary>
        /// <param name="buttonText">Text on selected button.</param>
        /// <returns>Corresponding <see cref="DialogResult"/>.</returns>
        private DialogResult GetDialogResult(string buttonText)
        {
            switch (buttonText)
            {
                case "Abort":
                    return DialogResult.Abort;
                case "Cancel":
                    return DialogResult.Cancel;
                case "Ignore":
                    return DialogResult.Ignore;
                case "No":
                    return DialogResult.No;
                case "OK":
                    return DialogResult.OK;
                case "Retry":
                    return DialogResult.Retry;
                case "Yes":
                    return DialogResult.Yes;
                default:
                    return DialogResult.None;
            }
        }

        # endregion

        #region Native Stuff

        internal class NativeMethods
        {
            public const int SC_CLOSE = 0xF060;
            public const int MF_BYCOMMAND = 0x0;
            public const int MF_GRAYED = 0x1;
            public const int MF_ENABLED = 0x0;

            [DllImport("user32.dll", CharSet = CharSet.Auto)]
            public static extern IntPtr GetSystemMenu(IntPtr hWnd, bool bRevert); 

            [DllImport("user32.dll", CharSet = CharSet.Auto)]
            public static extern bool EnableMenuItem(IntPtr hMenu, uint uIDEnableItem, uint uEnable); 

        }

        #endregion
    }
}