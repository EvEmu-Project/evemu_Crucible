using System;
using System.ComponentModel;
using System.Drawing;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

using EVEMon.APIErrorHandling;
using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.Net;
using EVEMon.Common.Notifications;
using EVEMon.Common.Serialization.API;

namespace EVEMon.ApiErrorHandling
{
    /// <summary>
    /// Displays an error window if appropriate a troubleshooter is displayed to help the user resolve the issue.
    /// </summary>
    public partial class APIErrorWindow : EVEMonForm
    {
        private APIErrorNotification m_notification;
        private ApiErrorTroubleshooter m_troubleshooter;
        private bool m_troubleshooterUsed;

        /// <summary>
        /// Constructor
        /// </summary>
        public APIErrorWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Gets or sets the notification for this error.
        /// </summary>
        [Browsable(false)]
        public APIErrorNotification Notification
        {
            get { return m_notification; }
            set
            {
                m_notification = value;
                ErrorLabel.Text = GetErrorLabelText(value);
                DetailsTextBox.Text = GetXmlData(value.Result);
                DisplayTroubleshooter(value.Result.Exception);
            }
        }

        /// <summary>
        /// Displays the troubleshooter.
        /// </summary>
        /// <param name="exception">The exception.</param>
        private void DisplayTroubleshooter(Exception exception)
        {
            TroubleshooterPanel.Visible = false;

            m_troubleshooter = GetTroubleshooter(exception);

            if (m_troubleshooter == null)
                return;

            TroubleshooterPanel.Visible = true;
            TroubleshooterPanel.Controls.Add(m_troubleshooter);

            m_troubleshooter.ErrorResolved += troubleshooter_ErrorResolved;
            m_troubleshooter.Dock = DockStyle.Fill;
        }

        /// <summary>
        /// Gets the troubleshooter.
        /// </summary>
        /// <param name="exception">The exception.</param>
        /// <returns>A troubleshooter for the error message.</returns>
        private static ApiErrorTroubleshooter GetTroubleshooter(Exception exception)
        {
            if (exception == null)
                return null;

            var httpException = exception as HttpWebServiceException;

            if (httpException == null)
                return null;

            if (httpException.Status == HttpWebServiceExceptionStatus.Timeout)
            {
                var troubleshooter = new HttpTimeoutTroubleshooter();
                return troubleshooter;
            }

            return null;
        }

        /// <summary>
        /// Handles the ErrorResolved event when a http timeout is displayed of the troubleshooter control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.Controls.ApiErrorTroubleshooterEventArgs"/> instance containing the event data.</param>
        private void troubleshooter_ErrorResolved(object sender, ApiErrorTroubleshooterEventArgs e)
        {
            m_troubleshooterUsed = true;

            if (e == null)
                return;

            if (!e.Resolved)
            {
                TroubleshooterPanel.BackColor = Color.DarkSalmon;
                return;
            }

            EveClient.Notifications.Invalidate(new NotificationInvalidationEventArgs(m_notification));
            PerformAction(e.Action);
        }

        /// <summary>
        /// Performs the action.
        /// </summary>
        /// <param name="action">The action.</param>
        private void PerformAction(ResolutionAction action)
        {
            switch (action)
            {
                case ResolutionAction.Close:
                    Close();
                    break;
                case ResolutionAction.HideTroubleshooter:
                    TroubleshooterPanel.Hide();
                    break;
                case ResolutionAction.None:
                    TroubleshooterPanel.BackColor = Color.PaleGreen;
                    break;
                default:
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        /// Gets the error label text.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns>String representing the error.</returns>
        private static string GetErrorLabelText(APIErrorNotification value)
        {
            if (value == null)
                return "No error selected.";

            if (value.Result == null)
                return String.Format("{0}{1}No details were provided.", value, Environment.NewLine);

            return String.Format("{0}{1}{2}", value, Environment.NewLine, GetErrorLabelTextDetail(value.Result));
        }

        /// <summary>
        /// Gets the error label text detail.
        /// </summary>
        /// <param name="value">The value.</param>
        private static string GetErrorLabelTextDetail(IAPIResult result)
        {
            switch (result.ErrorType)
            {
                case APIEnumerations.APIErrors.None:
                    return "No error specified";

                case APIEnumerations.APIErrors.CCP:
                    return String.Format(CultureConstants.DefaultCulture,
                        "CCP Error {0} : {1}",
                        result.CCPError.ErrorCode,
                        result.CCPError.ErrorMessage);

                case APIEnumerations.APIErrors.Http:
                    return String.Format(CultureConstants.DefaultCulture,
                        "HTTP error: {0}", result.ErrorMessage);

                case APIEnumerations.APIErrors.Xml:
                    return String.Format(CultureConstants.DefaultCulture,
                        "XML error: {0}", result.ErrorMessage);

                case APIEnumerations.APIErrors.Xslt:
                    return String.Format(CultureConstants.DefaultCulture,
                        "XSLT error: {0}", result.ErrorMessage);

                default:
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        /// Gets the XML data from the result.
        /// </summary>
        /// <param name="result">The result.</param>
        /// <returns></returns>
        private static string GetXmlData(IAPIResult result)
        {
            if (result == null || result.XmlDocument == null)
                return "There was no associated XML document.";

            return Util.GetXMLStringRepresentation(result.XmlDocument);
        }

        /// <summary>
        /// On closing, disposes of the troubleshooter.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            if (m_troubleshooter != null)
            {
                m_troubleshooter.Dispose();
                m_troubleshooter = null;
            }            

            base.OnFormClosed(e);
        }

        /// <summary>
        /// Handles the LinkClicked event of the CopyToClipboardLinkLabel control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.LinkLabelLinkClickedEventArgs"/> instance containing the event data.</param>
        private void CopyToClipboardLinkLabel_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            var version = Assembly.GetExecutingAssembly().GetName().Version;
            var builder = new StringBuilder();

            builder.AppendFormat("EVEMon {0} API Error:{1}", version, Environment.NewLine);
            builder.AppendLine();
            builder.AppendLine(GetErrorLabelText(Notification));
            builder.AppendLine();
            builder.AppendLine(GetXmlData(Notification.Result));

            if (m_troubleshooter != null)
            {
                builder.AppendLine();
                if (m_troubleshooterUsed)
                {
                    builder.Append("A troubleshooter was displayed and used.");
                }
                else
                {
                    builder.Append("A troubleshooter was displayed but not used.");
                }
            }

            try
            {
                Clipboard.Clear();
                Clipboard.SetText(builder.ToString(), TextDataFormat.Text);
            }
            catch (ExternalException ex)
            {
                // Occurs when another process is using the clipboard
                ExceptionHandler.LogException(ex, true);
                MessageBox.Show("Couldn't complete the operation, the clipboard is being used by another process. Wait a few moments and try again.");
            }
        }
    }
}
