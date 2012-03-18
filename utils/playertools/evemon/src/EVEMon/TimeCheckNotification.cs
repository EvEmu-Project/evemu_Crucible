using EVEMon.Common;
using System;
using EVEMon.Common.Controls;

namespace EVEMon
{
    /// <summary>
    /// Notification dialog to inform user of a possible clock error
    /// </summary>
    public partial class TimeCheckNotification : EVEMonForm
    {
        private readonly DateTime m_serverTime;
        private readonly DateTime m_localTime;

        public TimeCheckNotification()
        {
            InitializeComponent();
            this.uxTitleLabel.Font = FontFactory.GetFont("Tahoma", 12F);
        }

        public TimeCheckNotification(DateTime serverTime, DateTime localTime)
            : this ()
        {
            m_serverTime = serverTime;
            m_localTime = localTime;
        }

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            uxTimeZoneField.Text = TimeZone.CurrentTimeZone.StandardName;
            uxExpectedTimeField.Text = m_serverTime.ToString();
            uxActualTimeField.Text = m_localTime.ToString();
            uxCheckTimeOnStartUpCheckBox.Checked = Settings.Updates.CheckTimeOnStartup;
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            Settings.Updates.CheckTimeOnStartup = uxCheckTimeOnStartUpCheckBox.Checked;
            base.OnClosing(e);
        }

        private void uxCloseButton_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}

