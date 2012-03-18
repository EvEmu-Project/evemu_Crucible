namespace EVEMon.SettingsUI
{
    partial class TrayPopupConfigForm
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TrayPopupConfigForm));
            this.panel1 = new System.Windows.Forms.Panel();
            this.btnUseDefaults = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnOK = new System.Windows.Forms.Button();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tpGeneral = new System.Windows.Forms.TabPage();
            this.flowLayoutPanel3 = new System.Windows.Forms.FlowLayoutPanel();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.cbIndentGroupedAccounts = new System.Windows.Forms.CheckBox();
            this.cbGroupBy = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.cbHideNotTraining = new System.Windows.Forms.CheckBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.cbDisplayOrder2 = new System.Windows.Forms.ComboBox();
            this.lblDisplayOrder1 = new System.Windows.Forms.Label();
            this.lblDisplayOrder2 = new System.Windows.Forms.Label();
            this.cbDisplayOrder1 = new System.Windows.Forms.ComboBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.cbShowSkillQueueTrainingTime = new System.Windows.Forms.CheckBox();
            this.cbHighLightConflicts = new System.Windows.Forms.CheckBox();
            this.cbShowSkill = new System.Windows.Forms.CheckBox();
            this.cbShowCompletionTime = new System.Windows.Forms.CheckBox();
            this.cbShowTimeToCompletion = new System.Windows.Forms.CheckBox();
            this.cbPortraitSize = new System.Windows.Forms.ComboBox();
            this.cbShowWallet = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.cbShowPortrait = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.cbUseIncreasedContrast = new System.Windows.Forms.CheckBox();
            this.cbShowEveTime = new System.Windows.Forms.CheckBox();
            this.cbShowWarning = new System.Windows.Forms.CheckBox();
            this.cbShowServerStatus = new System.Windows.Forms.CheckBox();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.ttHelp = new System.Windows.Forms.ToolTip(this.components);
            this.panel1.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tpGeneral.SuspendLayout();
            this.flowLayoutPanel3.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.btnUseDefaults);
            this.panel1.Controls.Add(this.btnCancel);
            this.panel1.Controls.Add(this.btnOK);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 420);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(382, 45);
            this.panel1.TabIndex = 0;
            // 
            // btnUseDefaults
            // 
            this.btnUseDefaults.Location = new System.Drawing.Point(12, 10);
            this.btnUseDefaults.Name = "btnUseDefaults";
            this.btnUseDefaults.Size = new System.Drawing.Size(81, 23);
            this.btnUseDefaults.TabIndex = 2;
            this.btnUseDefaults.Text = "Use Defaults";
            this.btnUseDefaults.UseVisualStyleBackColor = true;
            this.btnUseDefaults.Click += new System.EventHandler(this.btnUseDefaults_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.Location = new System.Drawing.Point(295, 10);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 1;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // btnOK
            // 
            this.btnOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOK.Location = new System.Drawing.Point(214, 10);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 0;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tpGeneral);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(382, 420);
            this.tabControl1.TabIndex = 1;
            // 
            // tpGeneral
            // 
            this.tpGeneral.Controls.Add(this.flowLayoutPanel3);
            this.tpGeneral.Controls.Add(this.flowLayoutPanel1);
            this.tpGeneral.Location = new System.Drawing.Point(4, 22);
            this.tpGeneral.Name = "tpGeneral";
            this.tpGeneral.Padding = new System.Windows.Forms.Padding(3);
            this.tpGeneral.Size = new System.Drawing.Size(374, 394);
            this.tpGeneral.TabIndex = 0;
            this.tpGeneral.Text = "General";
            this.tpGeneral.UseVisualStyleBackColor = true;
            // 
            // flowLayoutPanel3
            // 
            this.flowLayoutPanel3.Controls.Add(this.groupBox1);
            this.flowLayoutPanel3.Controls.Add(this.groupBox4);
            this.flowLayoutPanel3.Controls.Add(this.groupBox2);
            this.flowLayoutPanel3.Controls.Add(this.groupBox3);
            this.flowLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.flowLayoutPanel3.Location = new System.Drawing.Point(3, 3);
            this.flowLayoutPanel3.Name = "flowLayoutPanel3";
            this.flowLayoutPanel3.Size = new System.Drawing.Size(368, 388);
            this.flowLayoutPanel3.TabIndex = 18;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.cbIndentGroupedAccounts);
            this.groupBox1.Controls.Add(this.cbGroupBy);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.cbHideNotTraining);
            this.groupBox1.Location = new System.Drawing.Point(3, 3);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(362, 96);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Layout";
            // 
            // cbIndentGroupedAccounts
            // 
            this.cbIndentGroupedAccounts.AutoSize = true;
            this.cbIndentGroupedAccounts.Location = new System.Drawing.Point(132, 72);
            this.cbIndentGroupedAccounts.Name = "cbIndentGroupedAccounts";
            this.cbIndentGroupedAccounts.Size = new System.Drawing.Size(148, 17);
            this.cbIndentGroupedAccounts.TabIndex = 19;
            this.cbIndentGroupedAccounts.Text = "Indent Grouped Accounts";
            this.ttHelp.SetToolTip(this.cbIndentGroupedAccounts, "When enabled, characters will be indented\r\nunder the first character in each acco" +
                    "unt group");
            this.cbIndentGroupedAccounts.UseVisualStyleBackColor = true;
            // 
            // cbGroupBy
            // 
            this.cbGroupBy.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbGroupBy.FormattingEnabled = true;
            this.cbGroupBy.Location = new System.Drawing.Point(132, 45);
            this.cbGroupBy.Name = "cbGroupBy";
            this.cbGroupBy.Size = new System.Drawing.Size(221, 21);
            this.cbGroupBy.TabIndex = 18;
            this.ttHelp.SetToolTip(this.cbGroupBy, resources.GetString("cbGroupBy.ToolTip"));
            this.cbGroupBy.SelectedIndexChanged += new System.EventHandler(this.cbGroupBy_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 48);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(111, 13);
            this.label1.TabIndex = 17;
            this.label1.Text = "Group Characters by:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // cbHideNotTraining
            // 
            this.cbHideNotTraining.AutoSize = true;
            this.cbHideNotTraining.Location = new System.Drawing.Point(6, 20);
            this.cbHideNotTraining.Name = "cbHideNotTraining";
            this.cbHideNotTraining.Size = new System.Drawing.Size(157, 17);
            this.cbHideNotTraining.TabIndex = 12;
            this.cbHideNotTraining.Text = "Hide Characters not training";
            this.ttHelp.SetToolTip(this.cbHideNotTraining, "When enabled, only characters currently\r\ntraining a skill will be displayed");
            this.cbHideNotTraining.UseVisualStyleBackColor = true;
            this.cbHideNotTraining.CheckedChanged += new System.EventHandler(this.cbHideNotTraining_CheckedChanged);
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.cbDisplayOrder2);
            this.groupBox4.Controls.Add(this.lblDisplayOrder1);
            this.groupBox4.Controls.Add(this.lblDisplayOrder2);
            this.groupBox4.Controls.Add(this.cbDisplayOrder1);
            this.groupBox4.Location = new System.Drawing.Point(3, 105);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(362, 79);
            this.groupBox4.TabIndex = 17;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Display Order";
            // 
            // cbDisplayOrder2
            // 
            this.cbDisplayOrder2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDisplayOrder2.FormattingEnabled = true;
            this.cbDisplayOrder2.Location = new System.Drawing.Point(132, 48);
            this.cbDisplayOrder2.Name = "cbDisplayOrder2";
            this.cbDisplayOrder2.Size = new System.Drawing.Size(221, 21);
            this.cbDisplayOrder2.TabIndex = 22;
            this.ttHelp.SetToolTip(this.cbDisplayOrder2, "Controls the display order of characters, dependent on grouping.");
            // 
            // lblDisplayOrder1
            // 
            this.lblDisplayOrder1.AutoSize = true;
            this.lblDisplayOrder1.Location = new System.Drawing.Point(12, 23);
            this.lblDisplayOrder1.Name = "lblDisplayOrder1";
            this.lblDisplayOrder1.Size = new System.Drawing.Size(114, 13);
            this.lblDisplayOrder1.TabIndex = 19;
            this.lblDisplayOrder1.Text = "Characters in training:";
            this.lblDisplayOrder1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lblDisplayOrder2
            // 
            this.lblDisplayOrder2.AutoSize = true;
            this.lblDisplayOrder2.Location = new System.Drawing.Point(4, 51);
            this.lblDisplayOrder2.Name = "lblDisplayOrder2";
            this.lblDisplayOrder2.Size = new System.Drawing.Size(122, 13);
            this.lblDisplayOrder2.TabIndex = 21;
            this.lblDisplayOrder2.Text = "Characters not training:";
            this.lblDisplayOrder2.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // cbDisplayOrder1
            // 
            this.cbDisplayOrder1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDisplayOrder1.FormattingEnabled = true;
            this.cbDisplayOrder1.Items.AddRange(new object[] {
            "Alphabetical, ascending",
            "Alphabetical, descending",
            "Training time remaining, ascending",
            "Training time remaining, descending"});
            this.cbDisplayOrder1.Location = new System.Drawing.Point(132, 20);
            this.cbDisplayOrder1.Name = "cbDisplayOrder1";
            this.cbDisplayOrder1.Size = new System.Drawing.Size(221, 21);
            this.cbDisplayOrder1.TabIndex = 20;
            this.ttHelp.SetToolTip(this.cbDisplayOrder1, "Controls the display order of characters, dependent on grouping.");
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.cbShowSkillQueueTrainingTime);
            this.groupBox2.Controls.Add(this.cbHighLightConflicts);
            this.groupBox2.Controls.Add(this.cbShowSkill);
            this.groupBox2.Controls.Add(this.cbShowCompletionTime);
            this.groupBox2.Controls.Add(this.cbShowTimeToCompletion);
            this.groupBox2.Controls.Add(this.cbPortraitSize);
            this.groupBox2.Controls.Add(this.cbShowWallet);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.cbShowPortrait);
            this.groupBox2.Location = new System.Drawing.Point(3, 190);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(362, 121);
            this.groupBox2.TabIndex = 15;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Character Details";
            // 
            // cbShowSkillQueueTrainingTime
            // 
            this.cbShowSkillQueueTrainingTime.AutoSize = true;
            this.cbShowSkillQueueTrainingTime.Location = new System.Drawing.Point(9, 88);
            this.cbShowSkillQueueTrainingTime.Name = "cbShowSkillQueueTrainingTime";
            this.cbShowSkillQueueTrainingTime.Size = new System.Drawing.Size(173, 17);
            this.cbShowSkillQueueTrainingTime.TabIndex = 35;
            this.cbShowSkillQueueTrainingTime.Text = "Show Skill Queue Training Time";
            this.ttHelp.SetToolTip(this.cbShowSkillQueueTrainingTime, "When enabled, shows the character\'s\r\nskill queue training time");
            this.cbShowSkillQueueTrainingTime.UseVisualStyleBackColor = true;
            // 
            // cbHighLightConflicts
            // 
            this.cbHighLightConflicts.AutoSize = true;
            this.cbHighLightConflicts.Location = new System.Drawing.Point(204, 20);
            this.cbHighLightConflicts.Name = "cbHighLightConflicts";
            this.cbHighLightConflicts.Size = new System.Drawing.Size(157, 17);
            this.cbHighLightConflicts.TabIndex = 34;
            this.cbHighLightConflicts.Text = "Highlight Schedule Conflicts";
            this.ttHelp.SetToolTip(this.cbHighLightConflicts, "When enabled, skills with a completion time\r\nconflicting with a blocked scheduler" +
                    " entry\r\nwill be highlighted in red.");
            this.cbHighLightConflicts.UseVisualStyleBackColor = true;
            // 
            // cbShowSkill
            // 
            this.cbShowSkill.AutoSize = true;
            this.cbShowSkill.Location = new System.Drawing.Point(9, 20);
            this.cbShowSkill.Name = "cbShowSkill";
            this.cbShowSkill.Size = new System.Drawing.Size(124, 17);
            this.cbShowSkill.TabIndex = 31;
            this.cbShowSkill.Text = "Show Skill in Training";
            this.ttHelp.SetToolTip(this.cbShowSkill, "When enabled, shows the currently\r\ntraining skill name and level");
            this.cbShowSkill.UseVisualStyleBackColor = true;
            // 
            // cbShowCompletionTime
            // 
            this.cbShowCompletionTime.AutoSize = true;
            this.cbShowCompletionTime.Location = new System.Drawing.Point(9, 66);
            this.cbShowCompletionTime.Name = "cbShowCompletionTime";
            this.cbShowCompletionTime.Size = new System.Drawing.Size(133, 17);
            this.cbShowCompletionTime.TabIndex = 33;
            this.cbShowCompletionTime.Text = "Show Completion Time";
            this.ttHelp.SetToolTip(this.cbShowCompletionTime, "When enabled, shows the estimated completion\r\ndate and time of the  currently tra" +
                    "ining skill");
            this.cbShowCompletionTime.UseVisualStyleBackColor = true;
            // 
            // cbShowTimeToCompletion
            // 
            this.cbShowTimeToCompletion.AutoSize = true;
            this.cbShowTimeToCompletion.Location = new System.Drawing.Point(9, 43);
            this.cbShowTimeToCompletion.Name = "cbShowTimeToCompletion";
            this.cbShowTimeToCompletion.Size = new System.Drawing.Size(146, 17);
            this.cbShowTimeToCompletion.TabIndex = 32;
            this.cbShowTimeToCompletion.Text = "Show Time to Completion";
            this.ttHelp.SetToolTip(this.cbShowTimeToCompletion, "When enabled, shows the time remaining\r\n until training is completed");
            this.cbShowTimeToCompletion.UseVisualStyleBackColor = true;
            // 
            // cbPortraitSize
            // 
            this.cbPortraitSize.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbPortraitSize.FormattingEnabled = true;
            this.cbPortraitSize.Location = new System.Drawing.Point(255, 89);
            this.cbPortraitSize.Name = "cbPortraitSize";
            this.cbPortraitSize.Size = new System.Drawing.Size(69, 21);
            this.cbPortraitSize.TabIndex = 28;
            this.ttHelp.SetToolTip(this.cbPortraitSize, "Selects the image size, in pixels, to be used\r\nfor the character portrait");
            // 
            // cbShowWallet
            // 
            this.cbShowWallet.AutoSize = true;
            this.cbShowWallet.Location = new System.Drawing.Point(204, 43);
            this.cbShowWallet.Name = "cbShowWallet";
            this.cbShowWallet.Size = new System.Drawing.Size(125, 17);
            this.cbShowWallet.TabIndex = 30;
            this.cbShowWallet.Text = "Show Wallet Balance";
            this.ttHelp.SetToolTip(this.cbShowWallet, "When enabled, shows the character\'s wallet balance");
            this.cbShowWallet.UseVisualStyleBackColor = true;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(219, 92);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(30, 13);
            this.label3.TabIndex = 27;
            this.label3.Text = "Size:";
            // 
            // cbShowPortrait
            // 
            this.cbShowPortrait.AutoSize = true;
            this.cbShowPortrait.Location = new System.Drawing.Point(204, 66);
            this.cbShowPortrait.Name = "cbShowPortrait";
            this.cbShowPortrait.Size = new System.Drawing.Size(142, 17);
            this.cbShowPortrait.TabIndex = 26;
            this.cbShowPortrait.Text = "Show Character Portrait";
            this.ttHelp.SetToolTip(this.cbShowPortrait, "When enabled, shows the character\'s portrait\r\nas a thumbnail alongside the charac" +
                    "ter\'s name");
            this.cbShowPortrait.UseVisualStyleBackColor = true;
            this.cbShowPortrait.CheckedChanged += new System.EventHandler(this.cbShowPortrait_CheckedChanged);
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.cbUseIncreasedContrast);
            this.groupBox3.Controls.Add(this.cbShowEveTime);
            this.groupBox3.Controls.Add(this.cbShowWarning);
            this.groupBox3.Controls.Add(this.cbShowServerStatus);
            this.groupBox3.Location = new System.Drawing.Point(3, 317);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(362, 64);
            this.groupBox3.TabIndex = 16;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Miscellaneous";
            // 
            // cbUseIncreasedContrast
            // 
            this.cbUseIncreasedContrast.AutoSize = true;
            this.cbUseIncreasedContrast.Location = new System.Drawing.Point(204, 43);
            this.cbUseIncreasedContrast.Name = "cbUseIncreasedContrast";
            this.cbUseIncreasedContrast.Size = new System.Drawing.Size(137, 17);
            this.cbUseIncreasedContrast.TabIndex = 21;
            this.cbUseIncreasedContrast.Text = "Use Increased Contrast";
            this.ttHelp.SetToolTip(this.cbUseIncreasedContrast, "When enabled, increases the contrast of the shown info");
            this.cbUseIncreasedContrast.UseVisualStyleBackColor = true;
            // 
            // cbShowEveTime
            // 
            this.cbShowEveTime.AutoSize = true;
            this.cbShowEveTime.Location = new System.Drawing.Point(6, 43);
            this.cbShowEveTime.Name = "cbShowEveTime";
            this.cbShowEveTime.Size = new System.Drawing.Size(103, 17);
            this.cbShowEveTime.TabIndex = 20;
            this.cbShowEveTime.Text = "Show EVE Time";
            this.ttHelp.SetToolTip(this.cbShowEveTime, "When enabled, displays the current EVE time");
            this.cbShowEveTime.UseVisualStyleBackColor = true;
            // 
            // cbShowWarning
            // 
            this.cbShowWarning.AutoSize = true;
            this.cbShowWarning.Location = new System.Drawing.Point(6, 20);
            this.cbShowWarning.Name = "cbShowWarning";
            this.cbShowWarning.Size = new System.Drawing.Size(193, 17);
            this.cbShowWarning.TabIndex = 19;
            this.cbShowWarning.Text = "Show non-training account warning";
            this.ttHelp.SetToolTip(this.cbShowWarning, "When enabled, shows a warning message when\r\nno characters on an account are train" +
                    "ing a skill.");
            this.cbShowWarning.UseVisualStyleBackColor = true;
            // 
            // cbShowServerStatus
            // 
            this.cbShowServerStatus.AutoSize = true;
            this.cbShowServerStatus.Location = new System.Drawing.Point(204, 20);
            this.cbShowServerStatus.Name = "cbShowServerStatus";
            this.cbShowServerStatus.Size = new System.Drawing.Size(120, 17);
            this.cbShowServerStatus.TabIndex = 14;
            this.cbShowServerStatus.Text = "Show Server Status";
            this.ttHelp.SetToolTip(this.cbShowServerStatus, "When enabled, displays the current\r\nTranquility server status");
            this.cbShowServerStatus.UseVisualStyleBackColor = true;
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.AutoSize = true;
            this.flowLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.flowLayoutPanel1.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.flowLayoutPanel1.Location = new System.Drawing.Point(5, 95);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(0, 0);
            this.flowLayoutPanel1.TabIndex = 17;
            // 
            // TrayPopupConfigForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.ClientSize = new System.Drawing.Size(382, 465);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.panel1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "TrayPopupConfigForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Tray Icon Popup Configuration";
            this.panel1.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.tpGeneral.ResumeLayout(false);
            this.tpGeneral.PerformLayout();
            this.flowLayoutPanel3.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tpGeneral;
        private System.Windows.Forms.CheckBox cbShowServerStatus;
        private System.Windows.Forms.ToolTip ttHelp;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.CheckBox cbHideNotTraining;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel3;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.CheckBox cbShowWallet;
        private System.Windows.Forms.CheckBox cbShowSkill;
        private System.Windows.Forms.CheckBox cbShowCompletionTime;
        private System.Windows.Forms.CheckBox cbShowTimeToCompletion;
        private System.Windows.Forms.ComboBox cbPortraitSize;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox cbShowPortrait;
        private System.Windows.Forms.ComboBox cbGroupBy;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox cbDisplayOrder2;
        private System.Windows.Forms.Label lblDisplayOrder2;
        private System.Windows.Forms.ComboBox cbDisplayOrder1;
        private System.Windows.Forms.Label lblDisplayOrder1;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.CheckBox cbShowWarning;
        private System.Windows.Forms.Button btnUseDefaults;
        private System.Windows.Forms.CheckBox cbHighLightConflicts;
        private System.Windows.Forms.CheckBox cbShowEveTime;
		private System.Windows.Forms.CheckBox cbShowSkillQueueTrainingTime;
		private System.Windows.Forms.CheckBox cbIndentGroupedAccounts;
        private System.Windows.Forms.CheckBox cbUseIncreasedContrast;

    }
}