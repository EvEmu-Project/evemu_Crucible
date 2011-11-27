namespace EVEMon
{
    partial class OverviewItem
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
            this.pbCharacterPortrait = new EVEMon.Common.Controls.CharacterPortrait();
            this.lblSkillQueueTrainingTime = new EVEMon.Controls.OverviewLabel();
            this.lblCompletionTime = new EVEMon.Controls.OverviewLabel();
            this.lblCharName = new EVEMon.Controls.OverviewLabel();
            this.lblSkillInTraining = new EVEMon.Controls.OverviewLabel();
            this.lblRemainingTime = new EVEMon.Controls.OverviewLabel();
            this.lblBalance = new EVEMon.Controls.OverviewLabel();
            this.SuspendLayout();
            // 
            // pbCharacterPortrait
            // 
            this.pbCharacterPortrait.Character = null;
            this.pbCharacterPortrait.CharacterID = ((long)(-1));
            this.pbCharacterPortrait.Enabled = false;
            this.pbCharacterPortrait.Location = new System.Drawing.Point(9, 11);
            this.pbCharacterPortrait.Name = "pbCharacterPortrait";
            this.pbCharacterPortrait.Size = new System.Drawing.Size(92, 92);
            this.pbCharacterPortrait.TabIndex = 0;
            this.pbCharacterPortrait.TabStop = false;
            // 
            // lblSkillQueueFreeRoom
            // 
            this.lblSkillQueueTrainingTime.AutoEllipsis = true;
            this.lblSkillQueueTrainingTime.BackColor = System.Drawing.Color.Transparent;
            this.lblSkillQueueTrainingTime.Enabled = false;
            this.lblSkillQueueTrainingTime.ForeColor = System.Drawing.Color.Red;
            this.lblSkillQueueTrainingTime.Location = new System.Drawing.Point(107, 92);
            this.lblSkillQueueTrainingTime.Name = "lblSkillQueueFreeRoom";
            this.lblSkillQueueTrainingTime.Size = new System.Drawing.Size(186, 13);
            this.lblSkillQueueTrainingTime.TabIndex = 12;
            this.lblSkillQueueTrainingTime.Text = "23h, 59m free room in skill queue";
            // 
            // lblCompletionTime
            // 
            this.lblCompletionTime.AutoEllipsis = true;
            this.lblCompletionTime.BackColor = System.Drawing.Color.Transparent;
            this.lblCompletionTime.Enabled = false;
            this.lblCompletionTime.ForeColor = System.Drawing.Color.DimGray;
            this.lblCompletionTime.Location = new System.Drawing.Point(107, 79);
            this.lblCompletionTime.Name = "lblCompletionTime";
            this.lblCompletionTime.Size = new System.Drawing.Size(186, 13);
            this.lblCompletionTime.TabIndex = 11;
            this.lblCompletionTime.Text = "Monday 5, 18:32:15";
            // 
            // lblCharName
            // 
            this.lblCharName.AutoEllipsis = true;
            this.lblCharName.BackColor = System.Drawing.Color.Transparent;
            this.lblCharName.Enabled = false;
            this.lblCharName.Location = new System.Drawing.Point(107, 11);
            this.lblCharName.Name = "lblCharName";
            this.lblCharName.Size = new System.Drawing.Size(186, 18);
            this.lblCharName.TabIndex = 6;
            this.lblCharName.Text = "Character Name";
            // 
            // lblSkillInTraining
            // 
            this.lblSkillInTraining.AutoEllipsis = true;
            this.lblSkillInTraining.BackColor = System.Drawing.Color.Transparent;
            this.lblSkillInTraining.Enabled = false;
            this.lblSkillInTraining.ForeColor = System.Drawing.Color.DimGray;
            this.lblSkillInTraining.Location = new System.Drawing.Point(107, 66);
            this.lblSkillInTraining.Name = "lblSkillInTraining";
            this.lblSkillInTraining.Size = new System.Drawing.Size(215, 13);
            this.lblSkillInTraining.TabIndex = 8;
            this.lblSkillInTraining.Text = "Projected Electronic Counter Measures III";
            // 
            // lblRemainingTime
            // 
            this.lblRemainingTime.AutoEllipsis = true;
            this.lblRemainingTime.BackColor = System.Drawing.Color.Transparent;
            this.lblRemainingTime.Enabled = false;
            this.lblRemainingTime.ForeColor = System.Drawing.Color.DimGray;
            this.lblRemainingTime.Location = new System.Drawing.Point(107, 52);
            this.lblRemainingTime.Name = "lblRemainingTime";
            this.lblRemainingTime.Size = new System.Drawing.Size(186, 14);
            this.lblRemainingTime.TabIndex = 7;
            this.lblRemainingTime.Text = "10d 12h 35m 24s";
            // 
            // lblBalance
            // 
            this.lblBalance.AutoEllipsis = true;
            this.lblBalance.BackColor = System.Drawing.Color.Transparent;
            this.lblBalance.Enabled = false;
            this.lblBalance.ForeColor = System.Drawing.Color.DimGray;
            this.lblBalance.Location = new System.Drawing.Point(108, 30);
            this.lblBalance.Name = "lblBalance";
            this.lblBalance.Size = new System.Drawing.Size(185, 16);
            this.lblBalance.TabIndex = 9;
            this.lblBalance.Text = "12,534,125,453.02 ISK";
            // 
            // OverviewItem
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.lblSkillQueueTrainingTime);
            this.Controls.Add(this.lblCompletionTime);
            this.Controls.Add(this.lblCharName);
            this.Controls.Add(this.lblSkillInTraining);
            this.Controls.Add(this.lblRemainingTime);
            this.Controls.Add(this.lblBalance);
            this.Controls.Add(this.pbCharacterPortrait);
            this.ForeColor = System.Drawing.SystemColors.ControlText;
            this.Name = "OverviewItem";
            this.Size = new System.Drawing.Size(330, 114);
            this.ResumeLayout(false);

        }

        #endregion

        private EVEMon.Common.Controls.CharacterPortrait pbCharacterPortrait;
        private EVEMon.Controls.OverviewLabel lblCharName;
        private EVEMon.Controls.OverviewLabel lblSkillInTraining;
        private EVEMon.Controls.OverviewLabel lblRemainingTime;
        private EVEMon.Controls.OverviewLabel lblBalance;
        private EVEMon.Controls.OverviewLabel lblCompletionTime;
        private EVEMon.Controls.OverviewLabel lblSkillQueueTrainingTime;
    }
}