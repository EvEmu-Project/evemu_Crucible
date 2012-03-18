using System;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Data;

namespace EVEMon.SkillPlanner
{
    public partial class ShipBrowserControl : EveObjectBrowserControl
    {
        /// <summary>
        /// Default constructor.
        /// </summary>
        public ShipBrowserControl()
        {
            InitializeComponent();
            scObjectBrowser.RememberDistanceKey = "ShipsBrowser_Left";
            Initialize(lvShipProperties, shipSelectControl, true);
        }

        #region Event Handlers
        /// <summary>
        /// Opens the BattleClinic Loadout window.
        /// </summary>
        private void lblBattleclinic_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            var window = WindowsFactory<ShipLoadoutSelectWindow>.ShowUnique(() => new ShipLoadoutSelectWindow(SelectedObject as Item, Plan));
        }

        /// <summary>
        /// Exports item info to CSV format.
        /// </summary>
        private void exportToCSVToolStripMenuItem_Click(object sender, EventArgs e) 
        {
            ListViewExporter.CreateCSV(lvShipProperties);
        }
        #endregion

        #region Methods
        /// <summary>
        /// Updates the controls when the selection is changed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected override void OnSelectionChanged(object sender, EventArgs e)
        {
            base.OnSelectionChanged(sender, e);
            if (SelectedObject == null)
                return;

            // Description
            tbDescription.Text = SelectedObject.Description;

            // Recommended Certificates
            recommendedCertificatesControl.Object = SelectedObject;
            
            // Required Skills
            requiredSkillsControl.Object = SelectedObject;

            var loadoutSelect = WindowsFactory<ShipLoadoutSelectWindow>.GetUnique();
            if (loadoutSelect != null)
                loadoutSelect.Ship = (Item)shipSelectControl.SelectedObject;
        }

        /// <summary>
        /// Updates the plan when the selection is changed.
        /// </summary>
        protected override void OnPlanChanged()
        {
            base.OnPlanChanged();
            recommendedCertificatesControl.Plan = Plan;
            requiredSkillsControl.Plan = Plan;

            // We recalculate the right panels minimum size
            int reqSkillControlMinWidth = requiredSkillsControl.MinimumSize.Width;
            int reqSkillPanelMinWidth = scDetails.Panel2MinSize;
            scDetails.Panel2MinSize = (reqSkillPanelMinWidth > reqSkillControlMinWidth ?
                                         reqSkillPanelMinWidth : reqSkillControlMinWidth);
        }
        #endregion
    }
}


