namespace EVEMon.SkillPlanner
{
    partial class ShipSelectControl
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
        private void InitializeComponent()
        {
            this.cbCaldari = new System.Windows.Forms.CheckBox();
            this.cbFaction = new System.Windows.Forms.CheckBox();
            this.cbGallente = new System.Windows.Forms.CheckBox();
            this.cbMinmatar = new System.Windows.Forms.CheckBox();
            this.cbORE = new System.Windows.Forms.CheckBox();
            this.cbAmarr = new System.Windows.Forms.CheckBox();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.upperPanel.SuspendLayout();
            this.lowerPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbSearchImage)).BeginInit();
            this.tableLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tbSearchText
            // 
            this.tbSearchText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbSearchText.Location = new System.Drawing.Point(45, 135);
            this.tbSearchText.Size = new System.Drawing.Size(179, 20);
            this.tbSearchText.TabIndex = 35;
            // 
            // tvItems
            // 
            this.tvItems.LineColor = System.Drawing.Color.Black;
            this.tvItems.Size = new System.Drawing.Size(224, 256);
            // 
            // lbNoMatches
            // 
            this.lbNoMatches.Size = new System.Drawing.Size(224, 256);
            // 
            // lbSearchList
            // 
            this.lbSearchList.Size = new System.Drawing.Size(224, 256);
            // 
            // upperPanel
            // 
            this.upperPanel.Controls.Add(this.tableLayoutPanel1);
            this.upperPanel.Size = new System.Drawing.Size(224, 162);
            this.upperPanel.Controls.SetChildIndex(this.cbUsabilityFilter, 0);
            this.upperPanel.Controls.SetChildIndex(this.tableLayoutPanel1, 0);
            this.upperPanel.Controls.SetChildIndex(this.pbSearchImage, 0);
            this.upperPanel.Controls.SetChildIndex(this.tbSearchText, 0);
            this.upperPanel.Controls.SetChildIndex(this.lbSearchTextHint, 0);
            this.upperPanel.Controls.SetChildIndex(this.lblFilter, 0);
            // 
            // lowerPanel
            // 
            this.lowerPanel.Location = new System.Drawing.Point(0, 162);
            this.lowerPanel.Size = new System.Drawing.Size(224, 256);
            // 
            // lbSearchTextHint
            // 
            this.lbSearchTextHint.Location = new System.Drawing.Point(49, 138);
            // 
            // pbSearchImage
            // 
            this.pbSearchImage.Location = new System.Drawing.Point(9, 135);
            // 
            // lblFilter
            // 
            this.lblFilter.Margin = new System.Windows.Forms.Padding(5, 0, 5, 0);
            // 
            // cbUsabilityFilter
            // 
            this.cbUsabilityFilter.Size = new System.Drawing.Size(179, 21);
            this.cbUsabilityFilter.SelectedIndexChanged += new System.EventHandler(this.cbUsabilityFilter_SelectedIndexChanged);
            // 
            // cbCaldari
            // 
            this.cbCaldari.AccessibleRole = System.Windows.Forms.AccessibleRole.None;
            this.cbCaldari.AutoSize = true;
            this.cbCaldari.Cursor = System.Windows.Forms.Cursors.Default;
            this.cbCaldari.Location = new System.Drawing.Point(0, 22);
            this.cbCaldari.Margin = new System.Windows.Forms.Padding(0);
            this.cbCaldari.Name = "cbCaldari";
            this.cbCaldari.Size = new System.Drawing.Size(58, 17);
            this.cbCaldari.TabIndex = 27;
            this.cbCaldari.Text = "Caldari";
            this.cbCaldari.UseVisualStyleBackColor = true;
            this.cbCaldari.CheckedChanged += new System.EventHandler(this.cbRace_SelectedChanged);
            // 
            // cbFaction
            // 
            this.cbFaction.AutoSize = true;
            this.cbFaction.Location = new System.Drawing.Point(106, 22);
            this.cbFaction.Margin = new System.Windows.Forms.Padding(0);
            this.cbFaction.Name = "cbFaction";
            this.cbFaction.Size = new System.Drawing.Size(61, 17);
            this.cbFaction.TabIndex = 30;
            this.cbFaction.Text = "Faction";
            this.cbFaction.UseVisualStyleBackColor = true;
            this.cbFaction.CheckedChanged += new System.EventHandler(this.cbRace_SelectedChanged);
            // 
            // cbGallente
            // 
            this.cbGallente.AutoSize = true;
            this.cbGallente.Location = new System.Drawing.Point(0, 44);
            this.cbGallente.Margin = new System.Windows.Forms.Padding(0);
            this.cbGallente.Name = "cbGallente";
            this.cbGallente.Size = new System.Drawing.Size(65, 17);
            this.cbGallente.TabIndex = 28;
            this.cbGallente.Text = "Gallente";
            this.cbGallente.UseVisualStyleBackColor = true;
            this.cbGallente.CheckedChanged += new System.EventHandler(this.cbRace_SelectedChanged);
            // 
            // cbMinmatar
            // 
            this.cbMinmatar.AutoSize = true;
            this.cbMinmatar.Location = new System.Drawing.Point(106, 0);
            this.cbMinmatar.Margin = new System.Windows.Forms.Padding(0);
            this.cbMinmatar.Name = "cbMinmatar";
            this.cbMinmatar.Size = new System.Drawing.Size(69, 17);
            this.cbMinmatar.TabIndex = 29;
            this.cbMinmatar.Text = "Minmatar";
            this.cbMinmatar.UseVisualStyleBackColor = true;
            this.cbMinmatar.CheckedChanged += new System.EventHandler(this.cbRace_SelectedChanged);
            // 
            // cbORE
            // 
            this.cbORE.AutoSize = true;
            this.cbORE.Location = new System.Drawing.Point(106, 44);
            this.cbORE.Margin = new System.Windows.Forms.Padding(0);
            this.cbORE.Name = "cbORE";
            this.cbORE.Size = new System.Drawing.Size(49, 17);
            this.cbORE.TabIndex = 31;
            this.cbORE.Text = "ORE";
            this.cbORE.UseVisualStyleBackColor = true;
            this.cbORE.CheckedChanged += new System.EventHandler(this.cbRace_SelectedChanged);
            // 
            // cbAmarr
            // 
            this.cbAmarr.AutoSize = true;
            this.cbAmarr.Location = new System.Drawing.Point(0, 0);
            this.cbAmarr.Margin = new System.Windows.Forms.Padding(0);
            this.cbAmarr.Name = "cbAmarr";
            this.cbAmarr.Size = new System.Drawing.Size(53, 17);
            this.cbAmarr.TabIndex = 26;
            this.cbAmarr.Text = "Amarr";
            this.cbAmarr.UseVisualStyleBackColor = true;
            this.cbAmarr.CheckedChanged += new System.EventHandler(this.cbRace_SelectedChanged);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.Controls.Add(this.cbAmarr, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.cbMinmatar, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.cbCaldari, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.cbFaction, 1, 1);
            this.tableLayoutPanel1.Controls.Add(this.cbGallente, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.cbORE, 1, 2);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(6, 45);
            this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 3;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 34F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(212, 69);
            this.tableLayoutPanel1.TabIndex = 36;
            // 
            // ShipSelectControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Name = "ShipSelectControl";
            this.Size = new System.Drawing.Size(224, 418);
            this.upperPanel.ResumeLayout(false);
            this.upperPanel.PerformLayout();
            this.lowerPanel.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pbSearchImage)).EndInit();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        #endregion

        private System.Windows.Forms.CheckBox cbFaction;
        private System.Windows.Forms.CheckBox cbAmarr;
        private System.Windows.Forms.CheckBox cbORE;
        private System.Windows.Forms.CheckBox cbGallente;
        private System.Windows.Forms.CheckBox cbMinmatar;
        private System.Windows.Forms.CheckBox cbCaldari;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
    }
}
