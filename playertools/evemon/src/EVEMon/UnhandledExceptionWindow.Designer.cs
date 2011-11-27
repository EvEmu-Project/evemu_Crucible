namespace EVEMon
{
    partial class UnhandledExceptionWindow
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
            this.MainPanel = new System.Windows.Forms.Panel();
            this.TechnicalDetailsPanel = new System.Windows.Forms.Panel();
            this.TechnicalDetailsTextBox = new System.Windows.Forms.TextBox();
            this.BugPictureBox = new System.Windows.Forms.PictureBox();
            this.DescriptionLabel = new System.Windows.Forms.Label();
            this.WhatCanYouDoTableLayoutPanel = new System.Windows.Forms.TableLayoutPanel();
            this.KnownProblemsLinkLabel = new System.Windows.Forms.LinkLabel();
            this.ReportLinkLabel = new System.Windows.Forms.LinkLabel();
            this.LatestBinariesLinkLabel = new System.Windows.Forms.LinkLabel();
            this.KnownProblemsPictureBox = new System.Windows.Forms.PictureBox();
            this.LatestBinariesPictureBox = new System.Windows.Forms.PictureBox();
            this.ReportPictureBox = new System.Windows.Forms.PictureBox();
            this.WhatCanYouDoLabel = new System.Windows.Forms.Label();
            this.CopyDetailsLinkLabel = new System.Windows.Forms.LinkLabel();
            this.TitleLabel = new System.Windows.Forms.Label();
            this.TechnicalDetailsLabel = new System.Windows.Forms.Label();
            this.ResetButton = new System.Windows.Forms.Button();
            this.CloseButton = new System.Windows.Forms.Button();
            this.MainPanel.SuspendLayout();
            this.TechnicalDetailsPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.BugPictureBox)).BeginInit();
            this.WhatCanYouDoTableLayoutPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.KnownProblemsPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.LatestBinariesPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.ReportPictureBox)).BeginInit();
            this.SuspendLayout();
            // 
            // MainPanel
            // 
            this.MainPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.MainPanel.BackColor = System.Drawing.Color.White;
            this.MainPanel.Controls.Add(this.TechnicalDetailsPanel);
            this.MainPanel.Controls.Add(this.BugPictureBox);
            this.MainPanel.Controls.Add(this.DescriptionLabel);
            this.MainPanel.Controls.Add(this.WhatCanYouDoTableLayoutPanel);
            this.MainPanel.Controls.Add(this.WhatCanYouDoLabel);
            this.MainPanel.Controls.Add(this.CopyDetailsLinkLabel);
            this.MainPanel.Controls.Add(this.TitleLabel);
            this.MainPanel.Controls.Add(this.TechnicalDetailsLabel);
            this.MainPanel.ForeColor = System.Drawing.Color.Black;
            this.MainPanel.Location = new System.Drawing.Point(0, 0);
            this.MainPanel.Name = "MainPanel";
            this.MainPanel.Size = new System.Drawing.Size(583, 465);
            this.MainPanel.TabIndex = 0;
            // 
            // TechnicalDetailsPanel
            // 
            this.TechnicalDetailsPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.TechnicalDetailsPanel.AutoSize = true;
            this.TechnicalDetailsPanel.Controls.Add(this.TechnicalDetailsTextBox);
            this.TechnicalDetailsPanel.Location = new System.Drawing.Point(12, 104);
            this.TechnicalDetailsPanel.Name = "TechnicalDetailsPanel";
            this.TechnicalDetailsPanel.Size = new System.Drawing.Size(559, 205);
            this.TechnicalDetailsPanel.TabIndex = 13;
            // 
            // TechnicalDetailsTextBox
            // 
            this.TechnicalDetailsTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TechnicalDetailsTextBox.Location = new System.Drawing.Point(0, 0);
            this.TechnicalDetailsTextBox.Multiline = true;
            this.TechnicalDetailsTextBox.Name = "TechnicalDetailsTextBox";
            this.TechnicalDetailsTextBox.ReadOnly = true;
            this.TechnicalDetailsTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.TechnicalDetailsTextBox.Size = new System.Drawing.Size(559, 205);
            this.TechnicalDetailsTextBox.TabIndex = 1;
            // 
            // BugPictureBox
            // 
            this.BugPictureBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.BugPictureBox.Location = new System.Drawing.Point(487, 13);
            this.BugPictureBox.Name = "BugPictureBox";
            this.BugPictureBox.Size = new System.Drawing.Size(81, 65);
            this.BugPictureBox.TabIndex = 2;
            this.BugPictureBox.TabStop = false;
            // 
            // DescriptionLabel
            // 
            this.DescriptionLabel.AutoSize = true;
            this.DescriptionLabel.Location = new System.Drawing.Point(13, 35);
            this.DescriptionLabel.Name = "DescriptionLabel";
            this.DescriptionLabel.Size = new System.Drawing.Size(422, 26);
            this.DescriptionLabel.TabIndex = 12;
            this.DescriptionLabel.Text = "I think Little Fluffy is dead, Jimmy. \r\nEVEMon will be shut down. Restart and try" +
                " again to test whether the problem repeats.";
            // 
            // WhatCanYouDoTableLayoutPanel
            // 
            this.WhatCanYouDoTableLayoutPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.WhatCanYouDoTableLayoutPanel.ColumnCount = 2;
            this.WhatCanYouDoTableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 30F));
            this.WhatCanYouDoTableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.WhatCanYouDoTableLayoutPanel.Controls.Add(this.KnownProblemsLinkLabel, 1, 0);
            this.WhatCanYouDoTableLayoutPanel.Controls.Add(this.ReportLinkLabel, 1, 2);
            this.WhatCanYouDoTableLayoutPanel.Controls.Add(this.LatestBinariesLinkLabel, 1, 1);
            this.WhatCanYouDoTableLayoutPanel.Controls.Add(this.KnownProblemsPictureBox, 0, 0);
            this.WhatCanYouDoTableLayoutPanel.Controls.Add(this.LatestBinariesPictureBox, 0, 1);
            this.WhatCanYouDoTableLayoutPanel.Controls.Add(this.ReportPictureBox, 0, 2);
            this.WhatCanYouDoTableLayoutPanel.Location = new System.Drawing.Point(16, 341);
            this.WhatCanYouDoTableLayoutPanel.Name = "WhatCanYouDoTableLayoutPanel";
            this.WhatCanYouDoTableLayoutPanel.RowCount = 3;
            this.WhatCanYouDoTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33F));
            this.WhatCanYouDoTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33F));
            this.WhatCanYouDoTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 34F));
            this.WhatCanYouDoTableLayoutPanel.Size = new System.Drawing.Size(555, 104);
            this.WhatCanYouDoTableLayoutPanel.TabIndex = 11;
            // 
            // KnownProblemsLinkLabel
            // 
            this.KnownProblemsLinkLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.KnownProblemsLinkLabel.AutoSize = true;
            this.KnownProblemsLinkLabel.LinkArea = new System.Windows.Forms.LinkArea(12, 14);
            this.KnownProblemsLinkLabel.Location = new System.Drawing.Point(33, 0);
            this.KnownProblemsLinkLabel.Name = "KnownProblemsLinkLabel";
            this.KnownProblemsLinkLabel.Size = new System.Drawing.Size(519, 34);
            this.KnownProblemsLinkLabel.TabIndex = 10;
            this.KnownProblemsLinkLabel.TabStop = true;
            this.KnownProblemsLinkLabel.Text = "Look at our known problems page and its top five, there is probably a solution.";
            this.KnownProblemsLinkLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.KnownProblemsLinkLabel.UseCompatibleTextRendering = true;
            this.KnownProblemsLinkLabel.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.llblKnownProblems_LinkClicked);
            // 
            // ReportLinkLabel
            // 
            this.ReportLinkLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.ReportLinkLabel.AutoSize = true;
            this.ReportLinkLabel.LinkArea = new System.Windows.Forms.LinkArea(31, 9);
            this.ReportLinkLabel.Location = new System.Drawing.Point(33, 68);
            this.ReportLinkLabel.Name = "ReportLinkLabel";
            this.ReportLinkLabel.Size = new System.Drawing.Size(519, 36);
            this.ReportLinkLabel.TabIndex = 8;
            this.ReportLinkLabel.TabStop = true;
            this.ReportLinkLabel.Text = "If none of that worked, please report us the problem\r\n";
            this.ReportLinkLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.ReportLinkLabel.UseCompatibleTextRendering = true;
            this.ReportLinkLabel.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.llblReport_LinkClicked);
            // 
            // LatestBinariesLinkLabel
            // 
            this.LatestBinariesLinkLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.LatestBinariesLinkLabel.AutoSize = true;
            this.LatestBinariesLinkLabel.LinkArea = new System.Windows.Forms.LinkArea(88, 24);
            this.LatestBinariesLinkLabel.Location = new System.Drawing.Point(33, 34);
            this.LatestBinariesLinkLabel.Name = "LatestBinariesLinkLabel";
            this.LatestBinariesLinkLabel.Size = new System.Drawing.Size(519, 34);
            this.LatestBinariesLinkLabel.TabIndex = 11;
            this.LatestBinariesLinkLabel.TabStop = true;
            this.LatestBinariesLinkLabel.Text = "Delete your settings and cache (start menu > run > \"%APPDATA%\\EVEMon\").\r\nOr insta" +
                "ll the latest snapshot binaries (beta), the problem may have been solved.";
            this.LatestBinariesLinkLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.LatestBinariesLinkLabel.UseCompatibleTextRendering = true;
            this.LatestBinariesLinkLabel.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.llblLatestBinaries_LinkClicked);
            // 
            // KnownProblemsPictureBox
            // 
            this.KnownProblemsPictureBox.Image = global::EVEMon.Common.Properties.Resources.Problem;
            this.KnownProblemsPictureBox.Location = new System.Drawing.Point(3, 3);
            this.KnownProblemsPictureBox.Name = "KnownProblemsPictureBox";
            this.KnownProblemsPictureBox.Size = new System.Drawing.Size(24, 28);
            this.KnownProblemsPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.KnownProblemsPictureBox.TabIndex = 12;
            this.KnownProblemsPictureBox.TabStop = false;
            // 
            // LatestBinariesPictureBox
            // 
            this.LatestBinariesPictureBox.Image = global::EVEMon.Common.Properties.Resources.Settings;
            this.LatestBinariesPictureBox.Location = new System.Drawing.Point(3, 37);
            this.LatestBinariesPictureBox.Name = "LatestBinariesPictureBox";
            this.LatestBinariesPictureBox.Size = new System.Drawing.Size(24, 28);
            this.LatestBinariesPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.LatestBinariesPictureBox.TabIndex = 13;
            this.LatestBinariesPictureBox.TabStop = false;
            // 
            // ReportPictureBox
            // 
            this.ReportPictureBox.Image = global::EVEMon.Common.Properties.Resources.Comment;
            this.ReportPictureBox.Location = new System.Drawing.Point(3, 71);
            this.ReportPictureBox.Name = "ReportPictureBox";
            this.ReportPictureBox.Size = new System.Drawing.Size(24, 30);
            this.ReportPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.ReportPictureBox.TabIndex = 14;
            this.ReportPictureBox.TabStop = false;
            // 
            // WhatCanYouDoLabel
            // 
            this.WhatCanYouDoLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.WhatCanYouDoLabel.AutoSize = true;
            this.WhatCanYouDoLabel.Location = new System.Drawing.Point(16, 312);
            this.WhatCanYouDoLabel.Name = "WhatCanYouDoLabel";
            this.WhatCanYouDoLabel.Size = new System.Drawing.Size(97, 13);
            this.WhatCanYouDoLabel.TabIndex = 9;
            this.WhatCanYouDoLabel.Text = "What can you do ?";
            // 
            // CopyDetailsLinkLabel
            // 
            this.CopyDetailsLinkLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.CopyDetailsLinkLabel.AutoSize = true;
            this.CopyDetailsLinkLabel.Location = new System.Drawing.Point(505, 88);
            this.CopyDetailsLinkLabel.Name = "CopyDetailsLinkLabel";
            this.CopyDetailsLinkLabel.Size = new System.Drawing.Size(66, 13);
            this.CopyDetailsLinkLabel.TabIndex = 6;
            this.CopyDetailsLinkLabel.TabStop = true;
            this.CopyDetailsLinkLabel.Text = "Copy details";
            this.CopyDetailsLinkLabel.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.CopyDetailsLinkLabel_LinkClicked);
            // 
            // TitleLabel
            // 
            this.TitleLabel.AutoSize = true;
            this.TitleLabel.Location = new System.Drawing.Point(13, 13);
            this.TitleLabel.Name = "TitleLabel";
            this.TitleLabel.Size = new System.Drawing.Size(231, 13);
            this.TitleLabel.TabIndex = 0;
            this.TitleLabel.Text = "EVEMon has encountered an unexpected error";
            // 
            // TechnicalDetailsLabel
            // 
            this.TechnicalDetailsLabel.AutoSize = true;
            this.TechnicalDetailsLabel.Location = new System.Drawing.Point(12, 88);
            this.TechnicalDetailsLabel.Name = "TechnicalDetailsLabel";
            this.TechnicalDetailsLabel.Size = new System.Drawing.Size(149, 13);
            this.TechnicalDetailsLabel.TabIndex = 2;
            this.TechnicalDetailsLabel.Text = "Technical details of this error:";
            // 
            // ResetButton
            // 
            this.ResetButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.ResetButton.Location = new System.Drawing.Point(16, 471);
            this.ResetButton.Name = "ResetButton";
            this.ResetButton.Size = new System.Drawing.Size(111, 23);
            this.ResetButton.TabIndex = 4;
            this.ResetButton.Text = "Reset Settings";
            this.ResetButton.UseVisualStyleBackColor = true;
            this.ResetButton.Click += new System.EventHandler(this.ResetButtonLinkLabel_Click);
            // 
            // CloseButton
            // 
            this.CloseButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.CloseButton.Location = new System.Drawing.Point(465, 471);
            this.CloseButton.Name = "CloseButton";
            this.CloseButton.Size = new System.Drawing.Size(106, 23);
            this.CloseButton.TabIndex = 5;
            this.CloseButton.Text = "Close EVEMon";
            this.CloseButton.UseVisualStyleBackColor = true;
            this.CloseButton.Click += new System.EventHandler(this.CloseButton_Click);
            // 
            // UnhandledExceptionWindow
            // 
            this.AcceptButton = this.CloseButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(583, 506);
            this.ControlBox = false;
            this.Controls.Add(this.CloseButton);
            this.Controls.Add(this.ResetButton);
            this.Controls.Add(this.MainPanel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "UnhandledExceptionWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "EVEMon Error";
            this.Load += new System.EventHandler(this.UnhandledExceptionWindow_Load);
            this.MainPanel.ResumeLayout(false);
            this.MainPanel.PerformLayout();
            this.TechnicalDetailsPanel.ResumeLayout(false);
            this.TechnicalDetailsPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.BugPictureBox)).EndInit();
            this.WhatCanYouDoTableLayoutPanel.ResumeLayout(false);
            this.WhatCanYouDoTableLayoutPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.KnownProblemsPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.LatestBinariesPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.ReportPictureBox)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel MainPanel;
        private System.Windows.Forms.Label TitleLabel;
        private System.Windows.Forms.TextBox TechnicalDetailsTextBox;
        private System.Windows.Forms.Label TechnicalDetailsLabel;
        private System.Windows.Forms.Button ResetButton;
        private System.Windows.Forms.Button CloseButton;
        private System.Windows.Forms.PictureBox BugPictureBox;
        private System.Windows.Forms.LinkLabel CopyDetailsLinkLabel;
        private System.Windows.Forms.Label WhatCanYouDoLabel;
        private System.Windows.Forms.TableLayoutPanel WhatCanYouDoTableLayoutPanel;
        private System.Windows.Forms.LinkLabel KnownProblemsLinkLabel;
        private System.Windows.Forms.Label DescriptionLabel;
        private System.Windows.Forms.LinkLabel ReportLinkLabel;
        private System.Windows.Forms.LinkLabel LatestBinariesLinkLabel;
        private System.Windows.Forms.PictureBox KnownProblemsPictureBox;
        private System.Windows.Forms.PictureBox LatestBinariesPictureBox;
        private System.Windows.Forms.PictureBox ReportPictureBox;
        private System.Windows.Forms.Panel TechnicalDetailsPanel;
    }
}