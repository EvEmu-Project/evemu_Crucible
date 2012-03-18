namespace EVEMon.Common.Controls
{
    partial class CopySaveOptionsWindow
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.tbPreview = new System.Windows.Forms.TextBox();
            this.btnOk = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.cbIncludeHeader = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.cbEntryCost = new System.Windows.Forms.CheckBox();
            this.cbEntryFinishDate = new System.Windows.Forms.CheckBox();
            this.cbEntryStartDate = new System.Windows.Forms.CheckBox();
            this.cbEntryTrainingTimes = new System.Windows.Forms.CheckBox();
            this.cbEntryNumber = new System.Windows.Forms.CheckBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.cbFooterCost = new System.Windows.Forms.CheckBox();
            this.cbFooterDate = new System.Windows.Forms.CheckBox();
            this.cbFooterTotalTime = new System.Windows.Forms.CheckBox();
            this.cbFooterCount = new System.Windows.Forms.CheckBox();
            this.cbRememberOptions = new System.Windows.Forms.CheckBox();
            this.cmbFormatting = new System.Windows.Forms.ComboBox();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.cbShoppingList = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.tbPreview);
            this.groupBox1.Location = new System.Drawing.Point(12, 292);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(340, 154);
            this.groupBox1.TabIndex = 6;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Output Sample";
            // 
            // tbPreview
            // 
            this.tbPreview.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbPreview.Location = new System.Drawing.Point(6, 20);
            this.tbPreview.Multiline = true;
            this.tbPreview.Name = "tbPreview";
            this.tbPreview.ReadOnly = true;
            this.tbPreview.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.tbPreview.Size = new System.Drawing.Size(328, 128);
            this.tbPreview.TabIndex = 0;
            this.tbPreview.WordWrap = false;
            // 
            // btnOk
            // 
            this.btnOk.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOk.Location = new System.Drawing.Point(198, 454);
            this.btnOk.Name = "btnOk";
            this.btnOk.Size = new System.Drawing.Size(75, 23);
            this.btnOk.TabIndex = 4;
            this.btnOk.Text = "OK";
            this.btnOk.UseVisualStyleBackColor = true;
            this.btnOk.Click += new System.EventHandler(this.btnOk_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(279, 454);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 5;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Controls.Add(this.cbIncludeHeader);
            this.groupBox2.Location = new System.Drawing.Point(12, 12);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(340, 47);
            this.groupBox2.TabIndex = 0;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Header Options";
            // 
            // cbIncludeHeader
            // 
            this.cbIncludeHeader.AutoSize = true;
            this.cbIncludeHeader.Location = new System.Drawing.Point(16, 20);
            this.cbIncludeHeader.Name = "cbIncludeHeader";
            this.cbIncludeHeader.Size = new System.Drawing.Size(196, 17);
            this.cbIncludeHeader.TabIndex = 0;
            this.cbIncludeHeader.Text = "Include header with character name";
            this.cbIncludeHeader.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox3.Controls.Add(this.cbEntryCost);
            this.groupBox3.Controls.Add(this.cbEntryFinishDate);
            this.groupBox3.Controls.Add(this.cbEntryStartDate);
            this.groupBox3.Controls.Add(this.cbEntryTrainingTimes);
            this.groupBox3.Controls.Add(this.cbEntryNumber);
            this.groupBox3.Location = new System.Drawing.Point(12, 65);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(340, 97);
            this.groupBox3.TabIndex = 1;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Body Options";
            // 
            // cbEntryCost
            // 
            this.cbEntryCost.AutoSize = true;
            this.cbEntryCost.Location = new System.Drawing.Point(16, 67);
            this.cbEntryCost.Name = "cbEntryCost";
            this.cbEntryCost.Size = new System.Drawing.Size(145, 17);
            this.cbEntryCost.TabIndex = 4;
            this.cbEntryCost.Text = "Include cost (if unknown)";
            this.cbEntryCost.UseVisualStyleBackColor = true;
            // 
            // cbEntryFinishDate
            // 
            this.cbEntryFinishDate.AutoSize = true;
            this.cbEntryFinishDate.Location = new System.Drawing.Point(169, 43);
            this.cbEntryFinishDate.Name = "cbEntryFinishDate";
            this.cbEntryFinishDate.Size = new System.Drawing.Size(136, 17);
            this.cbEntryFinishDate.TabIndex = 3;
            this.cbEntryFinishDate.Text = "Include finish date/time";
            this.cbEntryFinishDate.UseVisualStyleBackColor = true;
            // 
            // cbEntryStartDate
            // 
            this.cbEntryStartDate.AutoSize = true;
            this.cbEntryStartDate.Location = new System.Drawing.Point(169, 20);
            this.cbEntryStartDate.Name = "cbEntryStartDate";
            this.cbEntryStartDate.Size = new System.Drawing.Size(132, 17);
            this.cbEntryStartDate.TabIndex = 2;
            this.cbEntryStartDate.Text = "Include start date/time";
            this.cbEntryStartDate.UseVisualStyleBackColor = true;
            // 
            // cbEntryTrainingTimes
            // 
            this.cbEntryTrainingTimes.AutoSize = true;
            this.cbEntryTrainingTimes.Location = new System.Drawing.Point(16, 43);
            this.cbEntryTrainingTimes.Name = "cbEntryTrainingTimes";
            this.cbEntryTrainingTimes.Size = new System.Drawing.Size(120, 17);
            this.cbEntryTrainingTimes.TabIndex = 1;
            this.cbEntryTrainingTimes.Text = "Include training time";
            this.cbEntryTrainingTimes.UseVisualStyleBackColor = true;
            // 
            // cbEntryNumber
            // 
            this.cbEntryNumber.AutoSize = true;
            this.cbEntryNumber.Location = new System.Drawing.Point(16, 20);
            this.cbEntryNumber.Name = "cbEntryNumber";
            this.cbEntryNumber.Size = new System.Drawing.Size(97, 17);
            this.cbEntryNumber.TabIndex = 0;
            this.cbEntryNumber.Text = "Number entries";
            this.cbEntryNumber.UseVisualStyleBackColor = true;
            // 
            // groupBox4
            // 
            this.groupBox4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox4.Controls.Add(this.cbFooterCost);
            this.groupBox4.Controls.Add(this.cbFooterDate);
            this.groupBox4.Controls.Add(this.cbFooterTotalTime);
            this.groupBox4.Controls.Add(this.cbFooterCount);
            this.groupBox4.Location = new System.Drawing.Point(12, 168);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(192, 118);
            this.groupBox4.TabIndex = 2;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Footer Options";
            // 
            // cbFooterCost
            // 
            this.cbFooterCost.AutoSize = true;
            this.cbFooterCost.Location = new System.Drawing.Point(16, 90);
            this.cbFooterCost.Name = "cbFooterCost";
            this.cbFooterCost.Size = new System.Drawing.Size(168, 17);
            this.cbFooterCost.TabIndex = 3;
            this.cbFooterCost.Text = "Include cost of unknown skills";
            this.cbFooterCost.UseVisualStyleBackColor = true;
            // 
            // cbFooterDate
            // 
            this.cbFooterDate.AutoSize = true;
            this.cbFooterDate.Location = new System.Drawing.Point(16, 66);
            this.cbFooterDate.Name = "cbFooterDate";
            this.cbFooterDate.Size = new System.Drawing.Size(163, 17);
            this.cbFooterDate.TabIndex = 2;
            this.cbFooterDate.Text = "Include completion date/time";
            this.cbFooterDate.UseVisualStyleBackColor = true;
            // 
            // cbFooterTotalTime
            // 
            this.cbFooterTotalTime.AutoSize = true;
            this.cbFooterTotalTime.Location = new System.Drawing.Point(16, 43);
            this.cbFooterTotalTime.Name = "cbFooterTotalTime";
            this.cbFooterTotalTime.Size = new System.Drawing.Size(143, 17);
            this.cbFooterTotalTime.TabIndex = 1;
            this.cbFooterTotalTime.Text = "Include total training time";
            this.cbFooterTotalTime.UseVisualStyleBackColor = true;
            // 
            // cbFooterCount
            // 
            this.cbFooterCount.AutoSize = true;
            this.cbFooterCount.Location = new System.Drawing.Point(16, 20);
            this.cbFooterCount.Name = "cbFooterCount";
            this.cbFooterCount.Size = new System.Drawing.Size(111, 17);
            this.cbFooterCount.TabIndex = 0;
            this.cbFooterCount.Text = "Include skill count";
            this.cbFooterCount.UseVisualStyleBackColor = true;
            // 
            // cbRememberOptions
            // 
            this.cbRememberOptions.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.cbRememberOptions.AutoSize = true;
            this.cbRememberOptions.Location = new System.Drawing.Point(18, 458);
            this.cbRememberOptions.Name = "cbRememberOptions";
            this.cbRememberOptions.Size = new System.Drawing.Size(139, 17);
            this.cbRememberOptions.TabIndex = 3;
            this.cbRememberOptions.Text = "Save options as default";
            this.cbRememberOptions.UseVisualStyleBackColor = true;
            // 
            // cmbFormatting
            // 
            this.cmbFormatting.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbFormatting.FormattingEnabled = true;
            this.cmbFormatting.Items.AddRange(new object[] {
            "BBCode",
            "HTML",
            "Plain text"});
            this.cmbFormatting.Location = new System.Drawing.Point(8, 34);
            this.cmbFormatting.Margin = new System.Windows.Forms.Padding(2);
            this.cmbFormatting.Name = "cmbFormatting";
            this.cmbFormatting.Size = new System.Drawing.Size(92, 21);
            this.cmbFormatting.TabIndex = 3;
            this.cmbFormatting.SelectedIndexChanged += new System.EventHandler(this.comboBox1_SelectedIndexChanged);
            // 
            // groupBox5
            // 
            this.groupBox5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox5.Controls.Add(this.cbShoppingList);
            this.groupBox5.Controls.Add(this.label1);
            this.groupBox5.Controls.Add(this.cmbFormatting);
            this.groupBox5.Location = new System.Drawing.Point(221, 168);
            this.groupBox5.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox5.Size = new System.Drawing.Size(130, 118);
            this.groupBox5.TabIndex = 7;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Other Options";
            // 
            // cbShoppingList
            // 
            this.cbShoppingList.AutoSize = true;
            this.cbShoppingList.Location = new System.Drawing.Point(8, 67);
            this.cbShoppingList.Margin = new System.Windows.Forms.Padding(2);
            this.cbShoppingList.Name = "cbShoppingList";
            this.cbShoppingList.Size = new System.Drawing.Size(86, 17);
            this.cbShoppingList.TabIndex = 0;
            this.cbShoppingList.Text = "Shopping list";
            this.cbShoppingList.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 18);
            this.label1.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(63, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "Formatting:";
            // 
            // CopySaveOptionsWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.ClientSize = new System.Drawing.Size(364, 487);
            this.Controls.Add(this.cbRememberOptions);
            this.Controls.Add(this.groupBox5);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOk);
            this.Controls.Add(this.groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "CopySaveOptionsWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Copy Options";
            this.Load += new System.EventHandler(this.CopySaveOptionsWindow_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnOk;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.CheckBox cbIncludeHeader;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.CheckBox cbEntryFinishDate;
        private System.Windows.Forms.CheckBox cbEntryStartDate;
        private System.Windows.Forms.CheckBox cbEntryTrainingTimes;
        private System.Windows.Forms.CheckBox cbEntryNumber;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.CheckBox cbFooterDate;
        private System.Windows.Forms.CheckBox cbFooterTotalTime;
        private System.Windows.Forms.CheckBox cbFooterCount;
        private System.Windows.Forms.CheckBox cbRememberOptions;
        private System.Windows.Forms.TextBox tbPreview;
        private System.Windows.Forms.ComboBox cmbFormatting;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox cbShoppingList;
        private System.Windows.Forms.CheckBox cbFooterCost;
        private System.Windows.Forms.CheckBox cbEntryCost;
    }
}
