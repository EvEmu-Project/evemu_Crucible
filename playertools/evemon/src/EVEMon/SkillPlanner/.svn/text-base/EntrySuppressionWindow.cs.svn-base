using System;
using System.Drawing;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.SkillPlanner
{
    public partial class EntrySuppressionWindow : EVEMonForm
    {
        public EntrySuppressionWindow()
        {
            InitializeComponent();
        }

        public bool CancelMultipleSkills
        {
            set
            {
                string skillWording = (value) ? "these skills" : "this skill";
                lbQuestion.Text = String.Format(lbQuestion.Text,skillWording);
                btnThisOnly.Text = "Just " + skillWording;
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void btnThisOnly_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.No;
            this.Close();
        }

        private void btnAll_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Yes;
            this.Close();
        }

        private void CancelChoiceWindow_Load(object sender, EventArgs e)
        {
            int width = SystemIcons.Question.Width;
            int height = SystemIcons.Question.Height;

            Bitmap b = new Bitmap(width, height);
            using (Graphics g = Graphics.FromImage(b))
            {
                g.DrawIcon(SystemIcons.Question, 0, 0);
            }

            pictureBox1.Image = b;
        }
    }
}