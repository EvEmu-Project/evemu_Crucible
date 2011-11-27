namespace EVEMon.Accounting
{
    partial class AccountsManagementWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.Label accountsLabel;
            System.Windows.Forms.Label charactersLabel;
            System.Windows.Forms.Label label1;
            System.Windows.Forms.Label labelCharactersNote;
            System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem(new string[] {
            "",
            "CCP",
            "John Doe",
            "4567651",
            "(none)"}, 0);
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AccountsManagementWindow));
            this.closeButton = new System.Windows.Forms.Button();
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.panel1 = new System.Windows.Forms.Panel();
            this.charactersMultiPanel = new EVEMon.Controls.MultiPanel();
            this.charactersListPage = new EVEMon.Controls.MultiPanelPage();
            this.charactersListView = new System.Windows.Forms.ListView();
            this.columnMonitored = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnType = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnAccount = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnUri = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.noCharactersPage = new EVEMon.Controls.MultiPanelPage();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.importCharacterMenu = new System.Windows.Forms.ToolStripButton();
            this.deleteCharacterMenu = new System.Windows.Forms.ToolStripButton();
            this.editUriMenu = new System.Windows.Forms.ToolStripButton();
            this.groupingMenu = new System.Windows.Forms.ToolStripButton();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.panel2 = new System.Windows.Forms.Panel();
            this.accountsMultiPanel = new EVEMon.Controls.MultiPanel();
            this.accountsListPage = new EVEMon.Controls.MultiPanelPage();
            this.accountsListBox = new EVEMon.Accounting.AccountsListBox();
            this.noAccountsPage = new EVEMon.Controls.MultiPanelPage();
            this.toolStrip2 = new System.Windows.Forms.ToolStrip();
            this.addAccountMenu = new System.Windows.Forms.ToolStripButton();
            this.deleteAccountMenu = new System.Windows.Forms.ToolStripButton();
            this.editAccountMenu = new System.Windows.Forms.ToolStripButton();
            this.groupMenu = new System.Windows.Forms.ToolStripButton();
            this.addCharacterMenu = new System.Windows.Forms.ToolStripButton();
            accountsLabel = new System.Windows.Forms.Label();
            charactersLabel = new System.Windows.Forms.Label();
            label1 = new System.Windows.Forms.Label();
            labelCharactersNote = new System.Windows.Forms.Label();
            this.tabControl.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.panel1.SuspendLayout();
            this.charactersMultiPanel.SuspendLayout();
            this.charactersListPage.SuspendLayout();
            this.noCharactersPage.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.accountsMultiPanel.SuspendLayout();
            this.accountsListPage.SuspendLayout();
            this.noAccountsPage.SuspendLayout();
            this.toolStrip2.SuspendLayout();
            this.SuspendLayout();
            // 
            // accountsLabel
            // 
            accountsLabel.AutoSize = true;
            accountsLabel.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            accountsLabel.ForeColor = System.Drawing.SystemColors.Highlight;
            accountsLabel.Location = new System.Drawing.Point(6, 3);
            accountsLabel.Name = "accountsLabel";
            accountsLabel.Size = new System.Drawing.Size(73, 19);
            accountsLabel.TabIndex = 1;
            accountsLabel.Text = "Accounts";
            // 
            // charactersLabel
            // 
            charactersLabel.AutoSize = true;
            charactersLabel.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            charactersLabel.ForeColor = System.Drawing.SystemColors.Highlight;
            charactersLabel.Location = new System.Drawing.Point(6, 3);
            charactersLabel.Name = "charactersLabel";
            charactersLabel.Size = new System.Drawing.Size(83, 19);
            charactersLabel.TabIndex = 5;
            charactersLabel.Text = "Characters";
            // 
            // label1
            // 
            label1.Dock = System.Windows.Forms.DockStyle.Fill;
            label1.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            label1.ForeColor = System.Drawing.SystemColors.ControlDarkDark;
            label1.Location = new System.Drawing.Point(0, 0);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(633, 364);
            label1.TabIndex = 0;
            label1.Text = "First add your accounts with the buttons above.";
            label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // labelCharactersNote
            // 
            labelCharactersNote.Dock = System.Windows.Forms.DockStyle.Fill;
            labelCharactersNote.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            labelCharactersNote.ForeColor = System.Drawing.SystemColors.ControlDarkDark;
            labelCharactersNote.Location = new System.Drawing.Point(0, 0);
            labelCharactersNote.Name = "labelCharactersNote";
            labelCharactersNote.Size = new System.Drawing.Size(633, 364);
            labelCharactersNote.TabIndex = 10;
            labelCharactersNote.Text = "First add your accounts, characters will then appear in this list.\r\nYou can also " +
                "import XML character sheets from files or URLs.";
            labelCharactersNote.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // closeButton
            // 
            this.closeButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.closeButton.CausesValidation = false;
            this.closeButton.Location = new System.Drawing.Point(579, 464);
            this.closeButton.Name = "closeButton";
            this.closeButton.Size = new System.Drawing.Size(90, 23);
            this.closeButton.TabIndex = 9;
            this.closeButton.Text = "&Close";
            this.closeButton.UseVisualStyleBackColor = true;
            this.closeButton.Click += new System.EventHandler(this.closeButton_Click);
            // 
            // tabControl
            // 
            this.tabControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl.Controls.Add(this.tabPage2);
            this.tabControl.Controls.Add(this.tabPage1);
            this.tabControl.Location = new System.Drawing.Point(12, 12);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(657, 446);
            this.tabControl.TabIndex = 12;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.panel1);
            this.tabPage2.Controls.Add(charactersLabel);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(649, 420);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Characters";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.Controls.Add(this.charactersMultiPanel);
            this.panel1.Controls.Add(this.toolStrip1);
            this.panel1.Location = new System.Drawing.Point(10, 25);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(633, 389);
            this.panel1.TabIndex = 16;
            // 
            // charactersMultiPanel
            // 
            this.charactersMultiPanel.Controls.Add(this.charactersListPage);
            this.charactersMultiPanel.Controls.Add(this.noCharactersPage);
            this.charactersMultiPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.charactersMultiPanel.Location = new System.Drawing.Point(0, 25);
            this.charactersMultiPanel.Name = "charactersMultiPanel";
            this.charactersMultiPanel.SelectedPage = this.noCharactersPage;
            this.charactersMultiPanel.Size = new System.Drawing.Size(633, 364);
            this.charactersMultiPanel.TabIndex = 15;
            // 
            // charactersListPage
            // 
            this.charactersListPage.Controls.Add(this.charactersListView);
            this.charactersListPage.Dock = System.Windows.Forms.DockStyle.Fill;
            this.charactersListPage.Location = new System.Drawing.Point(0, 0);
            this.charactersListPage.Name = "charactersListPage";
            this.charactersListPage.Size = new System.Drawing.Size(633, 364);
            this.charactersListPage.TabIndex = 0;
            this.charactersListPage.Text = "charactersListPage";
            // 
            // charactersListView
            // 
            this.charactersListView.CheckBoxes = true;
            this.charactersListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnMonitored,
            this.columnType,
            this.columnName,
            this.columnAccount,
            this.columnUri});
            this.charactersListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.charactersListView.FullRowSelect = true;
            listViewItem1.StateImageIndex = 0;
            this.charactersListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem1});
            this.charactersListView.Location = new System.Drawing.Point(0, 0);
            this.charactersListView.MultiSelect = false;
            this.charactersListView.Name = "charactersListView";
            this.charactersListView.Size = new System.Drawing.Size(633, 364);
            this.charactersListView.TabIndex = 12;
            this.charactersListView.UseCompatibleStateImageBehavior = false;
            this.charactersListView.View = System.Windows.Forms.View.Details;
            // 
            // columnMonitored
            // 
            this.columnMonitored.Text = "";
            this.columnMonitored.Width = 36;
            // 
            // columnType
            // 
            this.columnType.Text = "Type";
            this.columnType.Width = 49;
            // 
            // columnName
            // 
            this.columnName.Text = "Name";
            this.columnName.Width = 117;
            // 
            // columnAccount
            // 
            this.columnAccount.Text = "Account";
            this.columnAccount.Width = 75;
            // 
            // columnUri
            // 
            this.columnUri.Text = "Uri";
            this.columnUri.Width = 358;
            // 
            // noCharactersPage
            // 
            this.noCharactersPage.Controls.Add(labelCharactersNote);
            this.noCharactersPage.Dock = System.Windows.Forms.DockStyle.Fill;
            this.noCharactersPage.Location = new System.Drawing.Point(0, 0);
            this.noCharactersPage.Name = "noCharactersPage";
            this.noCharactersPage.Size = new System.Drawing.Size(633, 364);
            this.noCharactersPage.TabIndex = 1;
            this.noCharactersPage.Text = "noCharactersPage";
            // 
            // toolStrip1
            // 
            this.toolStrip1.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.importCharacterMenu,
            this.deleteCharacterMenu,
            this.editUriMenu,
            this.groupingMenu});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(633, 25);
            this.toolStrip1.TabIndex = 13;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // importCharacterMenu
            // 
            this.importCharacterMenu.Image = global::EVEMon.Common.Properties.Resources.ImportCharacter;
            this.importCharacterMenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.importCharacterMenu.Name = "importCharacterMenu";
            this.importCharacterMenu.Size = new System.Drawing.Size(72, 22);
            this.importCharacterMenu.Text = "&Import...";
            this.importCharacterMenu.Click += new System.EventHandler(this.importCharacterMenu_Click);
            // 
            // deleteCharacterMenu
            // 
            this.deleteCharacterMenu.Enabled = false;
            this.deleteCharacterMenu.Image = global::EVEMon.Common.Properties.Resources.DeleteCharacter;
            this.deleteCharacterMenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.deleteCharacterMenu.Name = "deleteCharacterMenu";
            this.deleteCharacterMenu.Size = new System.Drawing.Size(69, 22);
            this.deleteCharacterMenu.Text = "&Delete...";
            this.deleteCharacterMenu.Click += new System.EventHandler(this.deleteCharacterMenu_Click);
            // 
            // editUriMenu
            // 
            this.editUriMenu.Enabled = false;
            this.editUriMenu.Image = global::EVEMon.Common.Properties.Resources.EditChar;
            this.editUriMenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.editUriMenu.Name = "editUriMenu";
            this.editUriMenu.Size = new System.Drawing.Size(74, 22);
            this.editUriMenu.Text = "&Edit Uri...";
            this.editUriMenu.Click += new System.EventHandler(this.editUriButton_Click);
            // 
            // groupingMenu
            // 
            this.groupingMenu.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.groupingMenu.Checked = true;
            this.groupingMenu.CheckOnClick = true;
            this.groupingMenu.CheckState = System.Windows.Forms.CheckState.Checked;
            this.groupingMenu.Image = ((System.Drawing.Image)(resources.GetObject("groupingMenu.Image")));
            this.groupingMenu.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
            this.groupingMenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.groupingMenu.Name = "groupingMenu";
            this.groupingMenu.Size = new System.Drawing.Size(117, 22);
            this.groupingMenu.Text = "&Group characters";
            this.groupingMenu.Click += new System.EventHandler(this.groupingMenu_Click);
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.panel2);
            this.tabPage1.Controls.Add(accountsLabel);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(649, 420);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Accounts";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // panel2
            // 
            this.panel2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panel2.Controls.Add(this.accountsMultiPanel);
            this.panel2.Controls.Add(this.toolStrip2);
            this.panel2.Location = new System.Drawing.Point(10, 25);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(633, 389);
            this.panel2.TabIndex = 17;
            // 
            // accountsMultiPanel
            // 
            this.accountsMultiPanel.Controls.Add(this.accountsListPage);
            this.accountsMultiPanel.Controls.Add(this.noAccountsPage);
            this.accountsMultiPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.accountsMultiPanel.Location = new System.Drawing.Point(0, 25);
            this.accountsMultiPanel.Name = "accountsMultiPanel";
            this.accountsMultiPanel.SelectedPage = this.noAccountsPage;
            this.accountsMultiPanel.Size = new System.Drawing.Size(633, 364);
            this.accountsMultiPanel.TabIndex = 16;
            // 
            // accountsListPage
            // 
            this.accountsListPage.Controls.Add(this.accountsListBox);
            this.accountsListPage.Dock = System.Windows.Forms.DockStyle.Fill;
            this.accountsListPage.Location = new System.Drawing.Point(0, 0);
            this.accountsListPage.Name = "accountsListPage";
            this.accountsListPage.Size = new System.Drawing.Size(633, 364);
            this.accountsListPage.TabIndex = 0;
            this.accountsListPage.Text = "accountsListPage";
            // 
            // accountsListBox
            // 
            this.accountsListBox.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.accountsListBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.accountsListBox.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
            this.accountsListBox.Font = new System.Drawing.Font("Tahoma", 9.75F);
            this.accountsListBox.FormattingEnabled = true;
            this.accountsListBox.IntegralHeight = false;
            this.accountsListBox.ItemHeight = 40;
            this.accountsListBox.Location = new System.Drawing.Point(0, 0);
            this.accountsListBox.Name = "accountsListBox";
            this.accountsListBox.Size = new System.Drawing.Size(633, 364);
            this.accountsListBox.TabIndex = 2;
            // 
            // noAccountsPage
            // 
            this.noAccountsPage.Controls.Add(label1);
            this.noAccountsPage.Dock = System.Windows.Forms.DockStyle.Fill;
            this.noAccountsPage.Location = new System.Drawing.Point(0, 0);
            this.noAccountsPage.Name = "noAccountsPage";
            this.noAccountsPage.Size = new System.Drawing.Size(633, 364);
            this.noAccountsPage.TabIndex = 1;
            this.noAccountsPage.Text = "noAccountsPage";
            // 
            // toolStrip2
            // 
            this.toolStrip2.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
            this.toolStrip2.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addAccountMenu,
            this.deleteAccountMenu,
            this.editAccountMenu});
            this.toolStrip2.Location = new System.Drawing.Point(0, 0);
            this.toolStrip2.Name = "toolStrip2";
            this.toolStrip2.Size = new System.Drawing.Size(633, 25);
            this.toolStrip2.TabIndex = 3;
            this.toolStrip2.Text = "toolStrip2";
            // 
            // addAccountMenu
            // 
            this.addAccountMenu.Image = global::EVEMon.Common.Properties.Resources.AddAccount;
            this.addAccountMenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.addAccountMenu.Name = "addAccountMenu";
            this.addAccountMenu.Size = new System.Drawing.Size(58, 22);
            this.addAccountMenu.Text = "&Add...";
            this.addAccountMenu.Click += new System.EventHandler(this.addAccountMenu_Click);
            // 
            // deleteAccountMenu
            // 
            this.deleteAccountMenu.Enabled = false;
            this.deleteAccountMenu.Image = global::EVEMon.Common.Properties.Resources.DeleteAccount;
            this.deleteAccountMenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.deleteAccountMenu.Name = "deleteAccountMenu";
            this.deleteAccountMenu.Size = new System.Drawing.Size(69, 22);
            this.deleteAccountMenu.Text = "&Delete...";
            this.deleteAccountMenu.Click += new System.EventHandler(this.deleteAccountMenu_Click);
            // 
            // editAccountMenu
            // 
            this.editAccountMenu.Enabled = false;
            this.editAccountMenu.Image = global::EVEMon.Common.Properties.Resources.EditAccount;
            this.editAccountMenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.editAccountMenu.Name = "editAccountMenu";
            this.editAccountMenu.Size = new System.Drawing.Size(56, 22);
            this.editAccountMenu.Text = "&Edit...";
            this.editAccountMenu.Click += new System.EventHandler(this.editAccountMenu_Click);
            // 
            // groupMenu
            // 
            this.groupMenu.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.groupMenu.Image = ((System.Drawing.Image)(resources.GetObject("groupMenu.Image")));
            this.groupMenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.groupMenu.Name = "groupMenu";
            this.groupMenu.Size = new System.Drawing.Size(23, 22);
            this.groupMenu.Text = "toolStripButton4";
            // 
            // addCharacterMenu
            // 
            this.addCharacterMenu.Name = "addCharacterMenu";
            this.addCharacterMenu.Size = new System.Drawing.Size(23, 23);
            // 
            // AccountsManagementWindow
            // 
            this.AcceptButton = this.closeButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.CausesValidation = false;
            this.ClientSize = new System.Drawing.Size(681, 499);
            this.Controls.Add(this.tabControl);
            this.Controls.Add(this.closeButton);
            this.MaximizeBox = false;
            this.MinimumSize = new System.Drawing.Size(580, 530);
            this.Name = "AccountsManagementWindow";
            this.Text = "Accounts Management";
            this.tabControl.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.charactersMultiPanel.ResumeLayout(false);
            this.charactersListPage.ResumeLayout(false);
            this.noCharactersPage.ResumeLayout(false);
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.panel2.ResumeLayout(false);
            this.panel2.PerformLayout();
            this.accountsMultiPanel.ResumeLayout(false);
            this.accountsListPage.ResumeLayout(false);
            this.noAccountsPage.ResumeLayout(false);
            this.toolStrip2.ResumeLayout(false);
            this.toolStrip2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button closeButton;
        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private AccountsListBox accountsListBox;
        private System.Windows.Forms.ListView charactersListView;
        private System.Windows.Forms.ColumnHeader columnType;
        private System.Windows.Forms.ColumnHeader columnName;
        private System.Windows.Forms.ColumnHeader columnAccount;
        private System.Windows.Forms.ColumnHeader columnUri;
        private System.Windows.Forms.ColumnHeader columnMonitored;
        private EVEMon.Controls.MultiPanel accountsMultiPanel;
        private EVEMon.Controls.MultiPanelPage accountsListPage;
        private EVEMon.Controls.MultiPanelPage noAccountsPage;
        private EVEMon.Controls.MultiPanel charactersMultiPanel;
        private EVEMon.Controls.MultiPanelPage charactersListPage;
        private EVEMon.Controls.MultiPanelPage noCharactersPage;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton groupMenu;
        private System.Windows.Forms.ToolStripButton addCharacterMenu;
        private System.Windows.Forms.ToolStripButton importCharacterMenu;
        private System.Windows.Forms.ToolStripButton deleteCharacterMenu;
        private System.Windows.Forms.ToolStripButton editUriMenu;
        private System.Windows.Forms.ToolStripButton groupingMenu;
        private System.Windows.Forms.ToolStrip toolStrip2;
        private System.Windows.Forms.ToolStripButton addAccountMenu;
        private System.Windows.Forms.ToolStripButton deleteAccountMenu;
        private System.Windows.Forms.ToolStripButton editAccountMenu;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Panel panel2;
    }
}