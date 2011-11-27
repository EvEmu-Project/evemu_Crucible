namespace EVEMon.SkillPlanner
{
    partial class SkillTreeDisplay
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
            this.tmrSkillTick = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // tmrSkillTick
            // 
            this.tmrSkillTick.Interval = 1000;
            this.tmrSkillTick.Tick += new System.EventHandler(this.tmrSkillTick_Tick);
            // 
            // SkillTreeDisplay
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.DoubleBuffered = true;
            this.Name = "SkillTreeDisplay";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Timer tmrSkillTick;
    }
}
