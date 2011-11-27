using System;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Net;
using EVEMon.Common.Controls;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.SettingsUI
{
    public partial class ProxyAuthenticationWindow : EVEMonForm
    {
        private ProxySettings m_proxySetting;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="settings"></param>
        public ProxyAuthenticationWindow(ProxySettings settings)
        {
            m_proxySetting = settings;
            InitializeComponent();
            UpdateFields();
        }

        private void UpdateFields()
        {
            if (m_proxySetting != null)
            {
                switch (m_proxySetting.Authentication)
                {
                    case ProxyAuthentication.None:
                        rbNoAuth.Checked = true;
                        break;

                    case ProxyAuthentication.SystemDefault:
                        rbSystemDefault.Checked = true;
                        break;

                    case ProxyAuthentication.Specified:
                        tbUsername.Text = m_proxySetting.Username;
                        tbPassword.Text = m_proxySetting.Password;
                        rbSuppliedAuth.Checked = true;
                        break;
                }
            }
        }

        private void rbSuppliedAuth_CheckedChanged(object sender, EventArgs e)
        {
            tlpSpecifiedAuth.Enabled = rbSuppliedAuth.Checked;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            if (rbNoAuth.Checked)
            {
                m_proxySetting.Authentication = ProxyAuthentication.None;
            }
            else if (rbSystemDefault.Checked)
            {
                m_proxySetting.Authentication = ProxyAuthentication.SystemDefault;
            }
            else if (rbSuppliedAuth.Checked)
            {
                m_proxySetting.Authentication = ProxyAuthentication.Specified;
                m_proxySetting.Username = tbUsername.Text;
                m_proxySetting.Password = tbPassword.Text;
            }

            this.DialogResult = DialogResult.OK;
            this.Close();
        }
    }
}