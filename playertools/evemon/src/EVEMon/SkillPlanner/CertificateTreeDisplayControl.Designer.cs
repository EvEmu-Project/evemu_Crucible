namespace EVEMon.SkillPlanner
{
    partial class CertificateTreeDisplayControl
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CertificateTreeDisplayControl));
            this.treeView = new System.Windows.Forms.TreeView();
            this.cmListSkills = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.tsmAddToPlan = new System.Windows.Forms.ToolStripMenuItem();
            this.showInMenuSeparator = new System.Windows.Forms.ToolStripSeparator();
            this.showInBrowserMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.showInExplorerMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.tsSeparatorToggle = new System.Windows.Forms.ToolStripSeparator();
            this.tsmExpandSelected = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmCollapseSelected = new System.Windows.Forms.ToolStripMenuItem();
            this.tsSeparator = new System.Windows.Forms.ToolStripSeparator();
            this.tsmExpandAll = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmCollapseAll = new System.Windows.Forms.ToolStripMenuItem();
            this.imageList = new System.Windows.Forms.ImageList(this.components);
            this.cmListSkills.SuspendLayout();
            this.SuspendLayout();
            // 
            // treeView
            // 
            this.treeView.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.treeView.ContextMenuStrip = this.cmListSkills;
            this.treeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.treeView.DrawMode = System.Windows.Forms.TreeViewDrawMode.OwnerDrawText;
            this.treeView.FullRowSelect = true;
            this.treeView.ImageIndex = 0;
            this.treeView.ImageList = this.imageList;
            this.treeView.Indent = 27;
            this.treeView.ItemHeight = 1;
            this.treeView.Location = new System.Drawing.Point(0, 10);
            this.treeView.Name = "treeView";
            this.treeView.SelectedImageIndex = 0;
            this.treeView.Size = new System.Drawing.Size(359, 441);
            this.treeView.TabIndex = 0;
            this.treeView.NodeMouseDoubleClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.treeView_NodeMouseDoubleClick);
            this.treeView.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.treeView_NodeMouseClick);
            // 
            // cmListSkills
            // 
            this.cmListSkills.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmAddToPlan,
            this.showInMenuSeparator,
            this.showInBrowserMenu,
            this.showInExplorerMenu,
            this.tsSeparatorToggle,
            this.tsmExpandSelected,
            this.tsmCollapseSelected,
            this.tsSeparator,
            this.tsmExpandAll,
            this.tsmCollapseAll});
            this.cmListSkills.Name = "cmListSkills";
            this.cmListSkills.Size = new System.Drawing.Size(195, 198);
            // 
            // tsmAddToPlan
            // 
            this.tsmAddToPlan.Image = global::EVEMon.Common.Properties.Resources.EditPlan;
            this.tsmAddToPlan.Name = "tsmAddToPlan";
            this.tsmAddToPlan.Size = new System.Drawing.Size(194, 22);
            this.tsmAddToPlan.Text = "&Plan...";
            this.tsmAddToPlan.Click += new System.EventHandler(this.tsmAddToPlan_Click);
            // 
            // showInMenuSeparator
            // 
            this.showInMenuSeparator.Name = "showInMenuSeparator";
            this.showInMenuSeparator.Size = new System.Drawing.Size(191, 6);
            // 
            // showInBrowserMenu
            // 
            this.showInBrowserMenu.Name = "showInBrowserMenu";
            this.showInBrowserMenu.Size = new System.Drawing.Size(194, 22);
            this.showInBrowserMenu.Text = "Show in Skill &Browser...";
            this.showInBrowserMenu.Click += new System.EventHandler(this.showInBrowserMenu_Click);
            // 
            // showInExplorerMenu
            // 
            this.showInExplorerMenu.Image = global::EVEMon.Common.Properties.Resources.LeadsTo;
            this.showInExplorerMenu.Name = "showInExplorerMenu";
            this.showInExplorerMenu.Size = new System.Drawing.Size(194, 22);
            this.showInExplorerMenu.Text = "Show in Skill &Explorer...";
            this.showInExplorerMenu.Click += new System.EventHandler(this.showInExplorerMenu_Click);
            // 
            // tsSeparatorToggle
            // 
            this.tsSeparatorToggle.Name = "tsSeparatorToggle";
            this.tsSeparatorToggle.Size = new System.Drawing.Size(191, 6);
            // 
            // tsmExpandSelected
            // 
            this.tsmExpandSelected.Name = "tsmExpandSelected";
            this.tsmExpandSelected.Size = new System.Drawing.Size(194, 22);
            this.tsmExpandSelected.Text = "Expand Selected";
            this.tsmExpandSelected.Click += new System.EventHandler(this.tsmExpandSelected_Click);
            // 
            // tsmCollapseSelected
            // 
            this.tsmCollapseSelected.Name = "tsmCollapseSelected";
            this.tsmCollapseSelected.Size = new System.Drawing.Size(194, 22);
            this.tsmCollapseSelected.Text = "Collapse Selected";
            this.tsmCollapseSelected.Click += new System.EventHandler(this.tsmCollapseSelected_Click);
            // 
            // tsSeparator
            // 
            this.tsSeparator.Name = "tsSeparator";
            this.tsSeparator.Size = new System.Drawing.Size(191, 6);
            // 
            // tsmExpandAll
            // 
            this.tsmExpandAll.Name = "tsmExpandAll";
            this.tsmExpandAll.Size = new System.Drawing.Size(194, 22);
            this.tsmExpandAll.Text = "&Expand All";
            this.tsmExpandAll.Click += new System.EventHandler(this.tsmExpandAll_Click);
            // 
            // tsmCollapseAll
            // 
            this.tsmCollapseAll.Name = "tsmCollapseAll";
            this.tsmCollapseAll.Size = new System.Drawing.Size(194, 22);
            this.tsmCollapseAll.Text = "&Collapse All";
            this.tsmCollapseAll.Click += new System.EventHandler(this.tsmCollapseAll_Click);
            // 
            // imageList
            // 
            this.imageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList.ImageStream")));
            this.imageList.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList.Images.SetKeyName(0, "Certificate - Granted.png");
            this.imageList.Images.SetKeyName(1, "Certificate - Claimable.png");
            this.imageList.Images.SetKeyName(2, "Certificate - Trainable.png");
            this.imageList.Images.SetKeyName(3, "Certificate - Untrainable.png");
            this.imageList.Images.SetKeyName(4, "Certificate.png");
            this.imageList.Images.SetKeyName(5, "Skillbook.png");
            // 
            // CertificateTreeDisplayControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.Controls.Add(this.treeView);
            this.Name = "CertificateTreeDisplayControl";
            this.Padding = new System.Windows.Forms.Padding(0, 10, 0, 10);
            this.Size = new System.Drawing.Size(359, 461);
            this.cmListSkills.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TreeView treeView;
        private System.Windows.Forms.ImageList imageList;
        private System.Windows.Forms.ContextMenuStrip cmListSkills;
        private System.Windows.Forms.ToolStripSeparator tsSeparator;
        private System.Windows.Forms.ToolStripMenuItem tsmExpandAll;
        private System.Windows.Forms.ToolStripMenuItem tsmCollapseAll;
        private System.Windows.Forms.ToolStripMenuItem tsmAddToPlan;
        private System.Windows.Forms.ToolStripMenuItem showInBrowserMenu;
        private System.Windows.Forms.ToolStripSeparator showInMenuSeparator;
        private System.Windows.Forms.ToolStripMenuItem showInExplorerMenu;
        private System.Windows.Forms.ToolStripSeparator tsSeparatorToggle;
        private System.Windows.Forms.ToolStripMenuItem tsmExpandSelected;
        private System.Windows.Forms.ToolStripMenuItem tsmCollapseSelected;
    }
}
