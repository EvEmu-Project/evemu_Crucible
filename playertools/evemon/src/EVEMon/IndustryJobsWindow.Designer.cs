namespace EVEMon
{
    partial class IndustryJobsWindow
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
            this.jobsList = new EVEMon.MainWindowIndustryJobsList();
            this.SuspendLayout();
            // 
            // jobsList
            // 
            this.jobsList.Character = null;
            this.jobsList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.jobsList.Grouping = EVEMon.Common.SettingsObjects.IndustryJobGrouping.State;
            this.jobsList.Location = new System.Drawing.Point(0, 0);
            this.jobsList.Name = "jobsList";
            this.jobsList.ShowIssuedFor = EVEMon.Common.IssuedFor.All;
            this.jobsList.Size = new System.Drawing.Size(292, 266);
            this.jobsList.TabIndex = 0;
            this.jobsList.TextFilter = "";
            // 
            // IndustryJobsWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(292, 266);
            this.Controls.Add(this.jobsList);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "IndustryJobsWindow";
            this.Text = "Notification Details";
            this.ResumeLayout(false);

        }

        #endregion

        private MainWindowIndustryJobsList jobsList;
    }
}
