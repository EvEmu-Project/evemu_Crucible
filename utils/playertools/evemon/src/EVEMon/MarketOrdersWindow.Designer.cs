namespace EVEMon
{
    partial class MarketOrdersWindow
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
            this.ordersList = new EVEMon.MainWindowMarketOrdersList();
            this.SuspendLayout();
            // 
            // ordersList
            // 
            this.ordersList.Character = null;
            this.ordersList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ordersList.Grouping = EVEMon.Common.SettingsObjects.MarketOrderGrouping.State;
            this.ordersList.Location = new System.Drawing.Point(0, 0);
            this.ordersList.Name = "ordersList";
            this.ordersList.ShowIssuedFor = EVEMon.Common.IssuedFor.All;
            this.ordersList.Size = new System.Drawing.Size(292, 266);
            this.ordersList.TabIndex = 0;
            this.ordersList.TextFilter = "";
            // 
            // MarketOrdersWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(292, 266);
            this.Controls.Add(this.ordersList);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MarketOrdersWindow";
            this.Text = "Notification Details";
            this.ResumeLayout(false);

        }

        #endregion

        private MainWindowMarketOrdersList ordersList;
    }
}