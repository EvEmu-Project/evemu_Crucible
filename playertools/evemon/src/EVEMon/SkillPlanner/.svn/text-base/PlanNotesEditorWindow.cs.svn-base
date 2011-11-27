using System;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.SkillPlanner
{
    public partial class PlanNotesEditorWindow : EVEMonForm
    {
        public PlanNotesEditorWindow()
        {
            InitializeComponent();
        }

        public PlanNotesEditorWindow(string skillName)
            : this()
        {
            this.Text = "Notes for " + skillName;
        }

        public string NoteText
        {
            get { return textBox1.Text; }
            set
            {
                if (String.IsNullOrEmpty(value))
                {
                    value = String.Empty;
                }
                textBox1.Lines = value.Split(new string[4] {"\r\n", "\n\r", "\r", "\n"}, StringSplitOptions.None);
            }
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

		private void textBox1_KeyDown(object sender, KeyEventArgs e) {
			if (e.Control && e.KeyCode == Keys.Enter)
				btnOk_Click(sender, e);
		}
    }
}
