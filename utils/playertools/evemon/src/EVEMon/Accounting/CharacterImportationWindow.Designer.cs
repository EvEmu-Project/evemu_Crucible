namespace EVEMon.Accounting
{
    partial class CharacterImportationWindow
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
            System.Windows.Forms.Label labelName;
            System.Windows.Forms.Label labelErrorHeader;
            this.urlRadio = new System.Windows.Forms.RadioButton();
            this.urlTextBox = new System.Windows.Forms.TextBox();
            this.fileRadio = new System.Windows.Forms.RadioButton();
            this.fileTextBox = new System.Windows.Forms.TextBox();
            this.fileButton = new System.Windows.Forms.Button();
            this.urlThrobber = new EVEMon.Controls.Throbber();
            this.namePanel = new System.Windows.Forms.Panel();
            this.nameTextBox = new System.Windows.Forms.TextBox();
            this.errorPanel = new System.Windows.Forms.FlowLayoutPanel();
            this.labelError = new System.Windows.Forms.Label();
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.delayQueryTimer = new System.Windows.Forms.Timer(this.components);
            labelName = new System.Windows.Forms.Label();
            labelErrorHeader = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.urlThrobber)).BeginInit();
            this.namePanel.SuspendLayout();
            this.errorPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelName
            // 
            labelName.AutoSize = true;
            labelName.Location = new System.Drawing.Point(5, 7);
            labelName.Name = "labelName";
            labelName.Size = new System.Drawing.Size(34, 13);
            labelName.TabIndex = 0;
            labelName.Text = "Name";
            // 
            // labelErrorHeader
            // 
            labelErrorHeader.AutoSize = true;
            labelErrorHeader.ForeColor = System.Drawing.Color.DarkRed;
            labelErrorHeader.Location = new System.Drawing.Point(3, 0);
            labelErrorHeader.Name = "labelErrorHeader";
            labelErrorHeader.Size = new System.Drawing.Size(272, 13);
            labelErrorHeader.TabIndex = 8;
            labelErrorHeader.Text = "An error occurred while trying to import this character :";
            // 
            // urlRadio
            // 
            this.urlRadio.AutoSize = true;
            this.urlRadio.Checked = true;
            this.urlRadio.Location = new System.Drawing.Point(13, 13);
            this.urlRadio.Name = "urlRadio";
            this.urlRadio.Size = new System.Drawing.Size(65, 17);
            this.urlRadio.TabIndex = 0;
            this.urlRadio.TabStop = true;
            this.urlRadio.Text = "Use URL";
            this.urlRadio.UseVisualStyleBackColor = true;
            this.urlRadio.CheckedChanged += new System.EventHandler(this.radio_CheckedChanged);
            // 
            // urlTextBox
            // 
            this.urlTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.urlTextBox.Location = new System.Drawing.Point(85, 13);
            this.urlTextBox.Name = "urlTextBox";
            this.urlTextBox.Size = new System.Drawing.Size(457, 21);
            this.urlTextBox.TabIndex = 1;
            // 
            // fileRadio
            // 
            this.fileRadio.AutoSize = true;
            this.fileRadio.Location = new System.Drawing.Point(13, 49);
            this.fileRadio.Name = "fileRadio";
            this.fileRadio.Size = new System.Drawing.Size(62, 17);
            this.fileRadio.TabIndex = 2;
            this.fileRadio.TabStop = true;
            this.fileRadio.Text = "Use File";
            this.fileRadio.UseVisualStyleBackColor = true;
            this.fileRadio.CheckedChanged += new System.EventHandler(this.radio_CheckedChanged);
            // 
            // fileTextBox
            // 
            this.fileTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.fileTextBox.Location = new System.Drawing.Point(85, 49);
            this.fileTextBox.Name = "fileTextBox";
            this.fileTextBox.ReadOnly = true;
            this.fileTextBox.Size = new System.Drawing.Size(457, 21);
            this.fileTextBox.TabIndex = 3;
            // 
            // fileButton
            // 
            this.fileButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.fileButton.Location = new System.Drawing.Point(548, 47);
            this.fileButton.Name = "fileButton";
            this.fileButton.Size = new System.Drawing.Size(29, 23);
            this.fileButton.TabIndex = 4;
            this.fileButton.Text = "...";
            this.fileButton.UseVisualStyleBackColor = true;
            this.fileButton.Click += new System.EventHandler(this.fileButton_Click);
            // 
            // urlThrobber
            // 
            this.urlThrobber.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.urlThrobber.Location = new System.Drawing.Point(548, 10);
            this.urlThrobber.MaximumSize = new System.Drawing.Size(24, 24);
            this.urlThrobber.MinimumSize = new System.Drawing.Size(24, 24);
            this.urlThrobber.Name = "urlThrobber";
            this.urlThrobber.Size = new System.Drawing.Size(24, 24);
            this.urlThrobber.State = EVEMon.Controls.ThrobberState.Stopped;
            this.urlThrobber.TabIndex = 5;
            this.urlThrobber.TabStop = false;
            // 
            // namePanel
            // 
            this.namePanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.namePanel.Controls.Add(this.nameTextBox);
            this.namePanel.Controls.Add(labelName);
            this.namePanel.Location = new System.Drawing.Point(85, 87);
            this.namePanel.Name = "namePanel";
            this.namePanel.Size = new System.Drawing.Size(280, 31);
            this.namePanel.TabIndex = 5;
            // 
            // nameTextBox
            // 
            this.nameTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.nameTextBox.Location = new System.Drawing.Point(45, 4);
            this.nameTextBox.Name = "nameTextBox";
            this.nameTextBox.Size = new System.Drawing.Size(232, 21);
            this.nameTextBox.TabIndex = 1;
            // 
            // errorPanel
            // 
            this.errorPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.errorPanel.Controls.Add(labelErrorHeader);
            this.errorPanel.Controls.Add(this.labelError);
            this.errorPanel.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.errorPanel.Location = new System.Drawing.Point(80, 87);
            this.errorPanel.Name = "errorPanel";
            this.errorPanel.Size = new System.Drawing.Size(492, 31);
            this.errorPanel.TabIndex = 9;
            // 
            // labelError
            // 
            this.labelError.AutoSize = true;
            this.labelError.ForeColor = System.Drawing.Color.DarkRed;
            this.labelError.Location = new System.Drawing.Point(3, 13);
            this.labelError.Name = "labelError";
            this.labelError.Size = new System.Drawing.Size(35, 13);
            this.labelError.TabIndex = 9;
            this.labelError.Text = "label2";
            // 
            // okButton
            // 
            this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.okButton.Location = new System.Drawing.Point(416, 126);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 6;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(497, 126);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 7;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            // 
            // delayQueryTimer
            // 
            this.delayQueryTimer.Interval = 400;
            this.delayQueryTimer.Tick += new System.EventHandler(this.delayQueryTimer_Tick);
            // 
            // CharacterImportationWindow
            // 
            this.AcceptButton = this.okButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(589, 161);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.Controls.Add(this.errorPanel);
            this.Controls.Add(this.namePanel);
            this.Controls.Add(this.urlThrobber);
            this.Controls.Add(this.fileButton);
            this.Controls.Add(this.fileTextBox);
            this.Controls.Add(this.fileRadio);
            this.Controls.Add(this.urlTextBox);
            this.Controls.Add(this.urlRadio);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(597, 193);
            this.Name = "CharacterImportationWindow";
            this.Text = "Character Importation";
            ((System.ComponentModel.ISupportInitialize)(this.urlThrobber)).EndInit();
            this.namePanel.ResumeLayout(false);
            this.namePanel.PerformLayout();
            this.errorPanel.ResumeLayout(false);
            this.errorPanel.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        #endregion

        private System.Windows.Forms.RadioButton urlRadio;
        private System.Windows.Forms.TextBox urlTextBox;
        private System.Windows.Forms.RadioButton fileRadio;
        private System.Windows.Forms.TextBox fileTextBox;
        private System.Windows.Forms.Button fileButton;
        private EVEMon.Controls.Throbber urlThrobber;
        private System.Windows.Forms.Panel namePanel;
        private System.Windows.Forms.TextBox nameTextBox;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Timer delayQueryTimer;
        private System.Windows.Forms.FlowLayoutPanel errorPanel;
        private System.Windows.Forms.Label labelError;
    }
}