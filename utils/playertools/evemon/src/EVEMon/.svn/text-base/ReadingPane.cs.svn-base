using System;
using System.Drawing;
using System.Windows.Forms;
using EVEMon.Common;

namespace EVEMon
{
    public partial class ReadingPane : UserControl
    {
        private IEveMessage m_selectedObject;

        /// <summary>
        /// Initializes a new instance of the <see cref="ReadingPane"/> class.
        /// </summary>
        public ReadingPane()
        {
            InitializeComponent();

            DoubleBuffered = true;

            lblMessageHeader.Font = FontFactory.GetDefaultFont(10F, FontStyle.Bold);
            lblSender.Font = FontFactory.GetDefaultFont(10F);
            flPanelHeader.ForeColor = SystemColors.ControlText;
        }

        /// <summary>
        /// Gets or sets the selected object.
        /// </summary>
        /// <value>The selected object.</value>
        internal IEveMessage SelectedObject
        {
            get { return m_selectedObject; }
            set
            {
                m_selectedObject = value;
                UpdatePane();
            }
        }

        /// <summary>
        /// Hides the reading pane.
        /// </summary>
        internal void HidePane()
        {
            flPanelHeader.Visible = false;
            wbMailBody.Visible = false;
        }

        /// <summary>
        /// Updates the reading pane.
        /// </summary>
        private void UpdatePane()
        {
            // Update the text on the header labels
            lblMessageHeader.Text = m_selectedObject.Title;
            lblSender.Text = m_selectedObject.Sender;
            lblSendDate.Text = String.Format(CultureConstants.DefaultCulture, "Sent: {0:ddd} {0:G}", m_selectedObject.SentDate.ToLocalTime());
            lblRecipient.Text = String.Format(CultureConstants.DefaultCulture, "To: {0}", string.Join(", ", m_selectedObject.Recipient));

            // Allows the text in the webbrowser to be displayed
            wbMailBody.AllowNavigation = true;

            // Parce the mail body text to the web browser
            // so for the text to be formatted accordingly
            wbMailBody.DocumentText = m_selectedObject.Text;

            // We need to wait for the Document to be loaded
            do
            {
                Application.DoEvents();
            } while (wbMailBody.ReadyState != WebBrowserReadyState.Complete);

            // Show the controls
            flPanelHeader.Visible = true;
            wbMailBody.Visible = true;       
        }

        /// <summary>
        /// Every time the mail header panel gets painted we add a line at the bottom.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.PaintEventArgs"/> instance containing the event data.</param>
        private void flPanelHeader_Paint(object sender, PaintEventArgs e)
        {
            // Calculate the height of the panel
            flPanelHeader.Height = lblMessageHeader.Height + lblSender.Height + lblSendDate.Height + lblRecipient.Height + 10;

            // Draw a line at the bottom of the panel
            using (Graphics g = flPanelHeader.CreateGraphics())
            {
                Pen blackPen = new Pen(Color.Black);
                g.DrawLine(blackPen, 5, flPanelHeader.Height - 1, flPanelHeader.Width - 5, flPanelHeader.Height - 1);
            }
        }

        /// <summary>
        /// Handles the Navigating event of the wbMailBody control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.WebBrowserNavigatingEventArgs"/> instance containing the event data.</param>
        private void wbMailBody_Navigating(object sender, WebBrowserNavigatingEventArgs e)
        {
            // Prevents the browser to navigate past the home page
            wbMailBody.AllowNavigation = false;
        }
    }
}
