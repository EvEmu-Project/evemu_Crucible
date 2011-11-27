namespace EVEMon.Common.Controls
{
    partial class CharacterPortrait
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CharacterPortrait));
            this.pictureBox = new System.Windows.Forms.PictureBox();
            this.cmsPictureOptions = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.updatePicture = new System.Windows.Forms.ToolStripMenuItem();
            this.updatePictureFromEVECache = new System.Windows.Forms.ToolStripMenuItem();
            this.setEVEFolder = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).BeginInit();
            this.cmsPictureOptions.SuspendLayout();
            this.SuspendLayout();
            // 
            // pictureBox
            // 
            this.pictureBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pictureBox.InitialImage = ((System.Drawing.Image)(resources.GetObject("pictureBox.InitialImage")));
            this.pictureBox.Location = new System.Drawing.Point(0, 0);
            this.pictureBox.Name = "pictureBox";
            this.pictureBox.Size = new System.Drawing.Size(256, 256);
            this.pictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox.TabIndex = 0;
            this.pictureBox.TabStop = false;
            this.pictureBox.Click += new System.EventHandler(this.pictureBox_Click);
            // 
            // cmsPictureOptions
            // 
            this.cmsPictureOptions.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.updatePicture,
            this.updatePictureFromEVECache,
            this.setEVEFolder});
            this.cmsPictureOptions.Name = "contextMenuStrip1";
            this.cmsPictureOptions.Size = new System.Drawing.Size(241, 70);
            // 
            // updatePicture
            // 
            this.updatePicture.Name = "updatePicture";
            this.updatePicture.Size = new System.Drawing.Size(240, 22);
            this.updatePicture.Text = "Update Portrait From The Web";
            this.updatePicture.Click += new System.EventHandler(this.miUpdatePicture_Click);
            // 
            // updatePictureFromEVECache
            // 
            this.updatePictureFromEVECache.Name = "updatePictureFromEVECache";
            this.updatePictureFromEVECache.Size = new System.Drawing.Size(240, 22);
            this.updatePictureFromEVECache.Text = "Update Portrait From EVE Cache";
            this.updatePictureFromEVECache.Click += new System.EventHandler(this.miUpdatePictureFromEVECache_Click);
            // 
            // setEVEFolder
            // 
            this.setEVEFolder.Name = "setEVEFolder";
            this.setEVEFolder.Size = new System.Drawing.Size(240, 22);
            this.setEVEFolder.Text = "Set Portrait Folder";
            this.setEVEFolder.Click += new System.EventHandler(this.miSetEVEFolder_Click);
            // 
            // CharacterPortrait
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.pictureBox);
            this.Name = "CharacterPortrait";
            this.Size = new System.Drawing.Size(256, 256);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).EndInit();
            this.cmsPictureOptions.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion

        private System.Windows.Forms.ToolStripMenuItem updatePicture;
        private System.Windows.Forms.ToolStripMenuItem updatePictureFromEVECache;
        private System.Windows.Forms.ToolStripMenuItem setEVEFolder;
        private System.Windows.Forms.ContextMenuStrip cmsPictureOptions;
        private System.Windows.Forms.PictureBox pictureBox;
    }
}
