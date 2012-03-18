using System;
using System.Data;
using System.Text;
using System.Linq;
using System.Drawing;
using System.Windows.Forms;
using System.ComponentModel;
using System.Collections.Generic;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.SettingsObjects;

namespace EVEMon
{
    public partial class IndustryJobsWindow : EVEMonForm
    {
        private bool m_init;

        public IndustryJobsWindow()
        {
            InitializeComponent();
            this.RememberPositionKey = "IndustryJobsWindow";
            m_init = true;
        }

        /// <summary>
        /// Gets or sets the grouping mode.
        /// </summary>
        public Enum Grouping
        {
            get { return jobsList.Grouping; }
            set 
            {
                jobsList.Grouping = value;

                if (m_init)
                {
                    jobsList.UpdateColumns();
                    jobsList.lvJobs.Visible = !jobsList.Jobs.IsEmpty();
                }
            }
        }

        /// <summary>
        /// Gets or sets the showIssuedFor mode.
        /// </summary>
        public IssuedFor ShowIssuedFor
        {
            get { return jobsList.ShowIssuedFor; }
            set 
            {
                jobsList.ShowIssuedFor = value;

                if (m_init)
                {
                    jobsList.UpdateColumns();
                    jobsList.lvJobs.Visible = !jobsList.Jobs.IsEmpty();
                }
            }
        }

        /// <summary>
        /// Gets or sets the enumeration of jobs to display.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public IEnumerable<IndustryJob> Jobs
        {
            get { return jobsList.Jobs; }
            set { jobsList.Jobs = value; }
        }

        /// <summary>
        /// Gets or sets the enumeration of displayed columns.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public IEnumerable<IndustryJobColumnSettings> Columns
        {
            get { return jobsList.Columns; }
            set
            {
                jobsList.Columns = value;

                if (m_init)
                {
                    jobsList.UpdateColumns();
                    jobsList.lvJobs.Visible = !jobsList.Jobs.IsEmpty();
                }
            }
        }
    }
}
