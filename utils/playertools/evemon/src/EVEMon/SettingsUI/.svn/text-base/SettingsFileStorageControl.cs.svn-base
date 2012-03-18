using System;
using System.Drawing;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Serialization.BattleClinic;
using EVEMon.Controls;

namespace EVEMon.SettingsUI
{
    public partial class SettingsFileStorageControl : UserControl
    {
        private bool m_queryPending;

        #region Constructor

        /// <summary>
        /// Initializes a new instance of the <see cref="SettingsFileStorageControl"/> class.
        /// </summary>
        public SettingsFileStorageControl()
        {
            InitializeComponent();
            Font = FontFactory.GetFont("Tahoma", FontStyle.Regular);
            apiResponseLabel.Font = FontFactory.GetFont("Tahoma", FontStyle.Bold);
            apiResponseLabel.Text = String.Empty;
            throbber.Visible = false;
            throbber.BringToFront();
        }

        #endregion


        #region Global Events

        /// <summary>
        /// Occurs when the BattleClinic API credentials get authenticated.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.BCAPIEventArgs"/> instance containing the event data.</param>
        private void EveClient_BCAPICredentialsUpdated(object sender, BCAPIEventArgs e)
        {
            Enabled = BCAPI.IsAuthenticated;
        }

        #endregion


        #region Local Events

        /// <summary>
        /// Occurs when the control loads.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void SettingsFileStorageControl_Load(object sender, EventArgs e)
        {
            EveClient.BCAPICredentialsUpdated += EveClient_BCAPICredentialsUpdated;
            Disposed += OnDisposed;

            alwaysUploadCheckBox.Checked = BCAPISettings.Default.UploadAlways;
            alwaysDownloadcheckBox.Checked = BCAPISettings.Default.DownloadAlways;
            Enabled = false;
        }

        /// <summary>
        /// Occurs when the control gets disposed.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.BCAPICredentialsUpdated -= EveClient_BCAPICredentialsUpdated;
            Disposed -= OnDisposed;
        }

        /// <summary>
        /// Occurs when the checkbox state changes.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void alwaysUploadCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            BCAPISettings.Default.UploadAlways = alwaysUploadCheckBox.Checked;
            BCAPISettings.Default.Save();
        }

        /// <summary>
        /// Occurs when the checkbox state changes.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void alwaysDownloadcheckBox_CheckedChanged(object sender, EventArgs e)
        {
            BCAPISettings.Default.DownloadAlways = alwaysDownloadcheckBox.Checked;
            BCAPISettings.Default.Save();
        }

        /// <summary>
        /// Occurs when clicking the "Upload settings file" button.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void uploadSettingsFileButton_Click(object sender, EventArgs e)
        {
            apiResponseLabel.ForeColor = SystemColors.ControlText;
            apiResponseLabel.Text = String.Empty;

            if (m_queryPending)
                return;

            m_queryPending = true;
            throbber.Visible = true;
            throbber.State = ThrobberState.Rotating;

            Settings.SaveImmediate();

            BCAPI.FileSaveAsync(OnFileSave);
        }

        /// <summary>
        /// Occurs when clicking the "Download settings file" button.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void downloadSettingsFileButton_Click(object sender, EventArgs e)
        {
            apiResponseLabel.ForeColor = SystemColors.ControlText;
            apiResponseLabel.Text = String.Empty;

            if (m_queryPending)
                return;

            m_queryPending = true;
            throbber.Visible = true;
            throbber.State = ThrobberState.Rotating;

            BCAPI.FileGetByNameAsync(OnFileGetByName);
        }

        #endregion


        #region Querying helpers

        /// <summary>
        /// On settings file uploaded we inform the user.
        /// </summary>
        /// <param name="result">The result.</param>
        /// <param name="errorMessage">The error message.</param>
        private void OnFileSave(BCAPIResult<SerializableBCAPIFiles> result, string errorMessage)
        {
            m_queryPending = false;

            throbber.State = ThrobberState.Stopped;
            throbber.Visible = false;
            apiResponseLabel.ForeColor = Color.Red;

            if (!String.IsNullOrEmpty(errorMessage))
            {
                apiResponseLabel.Text = errorMessage;
                return;
            }

            if (result.HasError)
            {
                apiResponseLabel.Text = result.Error.ErrorMessage;
                return;
            }

            apiResponseLabel.ForeColor = Color.Green;
            apiResponseLabel.Text = "File uploaded successfully.";
        }

        /// <summary>
        /// On settings file downloaded we inform the user.
        /// </summary>
        /// <param name="result">The result.</param>
        /// <param name="errorMessage">The error message.</param>
        private void OnFileGetByName(BCAPIResult<SerializableBCAPIFiles> result, string errorMessage)
        {
            m_queryPending = false;

            throbber.State = ThrobberState.Stopped;
            throbber.Visible = false;
            apiResponseLabel.ForeColor = Color.Red;

            if (!String.IsNullOrEmpty(errorMessage))
            {
                apiResponseLabel.Text = errorMessage;
                return;
            }

            if (result.HasError)
            {
                apiResponseLabel.Text = result.Error.ErrorMessage;
                return;
            }

            BCAPI.SaveSettingsFile(result.Result.Files[0]);
        }

        #endregion
    }
}
