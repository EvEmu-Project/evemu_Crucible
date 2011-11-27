namespace EVEMon.SkillPlanner
{
    partial class ObsoleteEntriesForm
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
            this.MainTableLayoutPanel = new System.Windows.Forms.TableLayoutPanel();
            this.InformationTextLabel = new System.Windows.Forms.Label();
            this.ObsoleteEntriesListView = new System.Windows.Forms.ListView();
            this.EntryColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.StatusColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.ButtonFlowLayoutPanel = new System.Windows.Forms.FlowLayoutPanel();
            this.KeepAllButton = new System.Windows.Forms.Button();
            this.RemoveAllButton = new System.Windows.Forms.Button();
            this.RemoveConfirmedButton = new System.Windows.Forms.Button();
            this.MainTableLayoutPanel.SuspendLayout();
            this.ButtonFlowLayoutPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // MainTableLayoutPanel
            // 
            this.MainTableLayoutPanel.ColumnCount = 1;
            this.MainTableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.MainTableLayoutPanel.Controls.Add(this.InformationTextLabel, 0, 0);
            this.MainTableLayoutPanel.Controls.Add(this.ObsoleteEntriesListView, 0, 1);
            this.MainTableLayoutPanel.Controls.Add(this.ButtonFlowLayoutPanel, 0, 2);
            this.MainTableLayoutPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.MainTableLayoutPanel.Location = new System.Drawing.Point(0, 0);
            this.MainTableLayoutPanel.Name = "MainTableLayoutPanel";
            this.MainTableLayoutPanel.RowCount = 3;
            this.MainTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.MainTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.MainTableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.MainTableLayoutPanel.Size = new System.Drawing.Size(395, 456);
            this.MainTableLayoutPanel.TabIndex = 0;
            // 
            // InformationTextLabel
            // 
            this.InformationTextLabel.AutoSize = true;
            this.InformationTextLabel.Location = new System.Drawing.Point(8, 8);
            this.InformationTextLabel.Margin = new System.Windows.Forms.Padding(8);
            this.InformationTextLabel.Name = "InformationTextLabel";
            this.InformationTextLabel.Size = new System.Drawing.Size(369, 26);
            this.InformationTextLabel.TabIndex = 0;
            this.InformationTextLabel.Text = "One or more skills exist in this plan that may have already completed, what do yo" +
                "u want to do with these entries?";
            // 
            // ObsoleteEntriesListView
            // 
            this.ObsoleteEntriesListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.EntryColumnHeader,
            this.StatusColumnHeader});
            this.ObsoleteEntriesListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ObsoleteEntriesListView.Location = new System.Drawing.Point(10, 52);
            this.ObsoleteEntriesListView.Margin = new System.Windows.Forms.Padding(10, 10, 10, 3);
            this.ObsoleteEntriesListView.Name = "ObsoleteEntriesListView";
            this.ObsoleteEntriesListView.Size = new System.Drawing.Size(375, 357);
            this.ObsoleteEntriesListView.TabIndex = 0;
            this.ObsoleteEntriesListView.UseCompatibleStateImageBehavior = false;
            this.ObsoleteEntriesListView.View = System.Windows.Forms.View.Details;
            // 
            // EntryColumnHeader
            // 
            this.EntryColumnHeader.Text = "Planned Entry";
            this.EntryColumnHeader.Width = 123;
            // 
            // StatusColumnHeader
            // 
            this.StatusColumnHeader.Text = "Status";
            // 
            // ButtonFlowLayoutPanel
            // 
            this.ButtonFlowLayoutPanel.AutoSize = true;
            this.ButtonFlowLayoutPanel.Controls.Add(this.KeepAllButton);
            this.ButtonFlowLayoutPanel.Controls.Add(this.RemoveAllButton);
            this.ButtonFlowLayoutPanel.Controls.Add(this.RemoveConfirmedButton);
            this.ButtonFlowLayoutPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ButtonFlowLayoutPanel.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.ButtonFlowLayoutPanel.Location = new System.Drawing.Point(3, 415);
            this.ButtonFlowLayoutPanel.Name = "ButtonFlowLayoutPanel";
            this.ButtonFlowLayoutPanel.Size = new System.Drawing.Size(389, 38);
            this.ButtonFlowLayoutPanel.TabIndex = 2;
            // 
            // KeepAllButton
            // 
            this.KeepAllButton.Location = new System.Drawing.Point(275, 3);
            this.KeepAllButton.Margin = new System.Windows.Forms.Padding(3, 3, 6, 8);
            this.KeepAllButton.Name = "KeepAllButton";
            this.KeepAllButton.Size = new System.Drawing.Size(108, 27);
            this.KeepAllButton.TabIndex = 3;
            this.KeepAllButton.Text = "&Keep All";
            this.KeepAllButton.UseVisualStyleBackColor = true;
            this.KeepAllButton.MouseLeave += new System.EventHandler(this.AnyButton_Leave);
            this.KeepAllButton.Click += new System.EventHandler(this.KeepAllButton_Click);
            this.KeepAllButton.Leave += new System.EventHandler(this.AnyButton_Leave);
            this.KeepAllButton.Enter += new System.EventHandler(this.KeepAllButton_Focused);
            this.KeepAllButton.MouseHover += new System.EventHandler(this.KeepAllButton_Focused);
            // 
            // RemoveAllButton
            // 
            this.RemoveAllButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.RemoveAllButton.Location = new System.Drawing.Point(161, 3);
            this.RemoveAllButton.Margin = new System.Windows.Forms.Padding(3, 3, 3, 8);
            this.RemoveAllButton.Name = "RemoveAllButton";
            this.RemoveAllButton.Size = new System.Drawing.Size(108, 27);
            this.RemoveAllButton.TabIndex = 2;
            this.RemoveAllButton.Text = "&Remove All";
            this.RemoveAllButton.UseVisualStyleBackColor = true;
            this.RemoveAllButton.MouseLeave += new System.EventHandler(this.AnyButton_Leave);
            this.RemoveAllButton.Click += new System.EventHandler(this.RemoveAllButton_Click);
            this.RemoveAllButton.Leave += new System.EventHandler(this.AnyButton_Leave);
            this.RemoveAllButton.Enter += new System.EventHandler(this.RemoveAllButton_Focused);
            this.RemoveAllButton.MouseHover += new System.EventHandler(this.RemoveAllButton_Focused);
            // 
            // RemoveConfirmedButton
            // 
            this.RemoveConfirmedButton.Location = new System.Drawing.Point(47, 3);
            this.RemoveConfirmedButton.Margin = new System.Windows.Forms.Padding(3, 3, 3, 8);
            this.RemoveConfirmedButton.Name = "RemoveConfirmedButton";
            this.RemoveConfirmedButton.Size = new System.Drawing.Size(108, 27);
            this.RemoveConfirmedButton.TabIndex = 1;
            this.RemoveConfirmedButton.Text = "Remove &Confirmed";
            this.RemoveConfirmedButton.UseVisualStyleBackColor = true;
            this.RemoveConfirmedButton.MouseLeave += new System.EventHandler(this.AnyButton_Leave);
            this.RemoveConfirmedButton.Click += new System.EventHandler(this.RemoveConfirmedButton_Click);
            this.RemoveConfirmedButton.Leave += new System.EventHandler(this.AnyButton_Leave);
            this.RemoveConfirmedButton.Enter += new System.EventHandler(this.RemoveConfirmedButton_Focused);
            this.RemoveConfirmedButton.MouseHover += new System.EventHandler(this.RemoveConfirmedButton_Focused);
            // 
            // ObsoleteEntriesForm
            // 
            this.AcceptButton = this.KeepAllButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.RemoveAllButton;
            this.ClientSize = new System.Drawing.Size(395, 456);
            this.Controls.Add(this.MainTableLayoutPanel);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ObsoleteEntriesForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Remove Obsolete Entries";
            this.MainTableLayoutPanel.ResumeLayout(false);
            this.MainTableLayoutPanel.PerformLayout();
            this.ButtonFlowLayoutPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel MainTableLayoutPanel;
        private System.Windows.Forms.Label InformationTextLabel;
        private System.Windows.Forms.ListView ObsoleteEntriesListView;
        private System.Windows.Forms.ColumnHeader EntryColumnHeader;
        private System.Windows.Forms.ColumnHeader StatusColumnHeader;
        private System.Windows.Forms.FlowLayoutPanel ButtonFlowLayoutPanel;
        private System.Windows.Forms.Button KeepAllButton;
        private System.Windows.Forms.Button RemoveAllButton;
        private System.Windows.Forms.Button RemoveConfirmedButton;
    }
}