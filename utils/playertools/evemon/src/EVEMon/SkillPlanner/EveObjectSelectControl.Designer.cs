namespace EVEMon.SkillPlanner
{
    partial class EveObjectSelectControl
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(EveObjectSelectControl));
            this.tbSearchText = new System.Windows.Forms.TextBox();
            this.lbSearchTextHint = new System.Windows.Forms.Label();
            this.lbNoMatches = new System.Windows.Forms.Label();
            this.lbSearchList = new System.Windows.Forms.ListBox();
            this.contextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.cmiExpandSelected = new System.Windows.Forms.ToolStripMenuItem();
            this.cmiCollapseSelected = new System.Windows.Forms.ToolStripMenuItem();
            this.tsSeparator = new System.Windows.Forms.ToolStripSeparator();
            this.cmiExpandAll = new System.Windows.Forms.ToolStripMenuItem();
            this.cmiCollapseAll = new System.Windows.Forms.ToolStripMenuItem();
            this.upperPanel = new System.Windows.Forms.Panel();
            this.cbUsabilityFilter = new System.Windows.Forms.ComboBox();
            this.lblFilter = new System.Windows.Forms.Label();
            this.pbSearchImage = new System.Windows.Forms.PictureBox();
            this.lowerPanel = new System.Windows.Forms.Panel();
            this.tvItems = new EVEMon.Common.Controls.TreeView();
            this.contextMenu.SuspendLayout();
            this.upperPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbSearchImage)).BeginInit();
            this.lowerPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // tbSearchText
            // 
            this.tbSearchText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbSearchText.Location = new System.Drawing.Point(45, 29);
            this.tbSearchText.Margin = new System.Windows.Forms.Padding(0);
            this.tbSearchText.Name = "tbSearchText";
            this.tbSearchText.Size = new System.Drawing.Size(140, 20);
            this.tbSearchText.TabIndex = 21;
            this.tbSearchText.TextChanged += new System.EventHandler(this.tbSearchText_TextChanged);
            this.tbSearchText.Leave += new System.EventHandler(this.tbSearchText_Leave);
            this.tbSearchText.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.tbSearchText_KeyPress);
            this.tbSearchText.Enter += new System.EventHandler(this.tbSearchText_Enter);
            // 
            // lbSearchTextHint
            // 
            this.lbSearchTextHint.AutoSize = true;
            this.lbSearchTextHint.BackColor = System.Drawing.SystemColors.Window;
            this.lbSearchTextHint.ForeColor = System.Drawing.SystemColors.GrayText;
            this.lbSearchTextHint.Location = new System.Drawing.Point(47, 32);
            this.lbSearchTextHint.Margin = new System.Windows.Forms.Padding(0);
            this.lbSearchTextHint.Name = "lbSearchTextHint";
            this.lbSearchTextHint.Size = new System.Drawing.Size(65, 13);
            this.lbSearchTextHint.TabIndex = 23;
            this.lbSearchTextHint.Text = "Search Text";
            this.lbSearchTextHint.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lbSearchTextHint.Click += new System.EventHandler(this.lbSearchTextHint_Click);
            // 
            // lbNoMatches
            // 
            this.lbNoMatches.BackColor = System.Drawing.SystemColors.Window;
            this.lbNoMatches.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lbNoMatches.ForeColor = System.Drawing.SystemColors.GrayText;
            this.lbNoMatches.Location = new System.Drawing.Point(0, 0);
            this.lbNoMatches.Margin = new System.Windows.Forms.Padding(0);
            this.lbNoMatches.Name = "lbNoMatches";
            this.lbNoMatches.Padding = new System.Windows.Forms.Padding(4, 30, 4, 4);
            this.lbNoMatches.Size = new System.Drawing.Size(185, 344);
            this.lbNoMatches.TabIndex = 24;
            this.lbNoMatches.Text = "No ships match your search.";
            this.lbNoMatches.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // lbSearchList
            // 
            this.lbSearchList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lbSearchList.FormattingEnabled = true;
            this.lbSearchList.IntegralHeight = false;
            this.lbSearchList.Location = new System.Drawing.Point(0, 0);
            this.lbSearchList.Margin = new System.Windows.Forms.Padding(0);
            this.lbSearchList.Name = "lbSearchList";
            this.lbSearchList.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.lbSearchList.Size = new System.Drawing.Size(185, 344);
            this.lbSearchList.TabIndex = 25;
            this.lbSearchList.Visible = false;
            this.lbSearchList.SelectedIndexChanged += new System.EventHandler(this.lbSearchList_SelectedIndexChanged);
            // 
            // contextMenu
            // 
            this.contextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cmiExpandSelected,
            this.cmiCollapseSelected,
            this.tsSeparator,
            this.cmiExpandAll,
            this.cmiCollapseAll});
            this.contextMenu.Name = "contextMenu";
            this.contextMenu.Size = new System.Drawing.Size(167, 120);
            this.contextMenu.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenu_Opening);
            // 
            // cmiExpandSelected
            // 
            this.cmiExpandSelected.Name = "cmiExpandSelected";
            this.cmiExpandSelected.Size = new System.Drawing.Size(166, 22);
            this.cmiExpandSelected.Text = "Expand Selected";
            this.cmiExpandSelected.Click += new System.EventHandler(this.cmiExpandSelected_Click);
            // 
            // cmiCollapseSelected
            // 
            this.cmiCollapseSelected.Name = "cmiCollapseSelected";
            this.cmiCollapseSelected.Size = new System.Drawing.Size(166, 22);
            this.cmiCollapseSelected.Text = "Collapse Selected";
            this.cmiCollapseSelected.Click += new System.EventHandler(this.cmiCollapseSelected_Click);
            // 
            // tsSeparator
            // 
            this.tsSeparator.Name = "tsSeparator";
            this.tsSeparator.Size = new System.Drawing.Size(163, 6);
            // 
            // cmiExpandAll
            // 
            this.cmiExpandAll.Name = "cmiExpandAll";
            this.cmiExpandAll.Size = new System.Drawing.Size(166, 22);
            this.cmiExpandAll.Text = "Expand All";
            this.cmiExpandAll.Click += new System.EventHandler(this.cmiExpandAll_Click);
            // 
            // cmiCollapseAll
            // 
            this.cmiCollapseAll.Name = "cmiCollapseAll";
            this.cmiCollapseAll.Size = new System.Drawing.Size(166, 22);
            this.cmiCollapseAll.Text = "Collapse All";
            this.cmiCollapseAll.Click += new System.EventHandler(this.cmiCollapseAll_Click);
            // 
            // upperPanel
            // 
            this.upperPanel.Controls.Add(this.cbUsabilityFilter);
            this.upperPanel.Controls.Add(this.lblFilter);
            this.upperPanel.Controls.Add(this.lbSearchTextHint);
            this.upperPanel.Controls.Add(this.tbSearchText);
            this.upperPanel.Controls.Add(this.pbSearchImage);
            this.upperPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.upperPanel.Location = new System.Drawing.Point(0, 0);
            this.upperPanel.Margin = new System.Windows.Forms.Padding(0);
            this.upperPanel.Name = "upperPanel";
            this.upperPanel.Size = new System.Drawing.Size(185, 54);
            this.upperPanel.TabIndex = 26;
            // 
            // cbUsabilityFilter
            // 
            this.cbUsabilityFilter.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.cbUsabilityFilter.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbUsabilityFilter.FormattingEnabled = true;
            this.cbUsabilityFilter.Items.AddRange(new object[] {
            "All",
            "Can",
            "Can not"});
            this.cbUsabilityFilter.Location = new System.Drawing.Point(45, 3);
            this.cbUsabilityFilter.Margin = new System.Windows.Forms.Padding(0);
            this.cbUsabilityFilter.Name = "cbUsabilityFilter";
            this.cbUsabilityFilter.Size = new System.Drawing.Size(140, 21);
            this.cbUsabilityFilter.TabIndex = 0;
            // 
            // lblFilter
            // 
            this.lblFilter.AutoSize = true;
            this.lblFilter.Location = new System.Drawing.Point(6, 6);
            this.lblFilter.Margin = new System.Windows.Forms.Padding(0);
            this.lblFilter.Name = "lblFilter";
            this.lblFilter.Size = new System.Drawing.Size(32, 13);
            this.lblFilter.TabIndex = 24;
            this.lblFilter.Text = "Filter:";
            // 
            // pbSearchImage
            // 
            this.pbSearchImage.Image = ((System.Drawing.Image)(resources.GetObject("pbSearchImage.Image")));
            this.pbSearchImage.InitialImage = null;
            this.pbSearchImage.Location = new System.Drawing.Point(9, 30);
            this.pbSearchImage.Margin = new System.Windows.Forms.Padding(0);
            this.pbSearchImage.Name = "pbSearchImage";
            this.pbSearchImage.Size = new System.Drawing.Size(20, 20);
            this.pbSearchImage.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pbSearchImage.TabIndex = 20;
            this.pbSearchImage.TabStop = false;
            // 
            // lowerPanel
            // 
            this.lowerPanel.AutoSize = true;
            this.lowerPanel.Controls.Add(this.lbNoMatches);
            this.lowerPanel.Controls.Add(this.lbSearchList);
            this.lowerPanel.Controls.Add(this.tvItems);
            this.lowerPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lowerPanel.Location = new System.Drawing.Point(0, 54);
            this.lowerPanel.Margin = new System.Windows.Forms.Padding(0);
            this.lowerPanel.Name = "lowerPanel";
            this.lowerPanel.Size = new System.Drawing.Size(185, 344);
            this.lowerPanel.TabIndex = 27;
            // 
            // tvItems
            // 
            this.tvItems.ContextMenuStrip = this.contextMenu;
            this.tvItems.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvItems.Location = new System.Drawing.Point(0, 0);
            this.tvItems.Margin = new System.Windows.Forms.Padding(0);
            this.tvItems.Name = "tvItems";
            this.tvItems.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            this.tvItems.SelectionMode = EVEMon.Common.Controls.TreeViewSelectionMode.MultiSelect;
            this.tvItems.Size = new System.Drawing.Size(185, 344);
            this.tvItems.TabIndex = 22;
            this.tvItems.SelectionsChanged += new System.EventHandler(this.tvItems_SelectionsChanged);
            // 
            // EveObjectSelectControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.Controls.Add(this.lowerPanel);
            this.Controls.Add(this.upperPanel);
            this.Margin = new System.Windows.Forms.Padding(0);
            this.Name = "EveObjectSelectControl";
            this.Size = new System.Drawing.Size(185, 398);
            this.Load += new System.EventHandler(this.EveObjectSelectControl_Load);
            this.contextMenu.ResumeLayout(false);
            this.upperPanel.ResumeLayout(false);
            this.upperPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbSearchImage)).EndInit();
            this.lowerPanel.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        protected System.Windows.Forms.TextBox tbSearchText;
        protected System.Windows.Forms.Label lbNoMatches;
        protected System.Windows.Forms.ListBox lbSearchList;
        protected System.Windows.Forms.Panel upperPanel;
        protected System.Windows.Forms.Panel lowerPanel;
        protected System.Windows.Forms.Label lbSearchTextHint;
        protected System.Windows.Forms.PictureBox pbSearchImage;
        protected System.Windows.Forms.Label lblFilter;
        protected System.Windows.Forms.ComboBox cbUsabilityFilter;
        protected internal EVEMon.Common.Controls.TreeView tvItems;
        private System.Windows.Forms.ContextMenuStrip contextMenu;
        private System.Windows.Forms.ToolStripMenuItem cmiExpandAll;
        private System.Windows.Forms.ToolStripMenuItem cmiCollapseAll;
        private System.Windows.Forms.ToolStripMenuItem cmiExpandSelected;
        private System.Windows.Forms.ToolStripMenuItem cmiCollapseSelected;
        private System.Windows.Forms.ToolStripSeparator tsSeparator;
    }
}
