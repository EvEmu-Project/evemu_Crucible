using System;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Controls;

using CommonProperties = EVEMon.Common.Properties;

namespace EVEMon.Accounting
{
    public partial class AccountUpdateOrAdditionWindow : EVEMonForm
    {
        private readonly bool m_updateMode;
        private AccountCreationEventArgs m_creationArgs;
        private Account m_account;

        /// <summary>
        /// Constructor for a new account creation window.
        /// </summary>
        public AccountUpdateOrAdditionWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Constructor for editing an existing account.
        /// </summary>
        /// <param name="account"></param>
        public AccountUpdateOrAdditionWindow(Account account)
            : this()
        {
            m_account = account;
            m_updateMode = (account != null);
        }

        /// <summary>
        /// Update the controls visibility depending on whether we are in update or creation mode.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            if (DesignMode)
                return;

            // Update controls depending on the update mode
            removalWarningLabel.Visible = m_updateMode;
            apiKeyTextBox.Text = (m_account != null ? m_account.APIKey : String.Empty);
            userIDTextBox.Text = (m_account != null ? m_account.UserID.ToString() : String.Empty);
            userIDTextBox.ReadOnly = m_updateMode;
            charactersListView.Items.Clear();

            multiPanel.SelectedPage = credentialsPage;
            multiPanel.SelectionChange += multiPanel_SelectionChange;
        }

        /// <summary>
        /// When we switch panels, we update the "next", "previous" and "cancel" buttons.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        void multiPanel_SelectionChange(object sender, MultiPanelSelectionChangeEventArgs args)
        {
            if (args.NewPage == credentialsPage)
            {
                previousButton.Enabled = false;
                nextButton.Enabled = true;
                nextButton.Text = "&Next >";
            }
            else if (args.NewPage == waitingPage)
            {
                previousButton.Enabled = true;
                nextButton.Enabled = false;
                nextButton.Text = "&Next >";
            }
            else
            {
                nextButton.Enabled = !errorGuideLabel.Visible;
                previousButton.Enabled = true;
                nextButton.Text = (m_updateMode ? "Update" : "Import");
            }
        }

        /// <summary>
        /// Previous.
        /// When the previous button is clicked, we select the first page.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void previousButton_Click(object sender, EventArgs e)
        {
            multiPanel.SelectedPage = credentialsPage;
        }

        /// <summary>
        /// Cancel.
        /// Closes the window.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cancelButton_Click(object sender, EventArgs e)
        {
            m_creationArgs = null;
            Close();
        }

        /// <summary>
        /// Next / Import / Update.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nextButton_Click(object sender, EventArgs e)
        {
            // Are we jumping from page 1/3 ?
            if (multiPanel.SelectedPage == credentialsPage)
            {
                // If the args have not been invalidated since the last time...
                if (m_creationArgs != null)
                {
                    multiPanel.SelectedPage = resultPage;
                    return;
                }

                // Are we updating existing account ?
                multiPanel.SelectedPage = waitingPage;
                throbber.State = EVEMon.Controls.ThrobberState.Rotating;
                if (m_account != null)
                {
                    m_account.TryUpdateAsync(apiKeyTextBox.Text, OnUpdated);
                }
                // Or creating a new one ?
                else
                {
                    Int64 userID;
                    Int64.TryParse(userIDTextBox.Text, out userID);
                    EveClient.Accounts.TryAddOrUpdateAsync(userID, apiKeyTextBox.Text, OnUpdated);
                }
                return;
            }

            // So we're at the end.
            Complete();
        }

        /// <summary>
        /// When an account's credentials have been updated.
        /// </summary>
        /// <returns></returns>
        private void OnUpdated(Object sender, AccountCreationEventArgs args)
        {
            m_creationArgs = args;

            // Updates the picture and label for key level
            switch (args.KeyLevel)
            {
                default:
                    keyPicture.Image = CommonProperties.Resources.APIKeyWrong;
                    keyLabel.Text = m_creationArgs.Result.ErrorMessage;
                    errorGuideLabel.Visible = true;
                    errorGuideLabel.BringToFront();
                    break;
                case CredentialsLevel.Limited:
                    keyPicture.Image = CommonProperties.Resources.APIKeyLimited;
                    keyLabel.Text = "This is a limited API key.";
                    errorGuideLabel.Visible = false;
                    break;
                case CredentialsLevel.Full:
                    keyPicture.Image = CommonProperties.Resources.APIKeyFull;
                    keyLabel.Text = "This is a full API key.";
                    errorGuideLabel.Visible = false;
                    break;
            }

            // Updates the characters list
            charactersListView.Items.Clear();
            foreach (var id in args.Identities)
            {
                var item = new ListViewItem(id.Name);
                item.Checked = (m_account == null || !m_account.IgnoreList.Contains(id));
                item.Tag = id;

                charactersListView.Items.Add(item);
            }

            // Selects the last page
            throbber.State = ThrobberState.Stopped;
            multiPanel.SelectedPage = resultPage;
        }

        /// <summary>
        /// Validates the operation and closes the window.
        /// </summary>
        private void Complete()
        {
            if (m_creationArgs == null)
                return;
            m_account = m_creationArgs.CreateOrUpdate();

            // Takes care of the ignore list
            foreach (ListViewItem item in charactersListView.Items)
            {
                var id = (CharacterIdentity)item.Tag;
                if (item.Checked)
                {
                    m_account.IgnoreList.Remove(id);
                }
                else
                {
                    var ccpCharacter = id.CCPCharacter;
                    if (ccpCharacter != null)
                        m_account.IgnoreList.Add(ccpCharacter);
                }
            }

            // Closes the window
            Close();
        }

        /// <summary>
        /// On the first page, when a textbox is changed, we ensure the previously generated <see cref="AccountCreationEventArgs"/> is destroyed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void userIDTextBox_TextChanged(object sender, EventArgs e)
        {
            m_creationArgs = null;
        }

        /// <summary>
        /// On the first page, when a textbox is changed, we ensure the previously generated <see cref="AccountCreationEventArgs"/> is destroyed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void apiKeyTextBox_TextChanged(object sender, EventArgs e)
        {
            m_creationArgs = null;
        }

        /// <summary>
        /// First page, link for the CCP page for API credentials.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ccpLinkLabel_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Util.OpenURL(NetworkConstants.APIAccountCredentials);
        }

        /// <summary>
        /// First page, link to the features window.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void featuresLinkLabel_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            WindowsFactory<FeaturesWindow>.ShowUnique();
        }
    }
}
