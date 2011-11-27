
namespace EVEMon.SkillPlanner
{
    partial class ShipLoadoutSelectWindow
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
            this.lblLoadouts = new System.Windows.Forms.Label();
            this.btnCancel = new System.Windows.Forms.Button();
            this.lowerPanel = new System.Windows.Forms.Panel();
            this.persistentSplitContainer1 = new EVEMon.Controls.PersistentSplitContainer();
            this.lvLoadouts = new System.Windows.Forms.ListView();
            this.colName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colAuthor = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colRating = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colDate = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tvLoadout = new System.Windows.Forms.TreeView();
            this.lblSubmitDate = new System.Windows.Forms.Label();
            this.SubDateLabel = new System.Windows.Forms.Label();
            this.lblForum = new System.Windows.Forms.LinkLabel();
            this.AuthorLabel = new System.Windows.Forms.Label();
            this.LoadoutNameLabel = new System.Windows.Forms.Label();
            this.ShipLabel = new System.Windows.Forms.Label();
            this.lblAuthor = new System.Windows.Forms.Label();
            this.lblLoadoutName = new System.Windows.Forms.Label();
            this.lblShipName = new System.Windows.Forms.Label();
            this.lblPlanned = new System.Windows.Forms.Label();
            this.TrainingTimeLabel = new System.Windows.Forms.Label();
            this.lblTrainTime = new System.Windows.Forms.Label();
            this.btnPlan = new System.Windows.Forms.Button();
            this.cmNode = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.miShowInBrowser = new System.Windows.Forms.ToolStripMenuItem();
            this.miExportToEFT = new System.Windows.Forms.ToolStripMenuItem();
            this.eveImage = new EVEMon.Common.Controls.EveImage();
            this.lowerPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.persistentSplitContainer1)).BeginInit();
            this.persistentSplitContainer1.Panel1.SuspendLayout();
            this.persistentSplitContainer1.Panel2.SuspendLayout();
            this.persistentSplitContainer1.SuspendLayout();
            this.cmNode.SuspendLayout();
            this.SuspendLayout();
            // 
            // lblLoadouts
            // 
            this.lblLoadouts.AutoSize = true;
            this.lblLoadouts.Location = new System.Drawing.Point(104, 82);
            this.lblLoadouts.Name = "lblLoadouts";
            this.lblLoadouts.Size = new System.Drawing.Size(103, 13);
            this.lblLoadouts.TabIndex = 1;
            this.lblLoadouts.Text = "Found {0} Loadouts";
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(641, 471);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 3;
            this.btnCancel.Text = "Close";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // lowerPanel
            // 
            this.lowerPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lowerPanel.Controls.Add(this.persistentSplitContainer1);
            this.lowerPanel.Location = new System.Drawing.Point(0, 123);
            this.lowerPanel.Name = "lowerPanel";
            this.lowerPanel.Size = new System.Drawing.Size(716, 342);
            this.lowerPanel.TabIndex = 6;
            // 
            // persistentSplitContainer1
            // 
            this.persistentSplitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.persistentSplitContainer1.Location = new System.Drawing.Point(0, 0);
            this.persistentSplitContainer1.Name = "persistentSplitContainer1";
            // 
            // persistentSplitContainer1.Panel1
            // 
            this.persistentSplitContainer1.Panel1.Controls.Add(this.lvLoadouts);
            this.persistentSplitContainer1.Panel1MinSize = 350;
            // 
            // persistentSplitContainer1.Panel2
            // 
            this.persistentSplitContainer1.Panel2.Controls.Add(this.tvLoadout);
            this.persistentSplitContainer1.RememberDistanceKey = null;
            this.persistentSplitContainer1.Size = new System.Drawing.Size(716, 342);
            this.persistentSplitContainer1.SplitterDistance = 373;
            this.persistentSplitContainer1.TabIndex = 5;
            // 
            // lvLoadouts
            // 
            this.lvLoadouts.AllowColumnReorder = true;
            this.lvLoadouts.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colName,
            this.colAuthor,
            this.colRating,
            this.colDate});
            this.lvLoadouts.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvLoadouts.FullRowSelect = true;
            this.lvLoadouts.Location = new System.Drawing.Point(0, 0);
            this.lvLoadouts.MinimumSize = new System.Drawing.Size(300, 190);
            this.lvLoadouts.MultiSelect = false;
            this.lvLoadouts.Name = "lvLoadouts";
            this.lvLoadouts.Size = new System.Drawing.Size(373, 342);
            this.lvLoadouts.TabIndex = 0;
            this.lvLoadouts.UseCompatibleStateImageBehavior = false;
            this.lvLoadouts.View = System.Windows.Forms.View.Details;
            this.lvLoadouts.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.lvLoadouts_ColumnClick);
            this.lvLoadouts.Click += new System.EventHandler(this.lvLoadouts_Click);
            // 
            // colName
            // 
            this.colName.Text = "Name";
            this.colName.Width = 153;
            // 
            // colAuthor
            // 
            this.colAuthor.Text = "Author";
            this.colAuthor.Width = 68;
            // 
            // colRating
            // 
            this.colRating.Text = "Rating";
            this.colRating.Width = 56;
            // 
            // colDate
            // 
            this.colDate.Text = "Date";
            this.colDate.Width = 90;
            // 
            // tvLoadout
            // 
            this.tvLoadout.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvLoadout.Location = new System.Drawing.Point(0, 0);
            this.tvLoadout.Name = "tvLoadout";
            this.tvLoadout.Size = new System.Drawing.Size(339, 342);
            this.tvLoadout.TabIndex = 3;
            this.tvLoadout.DoubleClick += new System.EventHandler(this.tvLoadout_DoubleClick);
            this.tvLoadout.MouseUp += new System.Windows.Forms.MouseEventHandler(this.tvLoadout_MouseUp);
            // 
            // lblSubmitDate
            // 
            this.lblSubmitDate.AutoSize = true;
            this.lblSubmitDate.Location = new System.Drawing.Point(194, 51);
            this.lblSubmitDate.Name = "lblSubmitDate";
            this.lblSubmitDate.Size = new System.Drawing.Size(62, 13);
            this.lblSubmitDate.TabIndex = 25;
            this.lblSubmitDate.Text = "SubmitDate";
            // 
            // SubDateLabel
            // 
            this.SubDateLabel.AutoSize = true;
            this.SubDateLabel.Location = new System.Drawing.Point(104, 51);
            this.SubDateLabel.Name = "SubDateLabel";
            this.SubDateLabel.Size = new System.Drawing.Size(89, 13);
            this.SubDateLabel.TabIndex = 24;
            this.SubDateLabel.Text = "Submission Date:";
            // 
            // lblForum
            // 
            this.lblForum.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblForum.AutoSize = true;
            this.lblForum.Location = new System.Drawing.Point(615, 96);
            this.lblForum.Name = "lblForum";
            this.lblForum.Size = new System.Drawing.Size(101, 13);
            this.lblForum.TabIndex = 23;
            this.lblForum.TabStop = true;
            this.lblForum.Text = "Discuss this loadout";
            this.lblForum.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.lblForum_LinkClicked);
            // 
            // AuthorLabel
            // 
            this.AuthorLabel.AutoSize = true;
            this.AuthorLabel.Location = new System.Drawing.Point(104, 38);
            this.AuthorLabel.Name = "AuthorLabel";
            this.AuthorLabel.Size = new System.Drawing.Size(44, 13);
            this.AuthorLabel.TabIndex = 22;
            this.AuthorLabel.Text = "Author:";
            // 
            // LoadoutNameLabel
            // 
            this.LoadoutNameLabel.AutoSize = true;
            this.LoadoutNameLabel.Location = new System.Drawing.Point(104, 25);
            this.LoadoutNameLabel.Name = "LoadoutNameLabel";
            this.LoadoutNameLabel.Size = new System.Drawing.Size(80, 13);
            this.LoadoutNameLabel.TabIndex = 21;
            this.LoadoutNameLabel.Text = "Loadout Name:";
            // 
            // ShipLabel
            // 
            this.ShipLabel.AutoSize = true;
            this.ShipLabel.Location = new System.Drawing.Point(104, 12);
            this.ShipLabel.Name = "ShipLabel";
            this.ShipLabel.Size = new System.Drawing.Size(31, 13);
            this.ShipLabel.TabIndex = 20;
            this.ShipLabel.Text = "Ship:";
            // 
            // lblAuthor
            // 
            this.lblAuthor.AutoSize = true;
            this.lblAuthor.Location = new System.Drawing.Point(194, 38);
            this.lblAuthor.Name = "lblAuthor";
            this.lblAuthor.Size = new System.Drawing.Size(67, 13);
            this.lblAuthor.TabIndex = 19;
            this.lblAuthor.Text = "AuthorName";
            // 
            // lblLoadoutName
            // 
            this.lblLoadoutName.AutoSize = true;
            this.lblLoadoutName.Location = new System.Drawing.Point(194, 25);
            this.lblLoadoutName.Name = "lblLoadoutName";
            this.lblLoadoutName.Size = new System.Drawing.Size(73, 13);
            this.lblLoadoutName.TabIndex = 18;
            this.lblLoadoutName.Text = "LoadoutName";
            // 
            // lblShipName
            // 
            this.lblShipName.AutoSize = true;
            this.lblShipName.Location = new System.Drawing.Point(194, 12);
            this.lblShipName.Name = "lblShipName";
            this.lblShipName.Size = new System.Drawing.Size(54, 13);
            this.lblShipName.TabIndex = 17;
            this.lblShipName.Text = "ShipName";
            // 
            // lblPlanned
            // 
            this.lblPlanned.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.lblPlanned.AutoSize = true;
            this.lblPlanned.Location = new System.Drawing.Point(191, 481);
            this.lblPlanned.Name = "lblPlanned";
            this.lblPlanned.Size = new System.Drawing.Size(0, 13);
            this.lblPlanned.TabIndex = 28;
            // 
            // TrainingTimeLabel
            // 
            this.TrainingTimeLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.TrainingTimeLabel.AutoSize = true;
            this.TrainingTimeLabel.Location = new System.Drawing.Point(12, 481);
            this.TrainingTimeLabel.Name = "TrainingTimeLabel";
            this.TrainingTimeLabel.Size = new System.Drawing.Size(176, 13);
            this.TrainingTimeLabel.TabIndex = 26;
            this.TrainingTimeLabel.Text = "Training Time for selected loadout: ";
            // 
            // lblTrainTime
            // 
            this.lblTrainTime.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.lblTrainTime.AutoSize = true;
            this.lblTrainTime.Location = new System.Drawing.Point(191, 481);
            this.lblTrainTime.Name = "lblTrainTime";
            this.lblTrainTime.Size = new System.Drawing.Size(25, 13);
            this.lblTrainTime.TabIndex = 27;
            this.lblTrainTime.Text = "N/A";
            // 
            // btnPlan
            // 
            this.btnPlan.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnPlan.Enabled = false;
            this.btnPlan.Location = new System.Drawing.Point(560, 471);
            this.btnPlan.Name = "btnPlan";
            this.btnPlan.Size = new System.Drawing.Size(75, 23);
            this.btnPlan.TabIndex = 29;
            this.btnPlan.Text = "Add To Plan";
            this.btnPlan.UseVisualStyleBackColor = true;
            this.btnPlan.Click += new System.EventHandler(this.btnPlan_Click);
            // 
            // cmNode
            // 
            this.cmNode.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.miShowInBrowser,
            this.miExportToEFT});
            this.cmNode.Name = "cmNode";
            this.cmNode.Size = new System.Drawing.Size(203, 48);
            // 
            // miShowInBrowser
            // 
            this.miShowInBrowser.Name = "miShowInBrowser";
            this.miShowInBrowser.Size = new System.Drawing.Size(202, 22);
            this.miShowInBrowser.Text = "Show in Items Browser...";
            this.miShowInBrowser.Click += new System.EventHandler(this.tvLoadout_DoubleClick);
            // 
            // miExportToEFT
            // 
            this.miExportToEFT.Name = "miExportToEFT";
            this.miExportToEFT.Size = new System.Drawing.Size(202, 22);
            this.miExportToEFT.Text = "Export Loadout To EFT";
            this.miExportToEFT.Click += new System.EventHandler(this.miExportToEFT_Click);
            // 
            // eveImage
            // 
            this.eveImage.EveItem = null;
            this.eveImage.ImageSize = EVEMon.Common.EveImageSize.x128;
            this.eveImage.Location = new System.Drawing.Point(12, 12);
            this.eveImage.Name = "eveImage";
            this.eveImage.PopUpEnabled = true;
            this.eveImage.Size = new System.Drawing.Size(80, 80);
            this.eveImage.SizeMode = EVEMon.Common.Controls.EveImage.EveImageSizeMode.StretchImage;
            this.eveImage.TabIndex = 30;
            // 
            // ShipLoadoutSelectWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.ClientSize = new System.Drawing.Size(728, 506);
            this.Controls.Add(this.eveImage);
            this.Controls.Add(this.btnPlan);
            this.Controls.Add(this.lblPlanned);
            this.Controls.Add(this.lblSubmitDate);
            this.Controls.Add(this.TrainingTimeLabel);
            this.Controls.Add(this.SubDateLabel);
            this.Controls.Add(this.lblTrainTime);
            this.Controls.Add(this.lblForum);
            this.Controls.Add(this.AuthorLabel);
            this.Controls.Add(this.LoadoutNameLabel);
            this.Controls.Add(this.ShipLabel);
            this.Controls.Add(this.lblAuthor);
            this.Controls.Add(this.lblLoadoutName);
            this.Controls.Add(this.lblShipName);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.lblLoadouts);
            this.Controls.Add(this.lowerPanel);
            this.MinimumSize = new System.Drawing.Size(744, 544);
            this.Name = "ShipLoadoutSelectWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "BattleClinic Loadout Selection";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.LoadoutSelect_FormClosing);
            this.Load += new System.EventHandler(this.LoadoutSelect_Load);
            this.lowerPanel.ResumeLayout(false);
            this.persistentSplitContainer1.Panel1.ResumeLayout(false);
            this.persistentSplitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.persistentSplitContainer1)).EndInit();
            this.persistentSplitContainer1.ResumeLayout(false);
            this.cmNode.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblLoadouts;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.ListView lvLoadouts;
        private System.Windows.Forms.ColumnHeader colName;
        private System.Windows.Forms.ColumnHeader colAuthor;
        private System.Windows.Forms.ColumnHeader colRating;
        private System.Windows.Forms.ColumnHeader colDate;
        private EVEMon.Controls.PersistentSplitContainer persistentSplitContainer1;
        private System.Windows.Forms.Panel lowerPanel;
        private System.Windows.Forms.Label lblSubmitDate;
        private System.Windows.Forms.Label SubDateLabel;
        private System.Windows.Forms.LinkLabel lblForum;
        private System.Windows.Forms.Label AuthorLabel;
        private System.Windows.Forms.Label LoadoutNameLabel;
        private System.Windows.Forms.Label ShipLabel;
        private System.Windows.Forms.Label lblAuthor;
        private System.Windows.Forms.Label lblLoadoutName;
        private System.Windows.Forms.Label lblShipName;
        private System.Windows.Forms.TreeView tvLoadout;
        private System.Windows.Forms.Label lblPlanned;
        private System.Windows.Forms.Label TrainingTimeLabel;
        private System.Windows.Forms.Label lblTrainTime;
        private System.Windows.Forms.Button btnPlan;
        private System.Windows.Forms.ContextMenuStrip cmNode;
        private System.Windows.Forms.ToolStripMenuItem miShowInBrowser;
        private System.Windows.Forms.ToolStripMenuItem miExportToEFT;
        private Common.Controls.EveImage eveImage;
    }
}
