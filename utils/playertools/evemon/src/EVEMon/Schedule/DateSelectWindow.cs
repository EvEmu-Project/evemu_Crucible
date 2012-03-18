using System;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.Schedule
{
    public partial class DateSelectWindow : EVEMonForm
    {
        public DateSelectWindow()
        {
            InitializeComponent();
        }

        private void DateSelectWindow_Load(object sender, EventArgs e)
        {
            //monthCalendar1.MinDate = DateTime.Today;
        }

        public DateTime SelectedDate
        {
            get { return monthCalendar1.SelectionStart; }
            set
            {
                monthCalendar1.SelectionStart = value;
                monthCalendar1.SelectionEnd = value;
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private int m_numClicks = 0;
        private DateTime m_firstClick = DateTime.MinValue;

        private void monthCalendar1_DateSelected(object sender, DateRangeEventArgs e)
        {
            if (m_numClicks >= 2)
            {
                btnOk_Click(this, new EventArgs());
            }
        }

        private void monthCalendar1_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                if (DateTime.Now - TimeSpan.FromMilliseconds(500) > m_firstClick)
                {
                    m_numClicks = 0;
                    m_firstClick = DateTime.Now;
                }
                m_numClicks++;
            }
        }
    }
}