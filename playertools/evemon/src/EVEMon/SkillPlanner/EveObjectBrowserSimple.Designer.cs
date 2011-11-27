namespace EVEMon.SkillPlanner
{
    partial class EveObjectBrowserSimple
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
            this.scDetails = new EVEMon.Common.Controls.SplitContainerMinFixed();
            this.scDetailsRight = new EVEMon.Common.Controls.SplitContainerMinFixed();
            this.gbDescription = new System.Windows.Forms.GroupBox();
            this.tbDescription = new System.Windows.Forms.RichTextBox();
            this.gbRequiredSkills = new System.Windows.Forms.GroupBox();
            this.requiredSkillsControl = new EVEMon.SkillPlanner.RequiredSkillsControl();
            this.scObjectBrowser.Panel2.SuspendLayout();
            this.scObjectBrowser.SuspendLayout();
            this.pnlDetails.SuspendLayout();
            this.pnlBrowserHeader.SuspendLayout();
            this.scDetails.Panel2.SuspendLayout();
            this.scDetails.SuspendLayout();
            this.scDetailsRight.Panel1.SuspendLayout();
            this.scDetailsRight.Panel2.SuspendLayout();
            this.scDetailsRight.SuspendLayout();
            this.gbDescription.SuspendLayout();
            this.gbRequiredSkills.SuspendLayout();
            this.SuspendLayout();
            // 
            // scObjectBrowser
            // 
            // 
            // pnlDetails
            // 
            this.pnlDetails.Controls.Add(this.scDetails);
            // 
            // scDetails
            // 
            this.scDetails.Dock = System.Windows.Forms.DockStyle.Fill;
            this.scDetails.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.scDetails.Location = new System.Drawing.Point(0, 0);
            this.scDetails.Name = "scDetails";
            // 
            // scDetails.Panel2
            // 
            this.scDetails.Panel2.Controls.Add(this.scDetailsRight);
            this.scDetails.Panel2MinSize = 240;
            this.scDetails.Size = new System.Drawing.Size(482, 343);
            this.scDetails.SplitterDistance = 238;
            this.scDetails.TabIndex = 13;
            // 
            // scDetailsRight
            // 
            this.scDetailsRight.Dock = System.Windows.Forms.DockStyle.Fill;
            this.scDetailsRight.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.scDetailsRight.Location = new System.Drawing.Point(0, 0);
            this.scDetailsRight.Name = "scDetailsRight";
            this.scDetailsRight.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // scDetailsRight.Panel1
            // 
            this.scDetailsRight.Panel1.Controls.Add(this.gbDescription);
            // 
            // scDetailsRight.Panel2
            // 
            this.scDetailsRight.Panel2.Controls.Add(this.gbRequiredSkills);
            this.scDetailsRight.Panel2MinSize = 120;
            this.scDetailsRight.Size = new System.Drawing.Size(240, 343);
            this.scDetailsRight.SplitterDistance = 218;
            this.scDetailsRight.TabIndex = 0;
            // 
            // gbDescription
            // 
            this.gbDescription.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.gbDescription.Controls.Add(this.tbDescription);
            this.gbDescription.Dock = System.Windows.Forms.DockStyle.Fill;
            this.gbDescription.Location = new System.Drawing.Point(0, 0);
            this.gbDescription.Name = "gbDescription";
            this.gbDescription.Size = new System.Drawing.Size(240, 218);
            this.gbDescription.TabIndex = 14;
            this.gbDescription.TabStop = false;
            this.gbDescription.Text = "Description";
            // 
            // tbDescription
            // 
            this.tbDescription.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tbDescription.Location = new System.Drawing.Point(3, 16);
            this.tbDescription.Name = "tbDescription";
            this.tbDescription.ReadOnly = true;
            this.tbDescription.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
            this.tbDescription.Size = new System.Drawing.Size(234, 199);
            this.tbDescription.TabIndex = 0;
            this.tbDescription.Text = "";
            // 
            // gbRequiredSkills
            // 
            this.gbRequiredSkills.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.gbRequiredSkills.Controls.Add(this.requiredSkillsControl);
            this.gbRequiredSkills.Dock = System.Windows.Forms.DockStyle.Fill;
            this.gbRequiredSkills.Location = new System.Drawing.Point(0, 0);
            this.gbRequiredSkills.Name = "gbRequiredSkills";
            this.gbRequiredSkills.Size = new System.Drawing.Size(240, 121);
            this.gbRequiredSkills.TabIndex = 13;
            this.gbRequiredSkills.TabStop = false;
            this.gbRequiredSkills.Text = "Required Skills";
            // 
            // requiredSkillsControl
            // 
            this.requiredSkillsControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.requiredSkillsControl.Object = null;
            this.requiredSkillsControl.Location = new System.Drawing.Point(3, 16);
            this.requiredSkillsControl.Name = "requiredSkillsControl";
            this.requiredSkillsControl.Plan = null;
            this.requiredSkillsControl.Size = new System.Drawing.Size(234, 102);
            this.requiredSkillsControl.TabIndex = 12;
            // 
            // EveObjectBrowserSimple
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.Name = "EveObjectBrowserSimple";
            this.scObjectBrowser.Panel2.ResumeLayout(false);
            this.scObjectBrowser.Panel2.PerformLayout();
            this.scObjectBrowser.ResumeLayout(false);
            this.pnlDetails.ResumeLayout(false);
            this.pnlBrowserHeader.ResumeLayout(false);
            this.pnlBrowserHeader.PerformLayout();
            this.scDetails.Panel2.ResumeLayout(false);
            this.scDetails.ResumeLayout(false);
            this.scDetailsRight.Panel1.ResumeLayout(false);
            this.scDetailsRight.Panel2.ResumeLayout(false);
            this.scDetailsRight.ResumeLayout(false);
            this.gbDescription.ResumeLayout(false);
            this.gbRequiredSkills.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        protected System.Windows.Forms.GroupBox gbDescription;
        protected System.Windows.Forms.GroupBox gbRequiredSkills;
        protected EVEMon.Common.Controls.SplitContainerMinFixed scDetailsRight;
        protected RequiredSkillsControl requiredSkillsControl;
        protected System.Windows.Forms.RichTextBox tbDescription;
        protected EVEMon.Common.Controls.SplitContainerMinFixed scDetails;
    }
}
