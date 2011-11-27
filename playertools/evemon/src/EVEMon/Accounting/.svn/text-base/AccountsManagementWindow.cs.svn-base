using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Controls;
using EVEMon.Controls;

namespace EVEMon.Accounting
{
    public partial class AccountsManagementWindow : EVEMonForm
    {
        private readonly Color m_readonlyColor = Color.WhiteSmoke;
        private int m_refreshingCharactersCounter;

        /// <summary>
        /// Constructor
        /// </summary>
        public AccountsManagementWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// On loading, intialize the controls and subscribe events.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            if (this.DesignMode)
                return;

            accountsListBox.Font = FontFactory.GetFont("Tahoma", 9.75f);
            accountsListBox.SelectedIndexChanged += accountsListBox_SelectedIndexChanged;
            accountsListBox.DoubleClick += accountsListBox_DoubleClick;
            accountsListBox.KeyDown += accountsListBox_KeyDown;

            charactersListView.Font = FontFactory.GetFont("Tahoma", 9.75f);
            charactersListView.SelectedIndexChanged += charactersListView_SelectedIndexChanged;
            charactersListView.DoubleClick += charactersListView_DoubleClick;
            charactersListView.KeyDown += charactersListView_KeyDown;
            charactersListView.ItemChecked += charactersListView_ItemChecked;

            EveClient.AccountCollectionChanged += EveClient_AccountCollectionChanged;
            EveClient.CharacterCollectionChanged += EveClient_CharacterCollectionChanged;
            EveClient.CharacterChanged += EveClient_CharacterChanged;

            EveClient_AccountCollectionChanged(null, null);
            EveClient_CharacterCollectionChanged(null, null);
            AdjustColumns();

            // Selects the second page if no account known so far.
            if (EveClient.Characters.Count == 0) tabControl.SelectedIndex = 1;
        }

        /// <summary>
        /// On closing, unsubscribe events.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosing(CancelEventArgs e)
        {
            EveClient.AccountCollectionChanged -= EveClient_AccountCollectionChanged;
            EveClient.CharacterCollectionChanged -= EveClient_CharacterCollectionChanged;
            EveClient.CharacterChanged -= EveClient_CharacterChanged;
            base.OnClosing(e);
        }

        /// <summary>
        /// When the size changes, we adjust the characters' columns.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnSizeChanged(EventArgs e)
        {
            if (charactersListView != null)
                AdjustColumns();

            base.OnSizeChanged(e);
        }

        #region Accounts management
        /// <summary>
        /// When the accounts collection changes, we update the content.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_AccountCollectionChanged(object sender, EventArgs e)
        {
            accountsListBox.Accounts = EveClient.Accounts;
            accountsMultiPanel.SelectedPage = (EveClient.Accounts.IsEmpty() ? noAccountsPage : accountsListPage);
        }

        /// <summary>
        /// When the selection changes, we update the controls.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void accountsListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            deleteAccountMenu.Enabled = (accountsListBox.SelectedIndex != -1);
            editAccountMenu.Enabled = (accountsListBox.SelectedIndex != -1);
        }

        /// <summary>
        /// On double click, forces the edition
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void accountsListBox_DoubleClick(object sender, EventArgs e)
        {
            // Search for the double-clicked item
            int index = 0;
            var point = Cursor.Position;
            point = accountsListBox.PointToClient(point);
            foreach (var account in accountsListBox.Accounts)
            {
                var rect = accountsListBox.GetItemRectangle(index);
                index++;

                if (rect.Contains(point))
                {
                    // Open the edition window
                    using (var window = new AccountUpdateOrAdditionWindow(account))
                    {
                        window.ShowDialog(this);
                        return;
                    }
                }
            }
        }

        /// <summary>
        /// Accounts toolbar > Edit
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void editAccountMenu_Click(object sender, EventArgs e)
        {
            if (accountsListBox.SelectedIndex == -1)
                return;

            var account = accountsListBox.Accounts.ElementAt(accountsListBox.SelectedIndex);
            using (var window = new AccountUpdateOrAdditionWindow(account))
            {
                window.ShowDialog(this);
                return;
            }
        }

        /// <summary>
        /// Accounts toolbar > Add
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void addAccountMenu_Click(object sender, EventArgs e)
        {
            using (var window = new AccountUpdateOrAdditionWindow())
            {
                window.ShowDialog(this);
                return;
            }
        }

        /// <summary>
        /// Accounts toolbar > Delete
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void deleteAccountMenu_Click(object sender, EventArgs e)
        {
            if (accountsListBox.SelectedIndex == -1) return;
            var account = accountsListBox.Accounts.ElementAt(accountsListBox.SelectedIndex);
            using (AccountDeletionWindow window = new AccountDeletionWindow(account))
            {
                window.ShowDialog(this);
            }
            deleteAccountMenu.Enabled = (accountsListBox.SelectedIndex != -1);
            editAccountMenu.Enabled = (accountsListBox.SelectedIndex != -1);
        }

        /// <summary>
        /// Handle "delete" for the accounts.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void accountsListBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Delete)
            {
                deleteAccountMenu_Click(sender, e);
            }
        }
        #endregion


        #region Characters management 
        /// <summary>
        /// When the characters collection changed, we update the characters list.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterCollectionChanged(object sender, EventArgs e)
        {
            // Begin the update
            m_refreshingCharactersCounter++;

            // Update the list view item
            UpdateCharactersListContent();

            // Invalidates the accounts list.
            accountsListBox.Invalidate();

            // Make a help message appears when no accounts exist.
            if (EveClient.Characters.Count == 0)
            {
                charactersMultiPanel.SelectedPage = noCharactersPage;
            }
            else
            {
                charactersMultiPanel.SelectedPage = charactersListPage;
            }

            // End of the update
            m_refreshingCharactersCounter--;
        }

        /// <summary>
        /// When the character changes, the displayed names changes too.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            m_refreshingCharactersCounter++;
            UpdateCharactersListContent();
            m_refreshingCharactersCounter--;
        }

        /// <summary>
        /// Recreate the items in the characters listview
        /// </summary>
        private void UpdateCharactersListContent()
        {
            charactersListView.BeginUpdate();
            try
            {
                // Retrieve current selection and grouping option
                List<Character> oldSelection = new List<Character>(charactersListView.SelectedItems.Cast<ListViewItem>().Select(x => x.Tag as Character));
                charactersListView.Groups.Clear();
                charactersListView.Items.Clear();
                
                // Grouping (no account, account #1, account #2, character files, character urls)
                bool isGrouping = groupingMenu.Checked;
                ListViewGroup noAccountsGroup = new ListViewGroup("No account");
                ListViewGroup fileGroup = new ListViewGroup("Character files");
                ListViewGroup urlGroup = new ListViewGroup("Character urls");
                var accountGroups = new Dictionary<Account, ListViewGroup>();

                if (isGrouping)
                {
                    bool hasNoAccount = false;
                    bool hasFileChars = false;
                    bool hasUrlChars = false;

                    // Scroll through listview items to gather the groups
                    foreach (var character in EveClient.Characters)
                    {
                        var uriCharacter = character as UriCharacter;

                        // Uri character ?
                        if (uriCharacter != null)
                        {
                            if (uriCharacter.Uri.IsFile)
                            {
                                hasFileChars = true;
                            }
                            else
                            {
                                hasUrlChars = true;
                            }
                        }
                        // CCP character ?
                        else
                        {
                            var account = character.Identity.Account;
                            if (account == null)
                            {
                                hasNoAccount = true;
                            }
                            else if (!accountGroups.ContainsKey(account))
                            {
                                accountGroups.Add(account, new ListViewGroup(String.Format("Account #{0}", account.UserID)));
                            }
                        }
                    }

                    // Add the groups
                    if (hasNoAccount)
                        charactersListView.Groups.Add(noAccountsGroup);

                    foreach (var group in accountGroups.Values)
                    {
                        charactersListView.Groups.Add(group);
                    }

                    if (hasFileChars)
                        charactersListView.Groups.Add(fileGroup);

                    if (hasUrlChars)
                        charactersListView.Groups.Add(urlGroup);
                }
                
                // Add items
                foreach (var character in EveClient.Characters.OrderBy(x => x.Name))
                {
                    var item = new ListViewItem();
                    item.Checked = character.Monitored;
                    item.Tag = character;

                    // Retrieve the texts for the different columns.
                    Account account = character.Identity.Account;
                    string accountText = (account == null ? String.Empty : account.UserID.ToString());
                    string typeText = "CCP";
                    string uriText = "-";

                    if (character is UriCharacter)
                    {
                        var uriCharacter = (UriCharacter)character;
                        typeText = (uriCharacter.Uri.IsFile ? "File" : "Url");
                        uriText = uriCharacter.Uri.ToString();

                        if (isGrouping)
                            item.Group = (uriCharacter.Uri.IsFile ? fileGroup : urlGroup);
                    }
                    // Grouping CCP characters
                    else if (isGrouping)
                    {
                        item.Group = (account == null ? noAccountsGroup : accountGroups[account]);
                    }

                    // Add the item and its subitems
                    item.SubItems.Add(new ListViewItem.ListViewSubItem(item, typeText));
                    item.SubItems.Add(new ListViewItem.ListViewSubItem(item, character.Name));
                    item.SubItems.Add(new ListViewItem.ListViewSubItem(item, accountText));
                    item.SubItems.Add(new ListViewItem.ListViewSubItem(item, uriText));

                    charactersListView.Items.Add(item);
                    if (oldSelection.Contains(character))
                        item.Selected = true;
                }
            }
            finally
            {
                charactersListView.EndUpdate();
            }

            // Forces a refresh of the enabled/disabled items
            charactersListView_SelectedIndexChanged(null, null);
        }

        /// <summary>
        /// Adjust the columns sizes.
        /// </summary>
        private void AdjustColumns()
        {
            int width = 0;
            foreach(ColumnHeader column in charactersListView.Columns)
            {
                if (column != columnUri)
                    width += column.Width;
            }
            columnUri.Width = charactersListView.ClientSize.Width - width;
        }

        /// <summary>
        /// We monitor/unmonitor characters as they are checked.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void charactersListView_ItemChecked(object sender, ItemCheckedEventArgs e)
        {
            if (m_refreshingCharactersCounter != 0)
                return;

            var character = (Character)e.Item.Tag;
            character.Monitored = e.Item.Checked;
        }

        /// <summary>
        /// Handle the "delete" button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void charactersListView_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Delete)
            {
                deleteCharacterMenu_Click(sender, e);
            }
        }

        /// <summary>
        /// On double click, we edit if this is an uri character.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void charactersListView_DoubleClick(object sender, EventArgs e)
        {
            editUriButton_Click(sender, e);
        }

        /// <summary>
        /// When the index changes, we enable or disable the toolbar buttons.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void charactersListView_SelectedIndexChanged(object sender, EventArgs e)
        {
            // No selection ?
            if (charactersListView.SelectedItems.Count == 0)
            {
                deleteCharacterMenu.Enabled = false;
                editUriMenu.Enabled = false;
                return;
            }

            // "Edit uri" enabled when an uri char is selected
            var item = charactersListView.SelectedItems[0];
            var uriCharacter = item.Tag as UriCharacter;
            editUriMenu.Enabled = (uriCharacter != null);

            // Delete char enabled if one character selected.
            deleteCharacterMenu.Enabled = true;

        }

        /// <summary>
        /// Characters toolbar > Import...
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void importCharacterMenu_Click(object sender, EventArgs e)
        {
            using (var form = new CharacterImportationWindow())
            {
                var result = form.ShowDialog(this);
            }
        }

        /// <summary>
        /// Characters toolbar > Delete...
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void deleteCharacterMenu_Click(object sender, EventArgs e)
        {
            // Retrieve the selected URI character
            if (charactersListView.SelectedItems.Count == 0) return;
            var item = charactersListView.SelectedItems[0];
            var character = item.Tag as Character;

            // Opens the character deletion
            using (var window = new CharacterDeletionWindow(character))
            {
                window.ShowDialog(this);
            }
        }

        /// <summary>
        /// Characters toolbar > Edit Uri...
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void editUriButton_Click(object sender, EventArgs e)
        {
            // Retrieve the selected URI character
            if (charactersListView.SelectedItems.Count == 0) return;
            var item = charactersListView.SelectedItems[0];
            var uriCharacter = item.Tag as UriCharacter;

            // Returns if the selected item is not an Uri character
            if (uriCharacter == null) return;

            // Opens the importation form
            using (var form = new CharacterImportationWindow(uriCharacter))
            {
                var result = form.ShowDialog(this);
            }
        }

        /// <summary>
        /// Characters toolbar > Group items
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void groupingMenu_Click(object sender, EventArgs e)
        {
            m_refreshingCharactersCounter++;
            UpdateCharactersListContent();
            m_refreshingCharactersCounter--;
        }
        #endregion

        /// <summary>
        /// Close on "close" button click.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void closeButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
