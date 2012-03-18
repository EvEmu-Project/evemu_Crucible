using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common.Controls;
using EVEMon.Common;

namespace EVEMon.Accounting
{
    public partial class AccountDeletionWindow : EVEMonForm
    {
        private readonly Account m_account;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="account"></param>
        public AccountDeletionWindow(Account account)
        {
            InitializeComponent();
            m_account = account;

            // Add characters
            charactersListView.Items.Clear();
            foreach (var id in account.CharacterIdentities)
            {
                // Skip if there is no CCP character for this identity.
                var ccpCharacter = id.CCPCharacter;
                if (ccpCharacter == null) continue;

                // Add an item for this character
                var item = new ListViewItem(ccpCharacter.Name);
                item.Tag = ccpCharacter;
                item.Checked = true;
                charactersListView.Items.Add(item);
            }
        }
        
        /// <summary>
        /// "Delete" button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void validationButton_Click(object sender, EventArgs e)
        {
            // Remove the account
            EveClient.Accounts.Remove(m_account);

            // Remove the characters
            foreach (ListViewItem item in charactersListView.Items)
            {
                if (item.Checked)
                {
                    var ccpCharacter = item.Tag as CCPCharacter;
                    EveClient.Characters.Remove(ccpCharacter);
                }
            }

            // Closes the window
            this.Close();
        }
    }
}
