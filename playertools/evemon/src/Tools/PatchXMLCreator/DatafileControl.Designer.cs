namespace PatchXmlCreator
{
    partial class DatafileControl
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
            this.gbDatafile = new System.Windows.Forms.GroupBox();
            this.dtpDatafiles = new System.Windows.Forms.DateTimePicker();
            this.lblDatafileDate = new System.Windows.Forms.Label();
            this.lblMD5Sum = new System.Windows.Forms.Label();
            this.lblDatafileMessage = new System.Windows.Forms.Label();
            this.rtbDatafileMessage = new System.Windows.Forms.RichTextBox();
            this.gbDatafile.SuspendLayout();
            this.SuspendLayout();
            // 
            // gbDatafile
            // 
            this.gbDatafile.Controls.Add(this.dtpDatafiles);
            this.gbDatafile.Controls.Add(this.lblDatafileDate);
            this.gbDatafile.Controls.Add(this.lblMD5Sum);
            this.gbDatafile.Controls.Add(this.lblDatafileMessage);
            this.gbDatafile.Controls.Add(this.rtbDatafileMessage);
            this.gbDatafile.Dock = System.Windows.Forms.DockStyle.Fill;
            this.gbDatafile.Location = new System.Drawing.Point(0, 0);
            this.gbDatafile.Name = "gbDatafile";
            this.gbDatafile.Size = new System.Drawing.Size(398, 92);
            this.gbDatafile.TabIndex = 17;
            this.gbDatafile.TabStop = false;
            this.gbDatafile.Text = "eve-datafilename-culture-xml.gz";
            this.gbDatafile.MouseCaptureChanged += new System.EventHandler(this.OnClick);
            // 
            // dtpDatafiles
            // 
            this.dtpDatafiles.CustomFormat = " dd MMMM yyyy";
            this.dtpDatafiles.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.dtpDatafiles.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.dtpDatafiles.Location = new System.Drawing.Point(47, 18);
            this.dtpDatafiles.Name = "dtpDatafiles";
            this.dtpDatafiles.Size = new System.Drawing.Size(166, 21);
            this.dtpDatafiles.TabIndex = 0;
            this.dtpDatafiles.Value = new System.DateTime(2010, 1, 1, 0, 0, 0, 0);
            // 
            // lblDatafileDate
            // 
            this.lblDatafileDate.AutoSize = true;
            this.lblDatafileDate.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.lblDatafileDate.Location = new System.Drawing.Point(4, 21);
            this.lblDatafileDate.Name = "lblDatafileDate";
            this.lblDatafileDate.Size = new System.Drawing.Size(37, 13);
            this.lblDatafileDate.TabIndex = 3;
            this.lblDatafileDate.Text = "Date :";
            // 
            // lblMD5Sum
            // 
            this.lblMD5Sum.AutoSize = true;
            this.lblMD5Sum.Location = new System.Drawing.Point(219, 21);
            this.lblMD5Sum.Name = "lblMD5Sum";
            this.lblMD5Sum.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.lblMD5Sum.Size = new System.Drawing.Size(54, 13);
            this.lblMD5Sum.TabIndex = 2;
            this.lblMD5Sum.Text = "MD5 Sum";
            this.lblMD5Sum.Click += new System.EventHandler(this.OnClick);
            // 
            // lblDatafileMessage
            // 
            this.lblDatafileMessage.AutoSize = true;
            this.lblDatafileMessage.Location = new System.Drawing.Point(4, 45);
            this.lblDatafileMessage.Name = "lblDatafileMessage";
            this.lblDatafileMessage.Size = new System.Drawing.Size(56, 13);
            this.lblDatafileMessage.TabIndex = 4;
            this.lblDatafileMessage.Text = "Message :";
            this.lblDatafileMessage.Click += new System.EventHandler(this.OnClick);
            // 
            // rtbDatafileMessage
            // 
            this.rtbDatafileMessage.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.rtbDatafileMessage.DetectUrls = false;
            this.rtbDatafileMessage.Location = new System.Drawing.Point(63, 42);
            this.rtbDatafileMessage.Name = "rtbDatafileMessage";
            this.rtbDatafileMessage.Size = new System.Drawing.Size(329, 44);
            this.rtbDatafileMessage.TabIndex = 1;
            this.rtbDatafileMessage.Text = "{0} {1} ({2}) {3} data file by the EVEMon Development Team";
            // 
            // DatafileControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.gbDatafile);
            this.Name = "DatafileControl";
            this.Size = new System.Drawing.Size(398, 92);
            this.gbDatafile.ResumeLayout(false);
            this.gbDatafile.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label lblDatafileMessage;
        protected internal System.Windows.Forms.GroupBox gbDatafile;
        protected internal System.Windows.Forms.Label lblMD5Sum;
        protected internal System.Windows.Forms.RichTextBox rtbDatafileMessage;
        private System.Windows.Forms.Label lblDatafileDate;
        protected internal System.Windows.Forms.DateTimePicker dtpDatafiles;

    }
}
