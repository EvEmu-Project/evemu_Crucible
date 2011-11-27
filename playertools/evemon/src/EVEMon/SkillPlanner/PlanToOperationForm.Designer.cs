namespace EVEMon.SkillPlanner
{
    partial class PlanToOperationForm
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
            System.Windows.Forms.Label label1;
            System.Windows.Forms.PictureBox warningPicture;
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(PlanToOperationForm));
            System.Windows.Forms.Label label5;
            System.Windows.Forms.Label label3;
            this.previousButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.nextButton = new System.Windows.Forms.Button();
            this.rootMultiPanel = new EVEMon.Controls.MultiPanel();
            this.additionPage = new EVEMon.Controls.MultiPanelPage();
            this.priorityGroup = new System.Windows.Forms.GroupBox();
            this.priorityLabel = new System.Windows.Forms.Label();
            this.lbExisting = new System.Windows.Forms.Label();
            this.priorityNumericBox = new System.Windows.Forms.NumericUpDown();
            this.additionsListBox = new System.Windows.Forms.ListBox();
            this.dependenciesSuppressionPage = new EVEMon.Controls.MultiPanelPage();
            this.suppressionListBox = new System.Windows.Forms.ListBox();
            this.label2 = new System.Windows.Forms.Label();
            this.uselessPrereqsSuppressionPage = new EVEMon.Controls.MultiPanelPage();
            this.uselessPrereqsListBox = new System.Windows.Forms.ListBox();
            this.uselessPrereqsCheckbox = new System.Windows.Forms.CheckBox();
            label1 = new System.Windows.Forms.Label();
            warningPicture = new System.Windows.Forms.PictureBox();
            label5 = new System.Windows.Forms.Label();
            label3 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(warningPicture)).BeginInit();
            this.rootMultiPanel.SuspendLayout();
            this.additionPage.SuspendLayout();
            this.priorityGroup.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.priorityNumericBox)).BeginInit();
            this.dependenciesSuppressionPage.SuspendLayout();
            this.uselessPrereqsSuppressionPage.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new System.Drawing.Point(4, 4);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(175, 13);
            label1.TabIndex = 1;
            label1.Text = "The following entries will be added.";
            // 
            // warningPicture
            // 
            warningPicture.Image = ((System.Drawing.Image)(resources.GetObject("warningPicture.Image")));
            warningPicture.Location = new System.Drawing.Point(3, 3);
            warningPicture.Name = "warningPicture";
            warningPicture.Size = new System.Drawing.Size(32, 32);
            warningPicture.TabIndex = 0;
            warningPicture.TabStop = false;
            // 
            // label5
            // 
            label5.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            label5.AutoSize = true;
            label5.Location = new System.Drawing.Point(7, 229);
            label5.Name = "label5";
            label5.Size = new System.Drawing.Size(90, 13);
            label5.TabIndex = 14;
            label5.Text = "Add with priority:";
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Location = new System.Drawing.Point(6, 17);
            label3.Name = "label3";
            label3.Size = new System.Drawing.Size(285, 26);
            label3.TabIndex = 6;
            label3.Text = "You have exisiting pre-requisites in your plan, so you must\r\n\r\n";
            // 
            // previousButton
            // 
            this.previousButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.previousButton.Location = new System.Drawing.Point(78, 368);
            this.previousButton.Name = "previousButton";
            this.previousButton.Size = new System.Drawing.Size(75, 23);
            this.previousButton.TabIndex = 2;
            this.previousButton.Text = "< &Previous";
            this.previousButton.UseVisualStyleBackColor = true;
            this.previousButton.Click += new System.EventHandler(this.previousButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(241, 368);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 3;
            this.cancelButton.Text = "&Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // nextButton
            // 
            this.nextButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.nextButton.Location = new System.Drawing.Point(160, 368);
            this.nextButton.Name = "nextButton";
            this.nextButton.Size = new System.Drawing.Size(75, 23);
            this.nextButton.TabIndex = 1;
            this.nextButton.Text = "Next >";
            this.nextButton.UseVisualStyleBackColor = true;
            this.nextButton.Click += new System.EventHandler(this.nextButton_Click);
            // 
            // rootMultiPanel
            // 
            this.rootMultiPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.rootMultiPanel.Controls.Add(this.additionPage);
            this.rootMultiPanel.Controls.Add(this.dependenciesSuppressionPage);
            this.rootMultiPanel.Controls.Add(this.uselessPrereqsSuppressionPage);
            this.rootMultiPanel.Location = new System.Drawing.Point(12, 12);
            this.rootMultiPanel.Name = "rootMultiPanel";
            this.rootMultiPanel.SelectedPage = this.additionPage;
            this.rootMultiPanel.Size = new System.Drawing.Size(304, 350);
            this.rootMultiPanel.TabIndex = 0;
            // 
            // additionPage
            // 
            this.additionPage.Controls.Add(label5);
            this.additionPage.Controls.Add(this.priorityGroup);
            this.additionPage.Controls.Add(this.priorityNumericBox);
            this.additionPage.Controls.Add(label1);
            this.additionPage.Controls.Add(this.additionsListBox);
            this.additionPage.Dock = System.Windows.Forms.DockStyle.Fill;
            this.additionPage.Location = new System.Drawing.Point(0, 0);
            this.additionPage.Name = "additionPage";
            this.additionPage.Size = new System.Drawing.Size(304, 350);
            this.additionPage.TabIndex = 0;
            this.additionPage.Text = "additionPage";
            // 
            // priorityGroup
            // 
            this.priorityGroup.Controls.Add(this.priorityLabel);
            this.priorityGroup.Controls.Add(this.lbExisting);
            this.priorityGroup.Controls.Add(label3);
            this.priorityGroup.Location = new System.Drawing.Point(7, 263);
            this.priorityGroup.Name = "priorityGroup";
            this.priorityGroup.Size = new System.Drawing.Size(294, 84);
            this.priorityGroup.TabIndex = 13;
            this.priorityGroup.TabStop = false;
            this.priorityGroup.Text = "Existing Priorities";
            // 
            // priorityLabel
            // 
            this.priorityLabel.AutoSize = true;
            this.priorityLabel.Location = new System.Drawing.Point(7, 57);
            this.priorityLabel.Name = "priorityLabel";
            this.priorityLabel.Size = new System.Drawing.Size(178, 13);
            this.priorityLabel.TabIndex = 10;
            this.priorityLabel.Text = "The highest priority you can set is 3";
            // 
            // lbExisting
            // 
            this.lbExisting.AutoSize = true;
            this.lbExisting.Location = new System.Drawing.Point(6, 30);
            this.lbExisting.Name = "lbExisting";
            this.lbExisting.Size = new System.Drawing.Size(251, 13);
            this.lbExisting.TabIndex = 9;
            this.lbExisting.Text = "select a lower priority than that of existing entries.";
            // 
            // priorityNumericBox
            // 
            this.priorityNumericBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            this.priorityNumericBox.Location = new System.Drawing.Point(103, 227);
            this.priorityNumericBox.Maximum = new decimal(new int[] {
            99,
            0,
            0,
            0});
            this.priorityNumericBox.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.priorityNumericBox.Name = "priorityNumericBox";
            this.priorityNumericBox.Size = new System.Drawing.Size(47, 21);
            this.priorityNumericBox.TabIndex = 12;
            this.priorityNumericBox.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // additionsListBox
            // 
            this.additionsListBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.additionsListBox.FormattingEnabled = true;
            this.additionsListBox.Location = new System.Drawing.Point(4, 22);
            this.additionsListBox.Name = "additionsListBox";
            this.additionsListBox.SelectionMode = System.Windows.Forms.SelectionMode.None;
            this.additionsListBox.Size = new System.Drawing.Size(297, 199);
            this.additionsListBox.TabIndex = 0;
            // 
            // dependenciesSuppressionPage
            // 
            this.dependenciesSuppressionPage.Controls.Add(this.suppressionListBox);
            this.dependenciesSuppressionPage.Controls.Add(this.label2);
            this.dependenciesSuppressionPage.Controls.Add(warningPicture);
            this.dependenciesSuppressionPage.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dependenciesSuppressionPage.Location = new System.Drawing.Point(0, 0);
            this.dependenciesSuppressionPage.Name = "dependenciesSuppressionPage";
            this.dependenciesSuppressionPage.Size = new System.Drawing.Size(357, 273);
            this.dependenciesSuppressionPage.TabIndex = 0;
            this.dependenciesSuppressionPage.Text = "dependenciesSuppressionPage";
            // 
            // suppressionListBox
            // 
            this.suppressionListBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.suppressionListBox.FormattingEnabled = true;
            this.suppressionListBox.Location = new System.Drawing.Point(4, 41);
            this.suppressionListBox.Name = "suppressionListBox";
            this.suppressionListBox.SelectionMode = System.Windows.Forms.SelectionMode.None;
            this.suppressionListBox.Size = new System.Drawing.Size(350, 225);
            this.suppressionListBox.TabIndex = 2;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(41, 13);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(190, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "The following entries will be removed :";
            // 
            // uselessPrereqsSuppressionPage
            // 
            this.uselessPrereqsSuppressionPage.Controls.Add(this.uselessPrereqsListBox);
            this.uselessPrereqsSuppressionPage.Controls.Add(this.uselessPrereqsCheckbox);
            this.uselessPrereqsSuppressionPage.Dock = System.Windows.Forms.DockStyle.Fill;
            this.uselessPrereqsSuppressionPage.Location = new System.Drawing.Point(0, 0);
            this.uselessPrereqsSuppressionPage.Name = "uselessPrereqsSuppressionPage";
            this.uselessPrereqsSuppressionPage.Size = new System.Drawing.Size(304, 273);
            this.uselessPrereqsSuppressionPage.TabIndex = 1;
            this.uselessPrereqsSuppressionPage.Text = "uselessPrereqsSuppressionPage";
            // 
            // uselessPrereqsListBox
            // 
            this.uselessPrereqsListBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.uselessPrereqsListBox.FormattingEnabled = true;
            this.uselessPrereqsListBox.Location = new System.Drawing.Point(4, 28);
            this.uselessPrereqsListBox.Name = "uselessPrereqsListBox";
            this.uselessPrereqsListBox.SelectionMode = System.Windows.Forms.SelectionMode.None;
            this.uselessPrereqsListBox.Size = new System.Drawing.Size(297, 225);
            this.uselessPrereqsListBox.TabIndex = 1;
            // 
            // uselessPrereqsCheckbox
            // 
            this.uselessPrereqsCheckbox.AutoSize = true;
            this.uselessPrereqsCheckbox.Location = new System.Drawing.Point(4, 4);
            this.uselessPrereqsCheckbox.Name = "uselessPrereqsCheckbox";
            this.uselessPrereqsCheckbox.Size = new System.Drawing.Size(273, 17);
            this.uselessPrereqsCheckbox.TabIndex = 0;
            this.uselessPrereqsCheckbox.Text = "Remove the now useless prerequisites listed below.";
            this.uselessPrereqsCheckbox.UseVisualStyleBackColor = true;
            // 
            // PlanToOperationForm
            // 
            this.AcceptButton = this.nextButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(328, 403);
            this.Controls.Add(this.nextButton);
            this.Controls.Add(this.rootMultiPanel);
            this.Controls.Add(this.previousButton);
            this.Controls.Add(this.cancelButton);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "PlanToOperationForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "PlanToOperationForm";
            ((System.ComponentModel.ISupportInitialize)(warningPicture)).EndInit();
            this.rootMultiPanel.ResumeLayout(false);
            this.additionPage.ResumeLayout(false);
            this.additionPage.PerformLayout();
            this.priorityGroup.ResumeLayout(false);
            this.priorityGroup.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.priorityNumericBox)).EndInit();
            this.dependenciesSuppressionPage.ResumeLayout(false);
            this.dependenciesSuppressionPage.PerformLayout();
            this.uselessPrereqsSuppressionPage.ResumeLayout(false);
            this.uselessPrereqsSuppressionPage.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private EVEMon.Controls.MultiPanel rootMultiPanel;
        private EVEMon.Controls.MultiPanelPage additionPage;
        private EVEMon.Controls.MultiPanelPage dependenciesSuppressionPage;
        private EVEMon.Controls.MultiPanelPage uselessPrereqsSuppressionPage;
        private System.Windows.Forms.Button previousButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button nextButton;
        private System.Windows.Forms.ListBox additionsListBox;
        private System.Windows.Forms.ListBox suppressionListBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ListBox uselessPrereqsListBox;
        private System.Windows.Forms.CheckBox uselessPrereqsCheckbox;
        private System.Windows.Forms.GroupBox priorityGroup;
        private System.Windows.Forms.Label priorityLabel;
        private System.Windows.Forms.Label lbExisting;
        private System.Windows.Forms.NumericUpDown priorityNumericBox;
    }
}