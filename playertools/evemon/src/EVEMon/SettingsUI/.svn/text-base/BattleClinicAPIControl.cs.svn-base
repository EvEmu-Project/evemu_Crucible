using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Serialization.BattleClinic;
using EVEMon.Controls;

namespace EVEMon.SettingsUI
{
    public partial class BattleClinicAPIControl : UserControl
    {
        #region Constructor

        /// <summary>
        /// Initializes a new instance of the <see cref="BattleClinicAPIControl"/> class.
        /// </summary>
        public BattleClinicAPIControl()
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
        /// When BattleClinic API credentials get checked, informs the user.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.BCAPIEventArgs"/> instance containing the event data.</param>
        private void EveClient_BCAPICredentialsUpdated(object sender, BCAPIEventArgs e)
        {
            throbber.State = ThrobberState.Stopped;
            throbber.Visible = false;

            if (e.HasError)
            {
                apiResponseLabel.ForeColor = Color.Red;
                apiResponseLabel.Text = e.ErrorMessage;
                return;
            }

            apiResponseLabel.ForeColor = Color.Green;
            apiResponseLabel.Text = "Authenticated.";
        }
        
        #endregion


        #region Helper Methods

        /// <summary>
        /// Checks the BattleClinic API credentials.
        /// </summary>
        private void CheckAPICredentials()
        {
            uint bcUserID = Convert.ToUInt32(bcUserIDTextBox.Text);
            string bcAPIKey = bcAPIKeyTextBox.Text;

            BCAPI.CheckAPICredentials(bcUserID, bcAPIKey);

            throbber.Visible = true;
            throbber.State = ThrobberState.Rotating;
        }

        #endregion


        #region Local Events

        /// <summary>
        /// Occurs when the control loads.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void BattleClinicAPIControl_Load(object sender, EventArgs e)
        {
            EveClient.BCAPICredentialsUpdated += EveClient_BCAPICredentialsUpdated;
            Disposed += OnDisposed;

            if (!BCAPI.HasCredentialsStored)
                return;

            bcUserIDTextBox.Text = BCAPISettings.Default.BCUserID.ToString();
            bcAPIKeyTextBox.Text = BCAPISettings.Default.BCAPIKey;

            CheckAPICredentials();
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
        /// Opens a web page to the users BattleClinic API credentials.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.LinkLabelLinkClickedEventArgs"/> instance containing the event data.</param>
        private void bcAPILinkLabel_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Util.OpenURL(NetworkConstants.BCAPIAccountCredentials);
        }

        /// <summary>
        /// Resets the BattleClinic API credentials.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void resetButton_Click(object sender, EventArgs e)
        {
            apiResponseLabel.ForeColor = SystemColors.ControlText;
            apiResponseLabel.Text = String.Empty;

            bcUserIDTextBox.ResetText();
            bcAPIKeyTextBox.ResetText();
            BCAPISettings.Default.Reset();

            // Disables the settingsFileStorageControl
            Control battleClinicServicePage = Parent.Parent;
            Control settingsFileStorageGroupBox = battleClinicServicePage.Controls["settingsFileStorageGroupBox"];
            Control settingsFileStorageControl = settingsFileStorageGroupBox.Controls["settingsFileStorageControl"];

            settingsFileStorageControl.Enabled = BCAPI.HasCredentialsStored;
        }

        /// <summary>
        /// Applies the BattleClinic API credentials.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void applyButton_Click(object sender, EventArgs e)
        {
            apiResponseLabel.ForeColor = SystemColors.ControlText;
            apiResponseLabel.Text = String.Empty;

            if (!ValidateChildren())
                return;

            CheckAPICredentials();
        }

        /// <summary>
        /// Checks the validity of the BattleClinic UserID.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.ComponentModel.CancelEventArgs"/> instance containing the event data.</param>
        private void bcUserIDTextBox_Validating(object sender, CancelEventArgs e)
        {
            if (String.IsNullOrEmpty(bcUserIDTextBox.Text))
            {
                errorProvider.SetError(bcUserIDTextBox, "UserID cannot be blank.");
                e.Cancel = true;
                return;
            }

            if (bcUserIDTextBox.TextLength == 1 && bcUserIDTextBox.Text == "0")
            {
                errorProvider.SetError(bcUserIDTextBox, "UserID must not be zero.");
                e.Cancel = true;
                return;
            }

            if (bcUserIDTextBox.Text.StartsWith("0"))
            {
                errorProvider.SetError(bcUserIDTextBox, "UserID must not start with zero.");
                e.Cancel = true;
                return;
            }

            for (int i = 0; i < bcUserIDTextBox.TextLength; i++)
            {
                if (!Char.IsDigit(bcUserIDTextBox.Text[i]))
                {
                    errorProvider.SetError(bcUserIDTextBox, "UserID must be numerical.");
                    e.Cancel = true;
                    break;
                }
            }
        }

        /// <summary>
        /// Occurs when the BattleClinic UserID gets validated.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void bcUserIDTextBox_Validated(object sender, EventArgs e)
        {
            errorProvider.SetError(bcUserIDTextBox, String.Empty);
        }

        /// <summary>
        /// Checks the validity of the BattleClinic APIKey.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.ComponentModel.CancelEventArgs"/> instance containing the event data.</param>
        private void bcAPIKeyTextBox_Validating(object sender, CancelEventArgs e)
        {
            string apiKey = bcAPIKeyTextBox.Text.Trim();
            if (!String.IsNullOrEmpty(apiKey))
                return;

            errorProvider.SetError(bcAPIKeyTextBox, "APIKey cannot be blank.");
            e.Cancel = true;
        }

        /// <summary>
        /// Occurs when the BattleClinic APIKey gets validated.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void bcAPIKeyTextBox_Validated(object sender, EventArgs e)
        {
            errorProvider.SetError(bcAPIKeyTextBox, String.Empty);
        }

        #endregion
    }
}
