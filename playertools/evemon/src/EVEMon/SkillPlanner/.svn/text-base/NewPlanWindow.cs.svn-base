using System;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.SkillPlanner
{
    public partial class NewPlanWindow : EVEMonForm
    {
        public NewPlanWindow()
        {
            InitializeComponent();
        }

        private String m_planName = String.Empty;
        public String PlanName
        {
            set { m_planName = value; }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private string m_result = String.Empty;

        public string Result
        {
            get { return m_result; }
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            m_result = textBox1.Text;
            DialogResult = DialogResult.OK;
            this.Close();
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            btnOk.Enabled = (!String.IsNullOrEmpty(textBox1.Text));
        }

        private void NewPlanWindow_Shown(object sender, EventArgs e)
        {
            textBox1.Text = m_planName;
            textBox1.SelectAll();
        }

    }
}
