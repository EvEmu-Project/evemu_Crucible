using System;
using System.Text;
using System.Linq;
using System.Drawing;
using System.Windows.Forms;
using System.ComponentModel;
using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.Notifications;

namespace EVEMon
{
    public partial class SkillCompletionWindow : EVEMonForm
    {
        private SkillCompletionNotification m_notification;

        /// <summary>
        /// Constructor
        /// </summary>
        public SkillCompletionWindow()
        {
            InitializeComponent();
            this.RememberPositionKey = "SkillCompletionWindow";
        }

        /// <summary>
        /// Gets or sets the list of completed skills.
        /// </summary> 
        [Browsable(false)]
        public SkillCompletionNotification Notification
        {
            get { return m_notification; }
            set 
            { 
                m_notification = value;
                StringBuilder text = new StringBuilder();
                foreach (var skill in m_notification.Skills.Reverse())
                {
                    text.AppendLine(skill.ToString());
                }

                Size textSize = TextRenderer.MeasureText(text.ToString(), Font);
                MinimumSize = new Size(280, textSize.Height + 30);

                detailsTextBox.Text = text.ToString();
            }
        }
    }
}
