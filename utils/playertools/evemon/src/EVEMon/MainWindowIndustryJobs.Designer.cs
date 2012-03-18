namespace EVEMon
{
    partial class MainWindowIndustryJobsList
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindowIndustryJobsList));
            this.lvJobs = new System.Windows.Forms.ListView();
            this.chState = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.chTTC = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.chInstalledItem = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.chOutputItem = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.ilIcons = new System.Windows.Forms.ImageList(this.components);
            this.noJobsLabel = new System.Windows.Forms.Label();
            this.industryExpPanelControl = new EVEMon.Controls.ExpandablePanelControl();
            this.SuspendLayout();
            // 
            // lvJobs
            // 
            this.lvJobs.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.lvJobs.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.chState,
            this.chTTC,
            this.chInstalledItem,
            this.chOutputItem});
            this.lvJobs.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvJobs.FullRowSelect = true;
            this.lvJobs.HideSelection = false;
            this.lvJobs.Location = new System.Drawing.Point(0, 0);
            this.lvJobs.Name = "lvJobs";
            this.lvJobs.Size = new System.Drawing.Size(454, 334);
            this.lvJobs.SmallImageList = this.ilIcons;
            this.lvJobs.TabIndex = 0;
            this.lvJobs.UseCompatibleStateImageBehavior = false;
            this.lvJobs.View = System.Windows.Forms.View.Details;
            // 
            // chState
            // 
            this.chState.Text = "State";
            // 
            // chTTC
            // 
            this.chTTC.Text = "TTC";
            // 
            // chInstalledItem
            // 
            this.chInstalledItem.Text = "Installed Item";
            // 
            // chOutputItem
            // 
            this.chOutputItem.Text = "Output Item";
            // 
            // ilIcons
            // 
            this.ilIcons.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("ilIcons.ImageStream")));
            this.ilIcons.TransparentColor = System.Drawing.Color.Transparent;
            this.ilIcons.Images.SetKeyName(0, "arrow_up.png");
            this.ilIcons.Images.SetKeyName(1, "arrow_down.png");
            this.ilIcons.Images.SetKeyName(2, "16x16Transparant.png");
            // 
            // noJobsLabel
            // 
            this.noJobsLabel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.noJobsLabel.ForeColor = System.Drawing.SystemColors.GrayText;
            this.noJobsLabel.Location = new System.Drawing.Point(0, 0);
            this.noJobsLabel.Name = "noJobsLabel";
            this.noJobsLabel.Size = new System.Drawing.Size(454, 434);
            this.noJobsLabel.TabIndex = 2;
            this.noJobsLabel.Text = "No industry jobs are available.";
            this.noJobsLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // industryExpPanelControl
            // 
            this.industryExpPanelControl.AnimationSpeed = EVEMon.Controls.AnimationSpeed.Medium;
            this.industryExpPanelControl.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.industryExpPanelControl.EnableContextMenu = false;
            this.industryExpPanelControl.ExpandDirection = EVEMon.Controls.Direction.Down;
            this.industryExpPanelControl.ExpandedOnStartup = false;
            this.industryExpPanelControl.HeaderHeight = 30;
            this.industryExpPanelControl.ImageCollapse = global::EVEMon.Common.Properties.Resources.Expand;
            this.industryExpPanelControl.ImageExpand = global::EVEMon.Common.Properties.Resources.Collapse;
            this.industryExpPanelControl.Location = new System.Drawing.Point(0, 334);
            this.industryExpPanelControl.Name = "industryExpPanelControl";
            this.industryExpPanelControl.Size = new System.Drawing.Size(454, 100);
            this.industryExpPanelControl.TabIndex = 1;
            // 
            // MainWindowIndustryJobsList
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.lvJobs);
            this.Controls.Add(this.industryExpPanelControl);
            this.Controls.Add(this.noJobsLabel);
            this.Name = "MainWindowIndustryJobsList";
            this.Size = new System.Drawing.Size(454, 434);
            this.ResumeLayout(false);

        }

        #endregion

        internal protected System.Windows.Forms.ListView lvJobs;
        private System.Windows.Forms.Label noJobsLabel;
        private System.Windows.Forms.ColumnHeader chState;
        private System.Windows.Forms.ColumnHeader chTTC;
        private System.Windows.Forms.ColumnHeader chInstalledItem;
        private System.Windows.Forms.ColumnHeader chOutputItem;
        private System.Windows.Forms.ImageList ilIcons;
        private EVEMon.Controls.ExpandablePanelControl industryExpPanelControl;
    }
}
