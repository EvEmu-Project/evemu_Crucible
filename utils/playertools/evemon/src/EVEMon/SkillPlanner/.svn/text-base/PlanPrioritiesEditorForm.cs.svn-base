using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common.Controls;

namespace EVEMon.SkillPlanner
{
    public partial class PlanPrioritiesEditorForm : EVEMonForm
    {
        public PlanPrioritiesEditorForm()
        {
            InitializeComponent();
        }

        public int Priority
        {
            get { return (int)nudPriority.Value; }
            set { nudPriority.Value = value; }
        }
        private void btnOk_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void ChangePriorityForm_Load(object sender, EventArgs e)
        {
            nudPriority.Select(0, 3);
        }
    }

}