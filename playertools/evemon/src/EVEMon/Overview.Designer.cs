namespace EVEMon
{
    partial class Overview
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
            this.labelNoCharacters = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // labelNoCharacters
            // 
            this.labelNoCharacters.BackColor = System.Drawing.Color.Transparent;
            this.labelNoCharacters.Dock = System.Windows.Forms.DockStyle.Fill;
            this.labelNoCharacters.Location = new System.Drawing.Point(0, 0);
            this.labelNoCharacters.Name = "labelNoCharacters";
            this.labelNoCharacters.Size = new System.Drawing.Size(361, 507);
            this.labelNoCharacters.TabIndex = 0;
            this.labelNoCharacters.Text = "No character loaded.\r\nTo add characters, click the File|Add Account... menu optio" +
                "n";
            this.labelNoCharacters.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // Overview
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.Transparent;
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.Controls.Add(this.labelNoCharacters);
            this.Name = "Overview";
            this.Size = new System.Drawing.Size(361, 507);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label labelNoCharacters;
    }
}
