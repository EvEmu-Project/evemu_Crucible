namespace EVEMon.SkillPlanner
{
    partial class ItemBrowserControl
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
            this.gbAttributes = new System.Windows.Forms.GroupBox();
            this.lvItemProperties = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.ItemAttributeContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.exportToCSVToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.itemSelectControl = new EVEMon.SkillPlanner.ItemSelectControl();
            this.ttItem = new System.Windows.Forms.ToolTip(this.components);
            this.gbRequiredSkills = new System.Windows.Forms.GroupBox();
            this.requiredSkillsControl = new EVEMon.SkillPlanner.RequiredSkillsControl();
            this.scDetailsRight.Panel1.SuspendLayout();
            this.scDetailsRight.Panel2.SuspendLayout();
            this.scDetailsRight.SuspendLayout();
            this.scDetails.Panel1.SuspendLayout();
            this.scDetails.Panel2.SuspendLayout();
            this.scDetails.SuspendLayout();
            this.gbDescription.SuspendLayout();
            this.scObjectBrowser.Panel1.SuspendLayout();
            this.scObjectBrowser.Panel2.SuspendLayout();
            this.scObjectBrowser.SuspendLayout();
            this.pnlDetails.SuspendLayout();
            this.pnlBrowserHeader.SuspendLayout();
            this.gbAttributes.SuspendLayout();
            this.ItemAttributeContextMenu.SuspendLayout();
            this.gbRequiredSkills.SuspendLayout();
            this.SuspendLayout();
            // 
            // scDetailsRight
            // 
            // 
            // scDetailsRight.Panel2
            // 
            this.scDetailsRight.Panel2.Controls.Add(this.gbRequiredSkills);
            this.scDetailsRight.SplitterDistance = 235;
            // 
            // scDetails
            // 
            // 
            // scDetails.Panel1
            // 
            this.scDetails.Panel1.Controls.Add(this.gbAttributes);
            // 
            // gbDescription
            // 
            this.gbDescription.Size = new System.Drawing.Size(240, 235);
            // 
            // tbDescription
            // 
            this.tbDescription.Size = new System.Drawing.Size(234, 216);
            // 
            // scObjectBrowser
            // 
            // 
            // scObjectBrowser.Panel1
            // 
            this.scObjectBrowser.Panel1.Controls.Add(this.itemSelectControl);
            // 
            // gbAttributes
            // 
            this.gbAttributes.Controls.Add(this.lvItemProperties);
            this.gbAttributes.Dock = System.Windows.Forms.DockStyle.Fill;
            this.gbAttributes.Location = new System.Drawing.Point(0, 0);
            this.gbAttributes.Name = "gbAttributes";
            this.gbAttributes.Size = new System.Drawing.Size(238, 343);
            this.gbAttributes.TabIndex = 15;
            this.gbAttributes.TabStop = false;
            this.gbAttributes.Text = "Attributes";
            // 
            // lvItemProperties
            // 
            this.lvItemProperties.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.lvItemProperties.ContextMenuStrip = this.ItemAttributeContextMenu;
            this.lvItemProperties.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvItemProperties.FullRowSelect = true;
            this.lvItemProperties.Location = new System.Drawing.Point(3, 16);
            this.lvItemProperties.Name = "lvItemProperties";
            this.lvItemProperties.Size = new System.Drawing.Size(232, 324);
            this.lvItemProperties.TabIndex = 8;
            this.lvItemProperties.UseCompatibleStateImageBehavior = false;
            this.lvItemProperties.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Attribute";
            this.columnHeader1.Width = 200;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Value";
            this.columnHeader2.Width = 140;
            // 
            // ItemAttributeContextMenu
            // 
            this.ItemAttributeContextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exportToCSVToolStripMenuItem});
            this.ItemAttributeContextMenu.Name = "ItemAttributeContextMenu";
            this.ItemAttributeContextMenu.Size = new System.Drawing.Size(158, 26);
            // 
            // exportToCSVToolStripMenuItem
            // 
            this.exportToCSVToolStripMenuItem.Name = "exportToCSVToolStripMenuItem";
            this.exportToCSVToolStripMenuItem.Size = new System.Drawing.Size(157, 22);
            this.exportToCSVToolStripMenuItem.Text = "Export to CSV ...";
            this.exportToCSVToolStripMenuItem.Click += new System.EventHandler(this.exportToCSVToolStripMenuItem_Click);
            // 
            // itemSelectControl
            // 
            this.itemSelectControl.AutoSize = true;
            this.itemSelectControl.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.itemSelectControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.itemSelectControl.Location = new System.Drawing.Point(0, 0);
            this.itemSelectControl.Margin = new System.Windows.Forms.Padding(2);
            this.itemSelectControl.Name = "itemSelectControl";
            this.itemSelectControl.Plan = null;
            this.itemSelectControl.SelectedObject = null;
            this.itemSelectControl.Size = new System.Drawing.Size(163, 413);
            this.itemSelectControl.TabIndex = 0;
            // 
            // gbRequiredSkills
            // 
            this.gbRequiredSkills.Controls.Add(this.requiredSkillsControl);
            this.gbRequiredSkills.Dock = System.Windows.Forms.DockStyle.Fill;
            this.gbRequiredSkills.Location = new System.Drawing.Point(0, 0);
            this.gbRequiredSkills.Name = "gbRequiredSkills";
            this.gbRequiredSkills.Size = new System.Drawing.Size(240, 104);
            this.gbRequiredSkills.TabIndex = 0;
            this.gbRequiredSkills.TabStop = false;
            this.gbRequiredSkills.Text = "Required Skills";
            // 
            // requiredSkillsControl
            // 
            this.requiredSkillsControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.requiredSkillsControl.Location = new System.Drawing.Point(3, 16);
            this.requiredSkillsControl.MinimumSize = new System.Drawing.Size(187, 0);
            this.requiredSkillsControl.Name = "requiredSkillsControl";
            this.requiredSkillsControl.Object = null;
            this.requiredSkillsControl.Plan = null;
            this.requiredSkillsControl.Size = new System.Drawing.Size(234, 85);
            this.requiredSkillsControl.TabIndex = 0;
            // 
            // ItemBrowserControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Name = "ItemBrowserControl";
            this.Controls.SetChildIndex(this.scObjectBrowser, 0);
            this.scDetailsRight.Panel1.ResumeLayout(false);
            this.scDetailsRight.Panel2.ResumeLayout(false);
            this.scDetailsRight.ResumeLayout(false);
            this.scDetails.Panel1.ResumeLayout(false);
            this.scDetails.Panel2.ResumeLayout(false);
            this.scDetails.ResumeLayout(false);
            this.gbDescription.ResumeLayout(false);
            this.scObjectBrowser.Panel1.ResumeLayout(false);
            this.scObjectBrowser.Panel1.PerformLayout();
            this.scObjectBrowser.Panel2.ResumeLayout(false);
            this.scObjectBrowser.Panel2.PerformLayout();
            this.scObjectBrowser.ResumeLayout(false);
            this.pnlDetails.ResumeLayout(false);
            this.pnlBrowserHeader.ResumeLayout(false);
            this.pnlBrowserHeader.PerformLayout();
            this.gbAttributes.ResumeLayout(false);
            this.ItemAttributeContextMenu.ResumeLayout(false);
            this.gbRequiredSkills.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lvItemProperties;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private ItemSelectControl itemSelectControl;
        private System.Windows.Forms.ToolTip ttItem;
        private System.Windows.Forms.GroupBox gbAttributes;
        private System.Windows.Forms.ContextMenuStrip ItemAttributeContextMenu;
        private System.Windows.Forms.ToolStripMenuItem exportToCSVToolStripMenuItem;
        private System.Windows.Forms.GroupBox gbRequiredSkills;
        private RequiredSkillsControl requiredSkillsControl;
    }
}
