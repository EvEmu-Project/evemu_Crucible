namespace EVEMon.Watchdog
{
    partial class WatchdogWindow
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
            this.TimerPictureBox = new System.Windows.Forms.PictureBox();
            this.StatusLabel = new System.Windows.Forms.Label();
            this.WaitTimer = new System.Windows.Forms.Timer(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.TimerPictureBox)).BeginInit();
            this.SuspendLayout();
            // 
            // TimerPictureBox
            // 
            this.TimerPictureBox.Image = global::EVEMon.Watchdog.Properties.Resources.TimeIcon;
            this.TimerPictureBox.Location = new System.Drawing.Point(12, 12);
            this.TimerPictureBox.Name = "TimerPictureBox";
            this.TimerPictureBox.Size = new System.Drawing.Size(16, 16);
            this.TimerPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.TimerPictureBox.TabIndex = 0;
            this.TimerPictureBox.TabStop = false;
            // 
            // StatusLabel
            // 
            this.StatusLabel.AutoSize = true;
            this.StatusLabel.Location = new System.Drawing.Point(35, 14);
            this.StatusLabel.Name = "StatusLabel";
            this.StatusLabel.Size = new System.Drawing.Size(146, 13);
            this.StatusLabel.TabIndex = 1;
            this.StatusLabel.Text = "Waiting for EVEMon to close.";
            // 
            // WaitTimer
            // 
            this.WaitTimer.Interval = 1000;
            this.WaitTimer.Tick += new System.EventHandler(this.WaitTimer_Tick);
            // 
            // WatchdogWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(286, 41);
            this.Controls.Add(this.StatusLabel);
            this.Controls.Add(this.TimerPictureBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "WatchdogWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "EVEMon Watchdog";
            this.Load += new System.EventHandler(this.WatchdogWindow_Load);
            ((System.ComponentModel.ISupportInitialize)(this.TimerPictureBox)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox TimerPictureBox;
        private System.Windows.Forms.Label StatusLabel;
        private System.Windows.Forms.Timer WaitTimer;
    }
}

