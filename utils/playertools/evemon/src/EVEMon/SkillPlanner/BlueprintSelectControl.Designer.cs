namespace EVEMon.SkillPlanner
{
    partial class BlueprintSelectControl
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
            this.cbTech1 = new System.Windows.Forms.CheckBox();
            this.cbTech2 = new System.Windows.Forms.CheckBox();
            this.tlpGrouping = new System.Windows.Forms.TableLayoutPanel();
            this.cbStoryline = new System.Windows.Forms.CheckBox();
            this.cbFaction = new System.Windows.Forms.CheckBox();
            this.cbTech3 = new System.Windows.Forms.CheckBox();
            this.lblActivity = new System.Windows.Forms.Label();
            this.cbActivityFilter = new System.Windows.Forms.ComboBox();
            this.cbOfficer = new System.Windows.Forms.CheckBox();
            this.upperPanel.SuspendLayout();
            this.lowerPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbSearchImage)).BeginInit();
            this.tlpGrouping.SuspendLayout();
            this.SuspendLayout();
            // 
            // tbSearchText
            // 
            this.tbSearchText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbSearchText.Location = new System.Drawing.Point(45, 148);
            this.tbSearchText.Size = new System.Drawing.Size(179, 20);
            this.tbSearchText.TabIndex = 3;
            this.tbSearchText.TextChanged += new System.EventHandler(this.tbSearchText_TextChanged);
            // 
            // lbNoMatches
            // 
            this.lbNoMatches.Size = new System.Drawing.Size(224, 240);
            this.lbNoMatches.TabIndex = 10;
            this.lbNoMatches.Text = "No blueprints match your search.";
            // 
            // lbSearchList
            // 
            this.lbSearchList.Size = new System.Drawing.Size(224, 240);
            this.lbSearchList.TabIndex = 9;
            // 
            // upperPanel
            // 
            this.upperPanel.Controls.Add(this.cbActivityFilter);
            this.upperPanel.Controls.Add(this.tlpGrouping);
            this.upperPanel.Controls.Add(this.lblActivity);
            this.upperPanel.Size = new System.Drawing.Size(224, 178);
            this.upperPanel.TabIndex = 0;
            this.upperPanel.Controls.SetChildIndex(this.lblActivity, 0);
            this.upperPanel.Controls.SetChildIndex(this.tbSearchText, 0);
            this.upperPanel.Controls.SetChildIndex(this.lblFilter, 0);
            this.upperPanel.Controls.SetChildIndex(this.lbSearchTextHint, 0);
            this.upperPanel.Controls.SetChildIndex(this.tlpGrouping, 0);
            this.upperPanel.Controls.SetChildIndex(this.cbActivityFilter, 0);
            this.upperPanel.Controls.SetChildIndex(this.pbSearchImage, 0);
            this.upperPanel.Controls.SetChildIndex(this.cbUsabilityFilter, 0);
            // 
            // lowerPanel
            // 
            this.lowerPanel.Location = new System.Drawing.Point(0, 178);
            this.lowerPanel.Size = new System.Drawing.Size(224, 240);
            // 
            // lbSearchTextHint
            // 
            this.lbSearchTextHint.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.lbSearchTextHint.Location = new System.Drawing.Point(49, 151);
            this.lbSearchTextHint.TabIndex = 8;
            // 
            // pbSearchImage
            // 
            this.pbSearchImage.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.pbSearchImage.Location = new System.Drawing.Point(9, 151);
            // 
            // lblFilter
            // 
            this.lblFilter.TabIndex = 0;
            // 
            // cbUsabilityFilter
            // 
            this.cbUsabilityFilter.Size = new System.Drawing.Size(179, 21);
            this.cbUsabilityFilter.TabIndex = 1;
            this.cbUsabilityFilter.SelectedIndexChanged += new System.EventHandler(this.cbUsabilityFilter_SelectedIndexChanged);
            // 
            // tvItems
            // 
            this.tvItems.LineColor = System.Drawing.Color.Black;
            this.tvItems.SelectionMode = EVEMon.Common.Controls.TreeViewSelectionMode.SingleSelect;
            this.tvItems.Size = new System.Drawing.Size(224, 240);
            this.tvItems.TabIndex = 4;
            // 
            // cbTech1
            // 
            this.cbTech1.AutoSize = true;
            this.cbTech1.Checked = true;
            this.cbTech1.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbTech1.Location = new System.Drawing.Point(3, 3);
            this.cbTech1.Name = "cbTech1";
            this.cbTech1.Size = new System.Drawing.Size(57, 17);
            this.cbTech1.TabIndex = 0;
            this.cbTech1.Text = "Tech I";
            this.cbTech1.UseVisualStyleBackColor = true;
            this.cbTech1.CheckedChanged += new System.EventHandler(this.cbMetagroup_CheckedChanged);
            // 
            // cbTech2
            // 
            this.cbTech2.AutoSize = true;
            this.cbTech2.Checked = true;
            this.cbTech2.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbTech2.Location = new System.Drawing.Point(3, 26);
            this.cbTech2.Name = "cbTech2";
            this.cbTech2.Size = new System.Drawing.Size(60, 17);
            this.cbTech2.TabIndex = 1;
            this.cbTech2.Text = "Tech II";
            this.cbTech2.UseVisualStyleBackColor = true;
            this.cbTech2.CheckedChanged += new System.EventHandler(this.cbMetagroup_CheckedChanged);
            // 
            // tlpGrouping
            // 
            this.tlpGrouping.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tlpGrouping.ColumnCount = 2;
            this.tlpGrouping.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tlpGrouping.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tlpGrouping.Controls.Add(this.cbOfficer, 0, 2);
            this.tlpGrouping.Controls.Add(this.cbStoryline, 1, 1);
            this.tlpGrouping.Controls.Add(this.cbFaction, 1, 0);
            this.tlpGrouping.Controls.Add(this.cbTech1, 0, 0);
            this.tlpGrouping.Controls.Add(this.cbTech2, 0, 1);
            this.tlpGrouping.Controls.Add(this.cbTech3, 0, 2);
            this.tlpGrouping.Location = new System.Drawing.Point(9, 63);
            this.tlpGrouping.Name = "tlpGrouping";
            this.tlpGrouping.RowCount = 3;
            this.tlpGrouping.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpGrouping.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpGrouping.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tlpGrouping.Size = new System.Drawing.Size(212, 70);
            this.tlpGrouping.TabIndex = 2;
            // 
            // cbStoryline
            // 
            this.cbStoryline.AutoSize = true;
            this.cbStoryline.Checked = true;
            this.cbStoryline.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbStoryline.Location = new System.Drawing.Point(109, 26);
            this.cbStoryline.Name = "cbStoryline";
            this.cbStoryline.Size = new System.Drawing.Size(66, 17);
            this.cbStoryline.TabIndex = 4;
            this.cbStoryline.Text = "Storyline";
            this.cbStoryline.UseVisualStyleBackColor = true;
            this.cbStoryline.CheckedChanged += new System.EventHandler(this.cbMetagroup_CheckedChanged);
            // 
            // cbFaction
            // 
            this.cbFaction.AutoSize = true;
            this.cbFaction.Checked = true;
            this.cbFaction.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbFaction.Location = new System.Drawing.Point(109, 3);
            this.cbFaction.Name = "cbFaction";
            this.cbFaction.Size = new System.Drawing.Size(61, 17);
            this.cbFaction.TabIndex = 3;
            this.cbFaction.Text = "Faction";
            this.cbFaction.UseVisualStyleBackColor = true;
            this.cbFaction.CheckedChanged += new System.EventHandler(this.cbMetagroup_CheckedChanged);
            // 
            // cbTech3
            // 
            this.cbTech3.AutoSize = true;
            this.cbTech3.Checked = true;
            this.cbTech3.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbTech3.Location = new System.Drawing.Point(3, 49);
            this.cbTech3.Name = "cbTech3";
            this.cbTech3.Size = new System.Drawing.Size(63, 17);
            this.cbTech3.TabIndex = 2;
            this.cbTech3.Text = "Tech III";
            this.cbTech3.UseVisualStyleBackColor = true;
            this.cbTech3.CheckedChanged += new System.EventHandler(this.cbMetagroup_CheckedChanged);
            // 
            // lblActivity
            // 
            this.lblActivity.AutoSize = true;
            this.lblActivity.Location = new System.Drawing.Point(16, 31);
            this.lblActivity.Name = "lblActivity";
            this.lblActivity.Size = new System.Drawing.Size(22, 13);
            this.lblActivity.TabIndex = 21;
            this.lblActivity.Text = "for:";
            // 
            // cbActivityFilter
            // 
            this.cbActivityFilter.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.cbActivityFilter.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbActivityFilter.FormattingEnabled = true;
            this.cbActivityFilter.Items.AddRange(new object[] {
            "Any Activity",
            "All Activities",
            "Manufacturing",
            "Copying",
            "Researching Material Efficiency ",
            "Researching Time Productivity",
            "Invention"});
            this.cbActivityFilter.Location = new System.Drawing.Point(45, 28);
            this.cbActivityFilter.Name = "cbActivityFilter";
            this.cbActivityFilter.Size = new System.Drawing.Size(179, 21);
            this.cbActivityFilter.TabIndex = 22;
            this.cbActivityFilter.SelectedIndexChanged += new System.EventHandler(this.cbActivity_SelectedIndexChanged);
            // 
            // cbOfficer
            // 
            this.cbOfficer.AutoSize = true;
            this.cbOfficer.Checked = true;
            this.cbOfficer.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbOfficer.Location = new System.Drawing.Point(109, 49);
            this.cbOfficer.Name = "cbOfficer";
            this.cbOfficer.Size = new System.Drawing.Size(57, 17);
            this.cbOfficer.TabIndex = 5;
            this.cbOfficer.Text = "Officer";
            this.cbOfficer.UseVisualStyleBackColor = true;
            // 
            // BlueprintSelectControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.DoubleBuffered = true;
            this.Name = "BlueprintSelectControl";
            this.Size = new System.Drawing.Size(224, 418);
            this.upperPanel.ResumeLayout(false);
            this.upperPanel.PerformLayout();
            this.lowerPanel.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pbSearchImage)).EndInit();
            this.tlpGrouping.ResumeLayout(false);
            this.tlpGrouping.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox cbTech1;
        private System.Windows.Forms.CheckBox cbTech2;
        private System.Windows.Forms.CheckBox cbStoryline;
        private System.Windows.Forms.CheckBox cbFaction;
        private System.Windows.Forms.CheckBox cbTech3;
        private System.Windows.Forms.TableLayoutPanel tlpGrouping;
        private System.Windows.Forms.ComboBox cbActivityFilter;
        private System.Windows.Forms.Label lblActivity;
        private System.Windows.Forms.CheckBox cbOfficer;
    }
}
