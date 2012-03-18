namespace EVEMon.SkillPlanner
{
    partial class RecommendedCertificatesControl
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RecommendedCertificatesControl));
            this.panel1 = new System.Windows.Forms.Panel();
            this.lblTimeRequired = new System.Windows.Forms.Label();
            this.btnAddCerts = new System.Windows.Forms.Button();
            this.panel2 = new System.Windows.Forms.Panel();
            this.contextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.showInMenuSeparator = new System.Windows.Forms.ToolStripSeparator();
            this.showInBrowserMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.tsSeparator = new System.Windows.Forms.ToolStripSeparator();
            this.tsmExpandAll = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmCollapseAll = new System.Windows.Forms.ToolStripMenuItem();
            this.imageList = new System.Windows.Forms.ImageList(this.components);
            this.tvCertList = new EVEMon.SkillPlanner.ReqCertificatesTreeView();
            this.tsmAddToPlan = new System.Windows.Forms.ToolStripMenuItem();
            this.showInExplorerMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.panel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.contextMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.lblTimeRequired);
            this.panel1.Controls.Add(this.btnAddCerts);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 127);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(244, 30);
            this.panel1.TabIndex = 5;
            // 
            // lblTimeRequired
            // 
            this.lblTimeRequired.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.lblTimeRequired.AutoSize = true;
            this.lblTimeRequired.Location = new System.Drawing.Point(3, 9);
            this.lblTimeRequired.Name = "lblTimeRequired";
            this.lblTimeRequired.Size = new System.Drawing.Size(71, 13);
            this.lblTimeRequired.TabIndex = 1;
            this.lblTimeRequired.Text = "Time required";
            // 
            // btnAddCerts
            // 
            this.btnAddCerts.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnAddCerts.Location = new System.Drawing.Point(153, 4);
            this.btnAddCerts.Name = "btnAddCerts";
            this.btnAddCerts.Size = new System.Drawing.Size(88, 23);
            this.btnAddCerts.TabIndex = 0;
            this.btnAddCerts.Text = "Add All To Plan";
            this.btnAddCerts.UseVisualStyleBackColor = true;
            this.btnAddCerts.Click += new System.EventHandler(this.btnAddCerts_Click);
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.tvCertList);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel2.Location = new System.Drawing.Point(0, 0);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(244, 127);
            this.panel2.TabIndex = 6;
            // 
            // contextMenu
            // 
            this.contextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmAddToPlan,
            this.showInMenuSeparator,
            this.showInBrowserMenu,
            this.showInExplorerMenu,
            this.tsSeparator,
            this.tsmExpandAll,
            this.tsmCollapseAll});
            this.contextMenu.Name = "contextMenu";
            this.contextMenu.Size = new System.Drawing.Size(195, 126);
            this.contextMenu.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenu_Opening);
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
            this.showInBrowserMenu.Click += new System.EventHandler(this.showInSkillBrowserMenu_Click);
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
            this.tsmExpandAll.Text = "&Expand all";
            this.tsmExpandAll.Click += new System.EventHandler(this.tsmExpandAll_Click);
            // 
            // tsmCollapseAll
            // 
            this.tsmCollapseAll.Name = "tsmCollapseAll";
            this.tsmCollapseAll.Size = new System.Drawing.Size(194, 22);
            this.tsmCollapseAll.Text = "&Collapse all";
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
            this.imageList.Images.SetKeyName(6, "Plan.png");
            // 
            // tvCertList
            // 
            this.tvCertList.ContextMenuStrip = this.contextMenu;
            this.tvCertList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvCertList.DrawMode = System.Windows.Forms.TreeViewDrawMode.OwnerDrawText;
            this.tvCertList.FullRowSelect = true;
            this.tvCertList.ImageIndex = 0;
            this.tvCertList.ImageList = this.imageList;
            this.tvCertList.Location = new System.Drawing.Point(0, 0);
            this.tvCertList.Name = "tvCertList";
            this.tvCertList.SelectedImageIndex = 0;
            this.tvCertList.Size = new System.Drawing.Size(244, 127);
            this.tvCertList.TabIndex = 0;
            this.tvCertList.NodeMouseDoubleClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.tvCertList_NodeMouseDoubleClick);
            this.tvCertList.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.tvCertList_NodeMouseClick);
            // 
            // tsmAddToPlan
            // 
            this.tsmAddToPlan.Image = global::EVEMon.Common.Properties.Resources.EditPlan;
            this.tsmAddToPlan.Name = "tsmAddToPlan";
            this.tsmAddToPlan.Size = new System.Drawing.Size(194, 22);
            this.tsmAddToPlan.Text = "&Plan...";
            this.tsmAddToPlan.Click += new System.EventHandler(this.tsmAddToPlan_Click);
            // 
            // showInExplorerMenu
            // 
            this.showInExplorerMenu.Image = global::EVEMon.Common.Properties.Resources.LeadsTo;
            this.showInExplorerMenu.Name = "showInExplorerMenu";
            this.showInExplorerMenu.Size = new System.Drawing.Size(194, 22);
            this.showInExplorerMenu.Text = "Show in Skill &Explorer...";
            this.showInExplorerMenu.Click += new System.EventHandler(this.showInSkillExplorerMenu_Click);
            // 
            // RecommendedCertificatesControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.panel2);
            this.Controls.Add(this.panel1);
            this.Name = "RecommendedCertificatesControl";
            this.Size = new System.Drawing.Size(244, 157);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.panel2.ResumeLayout(false);
            this.contextMenu.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Label lblTimeRequired;
        private System.Windows.Forms.Button btnAddCerts;
        private System.Windows.Forms.Panel panel2;
        private ReqCertificatesTreeView tvCertList;
        private System.Windows.Forms.ImageList imageList;
        private System.Windows.Forms.ContextMenuStrip contextMenu;
        private System.Windows.Forms.ToolStripMenuItem tsmAddToPlan;
        private System.Windows.Forms.ToolStripSeparator showInMenuSeparator;
        private System.Windows.Forms.ToolStripMenuItem showInBrowserMenu;
        private System.Windows.Forms.ToolStripMenuItem showInExplorerMenu;
        private System.Windows.Forms.ToolStripSeparator tsSeparator;
        private System.Windows.Forms.ToolStripMenuItem tsmExpandAll;
        private System.Windows.Forms.ToolStripMenuItem tsmCollapseAll;
    }
}
