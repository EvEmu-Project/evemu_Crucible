namespace EVEMon
{
    partial class SkillsPieChart
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
            this.skillPieChartControl = new EVEMon.PieChart.PieChartControl();
            this.pieControlPanel = new System.Windows.Forms.Panel();
            this.mergeMinorCheck = new System.Windows.Forms.CheckBox();
            this.saveButton = new System.Windows.Forms.Button();
            this.closeButton = new System.Windows.Forms.Button();
            this.pieAngle = new System.Windows.Forms.NumericUpDown();
            this.pieHeight = new System.Windows.Forms.NumericUpDown();
            this.pieAngleLabel = new System.Windows.Forms.Label();
            this.pieHeightLabel = new System.Windows.Forms.Label();
            this.sortBySizeCheck = new System.Windows.Forms.CheckBox();
            this.m_colorDialog = new System.Windows.Forms.ColorDialog();
            this.savePieDialog = new System.Windows.Forms.SaveFileDialog();
            this.planSelector = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.pieControlPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pieAngle)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pieHeight)).BeginInit();
            this.SuspendLayout();
            // 
            // skillPieChartControl
            // 
            this.skillPieChartControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.skillPieChartControl.BackColor = System.Drawing.Color.White;
            this.skillPieChartControl.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.skillPieChartControl.Colors = null;
            this.skillPieChartControl.Location = new System.Drawing.Point(12, 12);
            this.skillPieChartControl.Name = "skillPieChartControl";
            this.skillPieChartControl.Size = new System.Drawing.Size(642, 377);
            this.skillPieChartControl.TabIndex = 0;
            this.skillPieChartControl.ToolTips = null;
            this.skillPieChartControl.DoubleClick += new System.EventHandler(this.skillPieChartControl_DoubleClick);
            // 
            // pieControlPanel
            // 
            this.pieControlPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.pieControlPanel.Controls.Add(this.mergeMinorCheck);
            this.pieControlPanel.Controls.Add(this.saveButton);
            this.pieControlPanel.Controls.Add(this.closeButton);
            this.pieControlPanel.Controls.Add(this.pieAngle);
            this.pieControlPanel.Controls.Add(this.pieHeight);
            this.pieControlPanel.Controls.Add(this.pieAngleLabel);
            this.pieControlPanel.Controls.Add(this.pieHeightLabel);
            this.pieControlPanel.Controls.Add(this.sortBySizeCheck);
            this.pieControlPanel.Location = new System.Drawing.Point(12, 418);
            this.pieControlPanel.Name = "pieControlPanel";
            this.pieControlPanel.Size = new System.Drawing.Size(642, 26);
            this.pieControlPanel.TabIndex = 1;
            // 
            // mergeMinorCheck
            // 
            this.mergeMinorCheck.AutoSize = true;
            this.mergeMinorCheck.Location = new System.Drawing.Point(352, 5);
            this.mergeMinorCheck.Name = "mergeMinorCheck";
            this.mergeMinorCheck.Size = new System.Drawing.Size(121, 17);
            this.mergeMinorCheck.TabIndex = 7;
            this.mergeMinorCheck.Text = "Merge minor groups";
            this.mergeMinorCheck.CheckedChanged += new System.EventHandler(this.mergeMinorCheck_CheckedChanged);
            // 
            // saveButton
            // 
            this.saveButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.saveButton.Location = new System.Drawing.Point(485, 3);
            this.saveButton.Name = "saveButton";
            this.saveButton.Size = new System.Drawing.Size(76, 24);
            this.saveButton.TabIndex = 5;
            this.saveButton.Text = "Save";
            this.saveButton.UseVisualStyleBackColor = true;
            this.saveButton.Click += new System.EventHandler(this.saveButton_Click);
            // 
            // closeButton
            // 
            this.closeButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.closeButton.Location = new System.Drawing.Point(567, 3);
            this.closeButton.Name = "closeButton";
            this.closeButton.Size = new System.Drawing.Size(75, 23);
            this.closeButton.TabIndex = 4;
            this.closeButton.Text = "Close";
            this.closeButton.UseVisualStyleBackColor = true;
            this.closeButton.Click += new System.EventHandler(this.closeButton_Click);
            // 
            // pieAngle
            // 
            this.pieAngle.Increment = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.pieAngle.Location = new System.Drawing.Point(198, 3);
            this.pieAngle.Maximum = new decimal(new int[] {
            360,
            0,
            0,
            0});
            this.pieAngle.Minimum = new decimal(new int[] {
            360,
            0,
            0,
            -2147483648});
            this.pieAngle.Name = "pieAngle";
            this.pieAngle.Size = new System.Drawing.Size(52, 21);
            this.pieAngle.TabIndex = 3;
            this.pieAngle.Value = new decimal(new int[] {
            30,
            0,
            0,
            -2147483648});
            this.pieAngle.ValueChanged += new System.EventHandler(this.pieAngle_ValueChanged);
            // 
            // pieHeight
            // 
            this.pieHeight.DecimalPlaces = 2;
            this.pieHeight.Increment = new decimal(new int[] {
            5,
            0,
            0,
            131072});
            this.pieHeight.Location = new System.Drawing.Point(64, 3);
            this.pieHeight.Maximum = new decimal(new int[] {
            5,
            0,
            0,
            65536});
            this.pieHeight.Name = "pieHeight";
            this.pieHeight.Size = new System.Drawing.Size(54, 21);
            this.pieHeight.TabIndex = 2;
            this.pieHeight.Value = new decimal(new int[] {
            15,
            0,
            0,
            131072});
            this.pieHeight.ValueChanged += new System.EventHandler(this.pieHeight_ValueChanged);
            // 
            // pieAngleLabel
            // 
            this.pieAngleLabel.AutoSize = true;
            this.pieAngleLabel.Location = new System.Drawing.Point(129, 5);
            this.pieAngleLabel.Name = "pieAngleLabel";
            this.pieAngleLabel.Size = new System.Drawing.Size(66, 13);
            this.pieAngleLabel.TabIndex = 1;
            this.pieAngleLabel.Text = "Initial angle:";
            // 
            // pieHeightLabel
            // 
            this.pieHeightLabel.AutoSize = true;
            this.pieHeightLabel.Location = new System.Drawing.Point(1, 5);
            this.pieHeightLabel.Name = "pieHeightLabel";
            this.pieHeightLabel.Size = new System.Drawing.Size(58, 13);
            this.pieHeightLabel.TabIndex = 0;
            this.pieHeightLabel.Text = "Pie height:";
            // 
            // sortBySizeCheck
            // 
            this.sortBySizeCheck.AutoSize = true;
            this.sortBySizeCheck.Location = new System.Drawing.Point(264, 5);
            this.sortBySizeCheck.Name = "sortBySizeCheck";
            this.sortBySizeCheck.Size = new System.Drawing.Size(82, 17);
            this.sortBySizeCheck.TabIndex = 6;
            this.sortBySizeCheck.Text = "Sort by size";
            this.sortBySizeCheck.CheckedChanged += new System.EventHandler(this.sortBySizeCheck_CheckedChanged);
            // 
            // savePieDialog
            // 
            this.savePieDialog.DefaultExt = "png";
            this.savePieDialog.Filter = "PNG Image|*.png";
            // 
            // planSelector
            // 
            this.planSelector.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.planSelector.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.planSelector.FormattingEnabled = true;
            this.planSelector.Items.AddRange(new object[] {
            "<Current Skillpoints>"});
            this.planSelector.Location = new System.Drawing.Point(175, 391);
            this.planSelector.Name = "planSelector";
            this.planSelector.Size = new System.Drawing.Size(183, 21);
            this.planSelector.TabIndex = 2;
            this.planSelector.SelectedIndexChanged += new System.EventHandler(this.planSelector_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 394);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(157, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Projected Skillpoints using Plan:";
            // 
            // SkillsPieChart
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(666, 456);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.planSelector);
            this.Controls.Add(this.pieControlPanel);
            this.Controls.Add(this.skillPieChartControl);
            this.MinimumSize = new System.Drawing.Size(674, 490);
            this.Name = "SkillsPieChart";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "SkillsPieChart";
            this.pieControlPanel.ResumeLayout(false);
            this.pieControlPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pieAngle)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pieHeight)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private EVEMon.PieChart.PieChartControl skillPieChartControl;
        private System.Windows.Forms.Panel pieControlPanel;
        private System.Windows.Forms.Label pieAngleLabel;
        private System.Windows.Forms.Label pieHeightLabel;
        private System.Windows.Forms.NumericUpDown pieAngle;
        private System.Windows.Forms.NumericUpDown pieHeight;
        private System.Windows.Forms.Button closeButton;
        private System.Windows.Forms.ColorDialog m_colorDialog;
        private System.Windows.Forms.Button saveButton;
        private System.Windows.Forms.SaveFileDialog savePieDialog;
        private System.Windows.Forms.CheckBox sortBySizeCheck;
        private System.Windows.Forms.ComboBox planSelector;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox mergeMinorCheck;
    }
}
