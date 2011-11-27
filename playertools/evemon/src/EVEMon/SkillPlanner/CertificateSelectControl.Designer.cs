namespace EVEMon.SkillPlanner
{
    partial class CertificateSelectControl
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CertificateSelectControl));
            this.cbFilter = new System.Windows.Forms.ComboBox();
            this.cbSorting = new System.Windows.Forms.ComboBox();
            this.lblSort = new System.Windows.Forms.Label();
            this.pbSearchImage = new System.Windows.Forms.PictureBox();
            this.lbSearchTextHint = new System.Windows.Forms.Label();
            this.lblFilter = new System.Windows.Forms.Label();
            this.tbSearchText = new System.Windows.Forms.TextBox();
            this.pnlResults = new System.Windows.Forms.Panel();
            this.lbNoMatches = new System.Windows.Forms.Label();
            this.lbSearchList = new System.Windows.Forms.ListBox();
            this.cmListSkills = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.cmiLvPlanTo = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmLevel1 = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmLevel2 = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmLevel3 = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmLevel4 = new System.Windows.Forms.ToolStripMenuItem();
            this.tsSeparatorPlanTo = new System.Windows.Forms.ToolStripSeparator();
            this.tsmExpandSelected = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmCollapseSelected = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.tsmExpandAll = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmCollapseAll = new System.Windows.Forms.ToolStripMenuItem();
            this.tvItems = new EVEMon.Common.Controls.TreeView();
            this.ilSkillIcons = new System.Windows.Forms.ImageList(this.components);
            this.lvSortedList = new System.Windows.Forms.ListView();
            this.chName = new System.Windows.Forms.ColumnHeader();
            this.chSortKey = new System.Windows.Forms.ColumnHeader();
            this.pnlFilter = new System.Windows.Forms.Panel();
            ((System.ComponentModel.ISupportInitialize)(this.pbSearchImage)).BeginInit();
            this.pnlResults.SuspendLayout();
            this.cmListSkills.SuspendLayout();
            this.pnlFilter.SuspendLayout();
            this.SuspendLayout();
            // 
            // cbFilter
            // 
            this.cbFilter.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.cbFilter.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbFilter.Items.AddRange(new object[] {
            "All",
            "Claimable",
            "Hide elite",
            "Next grade is trainable",
            "Next grade is untrainable"});
            this.cbFilter.Location = new System.Drawing.Point(45, 3);
            this.cbFilter.Margin = new System.Windows.Forms.Padding(2, 3, 2, 3);
            this.cbFilter.MaxDropDownItems = 12;
            this.cbFilter.Name = "cbFilter";
            this.cbFilter.Size = new System.Drawing.Size(182, 21);
            this.cbFilter.TabIndex = 51;
            this.cbFilter.SelectedIndexChanged += new System.EventHandler(this.cbFilter_SelectedIndexChanged);
            // 
            // cbSorting
            // 
            this.cbSorting.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.cbSorting.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbSorting.FormattingEnabled = true;
            this.cbSorting.Items.AddRange(new object[] {
            "Name",
            "Time to next grade",
            "Time to elite grade"});
            this.cbSorting.Location = new System.Drawing.Point(45, 29);
            this.cbSorting.Margin = new System.Windows.Forms.Padding(2, 3, 2, 3);
            this.cbSorting.Name = "cbSorting";
            this.cbSorting.Size = new System.Drawing.Size(182, 21);
            this.cbSorting.TabIndex = 55;
            this.cbSorting.SelectedIndexChanged += new System.EventHandler(this.cbSorting_SelectedIndexChanged);
            // 
            // lblSort
            // 
            this.lblSort.AutoSize = true;
            this.lblSort.Location = new System.Drawing.Point(9, 32);
            this.lblSort.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.lblSort.Name = "lblSort";
            this.lblSort.Size = new System.Drawing.Size(29, 13);
            this.lblSort.TabIndex = 56;
            this.lblSort.Text = "Sort:";
            // 
            // pbSearchImage
            // 
            this.pbSearchImage.Image = ((System.Drawing.Image)(resources.GetObject("pbSearchImage.Image")));
            this.pbSearchImage.InitialImage = null;
            this.pbSearchImage.Location = new System.Drawing.Point(9, 54);
            this.pbSearchImage.Margin = new System.Windows.Forms.Padding(2);
            this.pbSearchImage.Name = "pbSearchImage";
            this.pbSearchImage.Size = new System.Drawing.Size(20, 20);
            this.pbSearchImage.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pbSearchImage.TabIndex = 52;
            this.pbSearchImage.TabStop = false;
            // 
            // lbSearchTextHint
            // 
            this.lbSearchTextHint.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            this.lbSearchTextHint.AutoSize = true;
            this.lbSearchTextHint.BackColor = System.Drawing.SystemColors.Window;
            this.lbSearchTextHint.ForeColor = System.Drawing.SystemColors.GrayText;
            this.lbSearchTextHint.Location = new System.Drawing.Point(49, 57);
            this.lbSearchTextHint.Name = "lbSearchTextHint";
            this.lbSearchTextHint.Size = new System.Drawing.Size(65, 13);
            this.lbSearchTextHint.TabIndex = 53;
            this.lbSearchTextHint.Text = "Search Text";
            this.lbSearchTextHint.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lbSearchTextHint.Click += new System.EventHandler(this.lbSearchTextHint_Click);
            // 
            // lblFilter
            // 
            this.lblFilter.AutoSize = true;
            this.lblFilter.Location = new System.Drawing.Point(6, 6);
            this.lblFilter.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.lblFilter.Name = "lblFilter";
            this.lblFilter.Size = new System.Drawing.Size(32, 13);
            this.lblFilter.TabIndex = 54;
            this.lblFilter.Text = "Filter:";
            // 
            // tbSearchText
            // 
            this.tbSearchText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbSearchText.Location = new System.Drawing.Point(45, 54);
            this.tbSearchText.Margin = new System.Windows.Forms.Padding(2, 3, 2, 3);
            this.tbSearchText.Name = "tbSearchText";
            this.tbSearchText.Size = new System.Drawing.Size(182, 20);
            this.tbSearchText.TabIndex = 57;
            this.tbSearchText.TextChanged += new System.EventHandler(this.tbSearchText_TextChanged);
            // 
            // pnlResults
            // 
            this.pnlResults.AutoSize = true;
            this.pnlResults.Controls.Add(this.lbNoMatches);
            this.pnlResults.Controls.Add(this.lbSearchList);
            this.pnlResults.Controls.Add(this.tvItems);
            this.pnlResults.Controls.Add(this.lvSortedList);
            this.pnlResults.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pnlResults.Location = new System.Drawing.Point(0, 78);
            this.pnlResults.Margin = new System.Windows.Forms.Padding(0);
            this.pnlResults.Name = "pnlResults";
            this.pnlResults.Size = new System.Drawing.Size(227, 343);
            this.pnlResults.TabIndex = 60;
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
            this.lbNoMatches.Size = new System.Drawing.Size(227, 343);
            this.lbNoMatches.TabIndex = 23;
            this.lbNoMatches.Text = "No certificates match your search.";
            this.lbNoMatches.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            this.lbNoMatches.Visible = false;
            // 
            // lbSearchList
            // 
            this.lbSearchList.ContextMenuStrip = this.cmListSkills;
            this.lbSearchList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lbSearchList.FormattingEnabled = true;
            this.lbSearchList.IntegralHeight = false;
            this.lbSearchList.Location = new System.Drawing.Point(0, 0);
            this.lbSearchList.Margin = new System.Windows.Forms.Padding(2);
            this.lbSearchList.Name = "lbSearchList";
            this.lbSearchList.Size = new System.Drawing.Size(227, 343);
            this.lbSearchList.TabIndex = 46;
            this.lbSearchList.Visible = false;
            this.lbSearchList.SelectedIndexChanged += new System.EventHandler(this.lbSearchList_SelectedIndexChanged);
            // 
            // cmListSkills
            // 
            this.cmListSkills.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cmiLvPlanTo,
            this.tsSeparatorPlanTo,
            this.tsmExpandSelected,
            this.tsmCollapseSelected,
            this.toolStripSeparator1,
            this.tsmExpandAll,
            this.tsmCollapseAll});
            this.cmListSkills.Name = "cmListSkills";
            this.cmListSkills.Size = new System.Drawing.Size(167, 148);
            // 
            // cmiLvPlanTo
            // 
            this.cmiLvPlanTo.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmLevel1,
            this.tsmLevel2,
            this.tsmLevel3,
            this.tsmLevel4});
            this.cmiLvPlanTo.Image = global::EVEMon.Common.Properties.Resources.EditPlan;
            this.cmiLvPlanTo.Name = "cmiLvPlanTo";
            this.cmiLvPlanTo.Size = new System.Drawing.Size(166, 22);
            this.cmiLvPlanTo.Text = "&Add to plan...";
            // 
            // tsmLevel1
            // 
            this.tsmLevel1.Name = "tsmLevel1";
            this.tsmLevel1.Size = new System.Drawing.Size(125, 22);
            this.tsmLevel1.Text = "&Basic";
            this.tsmLevel1.Click += new System.EventHandler(this.tsmLevel1_Click);
            // 
            // tsmLevel2
            // 
            this.tsmLevel2.Name = "tsmLevel2";
            this.tsmLevel2.Size = new System.Drawing.Size(125, 22);
            this.tsmLevel2.Text = "&Standard";
            this.tsmLevel2.Click += new System.EventHandler(this.tsmLevel2_Click);
            // 
            // tsmLevel3
            // 
            this.tsmLevel3.Name = "tsmLevel3";
            this.tsmLevel3.Size = new System.Drawing.Size(125, 22);
            this.tsmLevel3.Text = "&Improved";
            this.tsmLevel3.Click += new System.EventHandler(this.tsmLevel3_Click);
            // 
            // tsmLevel4
            // 
            this.tsmLevel4.Name = "tsmLevel4";
            this.tsmLevel4.Size = new System.Drawing.Size(125, 22);
            this.tsmLevel4.Text = "&Elite";
            this.tsmLevel4.Click += new System.EventHandler(this.tsmLevel4_Click);
            // 
            // tsSeparatorPlanTo
            // 
            this.tsSeparatorPlanTo.Name = "tsSeparatorPlanTo";
            this.tsSeparatorPlanTo.Size = new System.Drawing.Size(163, 6);
            // 
            // tsmExpandSelected
            // 
            this.tsmExpandSelected.Name = "tsmExpandSelected";
            this.tsmExpandSelected.Size = new System.Drawing.Size(166, 22);
            this.tsmExpandSelected.Text = "Expand Selected";
            this.tsmExpandSelected.Click += new System.EventHandler(this.tsmExpandSelected_Click);
            // 
            // tsmCollapseSelected
            // 
            this.tsmCollapseSelected.Name = "tsmCollapseSelected";
            this.tsmCollapseSelected.Size = new System.Drawing.Size(166, 22);
            this.tsmCollapseSelected.Text = "Collapse Selected";
            this.tsmCollapseSelected.Click += new System.EventHandler(this.tsmCollapseSelected_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(163, 6);
            // 
            // tsmExpandAll
            // 
            this.tsmExpandAll.Name = "tsmExpandAll";
            this.tsmExpandAll.Size = new System.Drawing.Size(166, 22);
            this.tsmExpandAll.Text = "&Expand All";
            this.tsmExpandAll.Click += new System.EventHandler(this.tsmExpandAll_Click);
            // 
            // tsmCollapseAll
            // 
            this.tsmCollapseAll.Name = "tsmCollapseAll";
            this.tsmCollapseAll.Size = new System.Drawing.Size(166, 22);
            this.tsmCollapseAll.Text = "&Collapse All";
            this.tsmCollapseAll.Click += new System.EventHandler(this.tsmCollapseAll_Click);
            // 
            // tvItems
            // 
            this.tvItems.ContextMenuStrip = this.cmListSkills;
            this.tvItems.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvItems.HideSelection = false;
            this.tvItems.ImageIndex = 0;
            this.tvItems.ImageList = this.ilSkillIcons;
            this.tvItems.Location = new System.Drawing.Point(0, 0);
            this.tvItems.Margin = new System.Windows.Forms.Padding(2);
            this.tvItems.Name = "tvItems";
            this.tvItems.SelectedImageIndex = 0;
            this.tvItems.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            this.tvItems.SelectionMode = EVEMon.Common.Controls.TreeViewSelectionMode.SingleSelect;
            this.tvItems.Size = new System.Drawing.Size(227, 343);
            this.tvItems.TabIndex = 20;
            // 
            // ilSkillIcons
            // 
            this.ilSkillIcons.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("ilSkillIcons.ImageStream")));
            this.ilSkillIcons.TransparentColor = System.Drawing.Color.Transparent;
            this.ilSkillIcons.Images.SetKeyName(0, "Certificate");
            this.ilSkillIcons.Images.SetKeyName(1, "AllUntrained");
            this.ilSkillIcons.Images.SetKeyName(2, "AllGranted");
            // 
            // lvSortedList
            // 
            this.lvSortedList.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.chName,
            this.chSortKey});
            this.lvSortedList.ContextMenuStrip = this.cmListSkills;
            this.lvSortedList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvSortedList.FullRowSelect = true;
            this.lvSortedList.Location = new System.Drawing.Point(0, 0);
            this.lvSortedList.Margin = new System.Windows.Forms.Padding(2);
            this.lvSortedList.Name = "lvSortedList";
            this.lvSortedList.Size = new System.Drawing.Size(227, 343);
            this.lvSortedList.TabIndex = 28;
            this.lvSortedList.TileSize = new System.Drawing.Size(16, 16);
            this.lvSortedList.UseCompatibleStateImageBehavior = false;
            this.lvSortedList.View = System.Windows.Forms.View.Details;
            this.lvSortedList.Visible = false;
            // 
            // chName
            // 
            this.chName.Text = "Name";
            this.chName.Width = 120;
            // 
            // chSortKey
            // 
            this.chSortKey.Text = "Sort";
            // 
            // pnlFilter
            // 
            this.pnlFilter.Controls.Add(this.cbSorting);
            this.pnlFilter.Controls.Add(this.cbFilter);
            this.pnlFilter.Controls.Add(this.lbSearchTextHint);
            this.pnlFilter.Controls.Add(this.tbSearchText);
            this.pnlFilter.Controls.Add(this.lblFilter);
            this.pnlFilter.Controls.Add(this.pbSearchImage);
            this.pnlFilter.Controls.Add(this.lblSort);
            this.pnlFilter.Dock = System.Windows.Forms.DockStyle.Top;
            this.pnlFilter.Location = new System.Drawing.Point(0, 0);
            this.pnlFilter.Margin = new System.Windows.Forms.Padding(2);
            this.pnlFilter.Name = "pnlFilter";
            this.pnlFilter.Size = new System.Drawing.Size(227, 78);
            this.pnlFilter.TabIndex = 47;
            // 
            // CertificateSelectControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.pnlResults);
            this.Controls.Add(this.pnlFilter);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "CertificateSelectControl";
            this.Size = new System.Drawing.Size(227, 421);
            ((System.ComponentModel.ISupportInitialize)(this.pbSearchImage)).EndInit();
            this.pnlResults.ResumeLayout(false);
            this.cmListSkills.ResumeLayout(false);
            this.pnlFilter.ResumeLayout(false);
            this.pnlFilter.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cbFilter;
        private System.Windows.Forms.ComboBox cbSorting;
        private System.Windows.Forms.Label lblSort;
        private System.Windows.Forms.PictureBox pbSearchImage;
        private System.Windows.Forms.Label lbSearchTextHint;
        private System.Windows.Forms.Label lblFilter;
        private System.Windows.Forms.TextBox tbSearchText;
        private System.Windows.Forms.Panel pnlResults;
        private System.Windows.Forms.Label lbNoMatches;
        private System.Windows.Forms.ListBox lbSearchList;
        private EVEMon.Common.Controls.TreeView tvItems;
        private System.Windows.Forms.ListView lvSortedList;
        private System.Windows.Forms.ColumnHeader chName;
        private System.Windows.Forms.ColumnHeader chSortKey;
        private System.Windows.Forms.ImageList ilSkillIcons;
        private System.Windows.Forms.ContextMenuStrip cmListSkills;
        private System.Windows.Forms.ToolStripMenuItem cmiLvPlanTo;
        private System.Windows.Forms.ToolStripMenuItem tsmLevel1;
        private System.Windows.Forms.ToolStripMenuItem tsmLevel2;
        private System.Windows.Forms.ToolStripMenuItem tsmLevel3;
        private System.Windows.Forms.ToolStripMenuItem tsmLevel4;
        private System.Windows.Forms.ToolStripSeparator tsSeparatorPlanTo;
        private System.Windows.Forms.ToolStripMenuItem tsmExpandAll;
        private System.Windows.Forms.ToolStripMenuItem tsmCollapseAll;
        private System.Windows.Forms.Panel pnlFilter;
        private System.Windows.Forms.ToolStripMenuItem tsmExpandSelected;
        private System.Windows.Forms.ToolStripMenuItem tsmCollapseSelected;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
    }
}
