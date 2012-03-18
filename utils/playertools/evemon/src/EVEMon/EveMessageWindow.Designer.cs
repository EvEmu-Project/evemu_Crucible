namespace EVEMon
{
    partial class EveMessageWindow
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
            this.readingPane = new EVEMon.ReadingPane();
            this.SuspendLayout();
            // 
            // eveMailReadingPane
            // 
            this.readingPane.Dock = System.Windows.Forms.DockStyle.Fill;
            this.readingPane.Location = new System.Drawing.Point(0, 0);
            this.readingPane.Name = "eveMailReadingPane";
            this.readingPane.Size = new System.Drawing.Size(584, 232);
            this.readingPane.TabIndex = 0;
            // 
            // EVEMailWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 232);
            this.Controls.Add(this.readingPane);
            this.Name = "EVEMailWindow";
            this.Text = "EVE Mail Message";
            this.ResumeLayout(false);

        }

        #endregion

        private ReadingPane readingPane;
    }
}
