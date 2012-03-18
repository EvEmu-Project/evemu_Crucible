using System;
using System.Text;
using System.Globalization;
using System.Collections.Generic;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common.Notifications
{
    public sealed class SkillCompletionNotification : Notification
    {
        private readonly List<QueuedSkill> m_skills;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="skills"></param>
        public SkillCompletionNotification(Object sender, IEnumerable<QueuedSkill> skills)
            : base(NotificationCategory.SkillCompletion, sender)
        {
            m_skills = new List<QueuedSkill>();
            foreach (var skill in skills)
            {
                m_skills.Add(skill);
            }
            m_skills.Reverse();
            UpdateDescription();
        }

        /// <summary>
        /// Gets the associated API result.
        /// </summary>
        public IEnumerable<QueuedSkill> Skills
        {
            get 
            {
                foreach (var skill in m_skills) yield return skill;
            }
        }

        /// <summary>
        /// Gets true if the notification has details.
        /// </summary>
        public override bool HasDetails
        {
            get
            {
                if (m_skills.Count == 1)
                    return false;
                return true;
            }
        }

        /// <summary>
        /// Enqueue the skills from the given notification at the end of this notification.
        /// </summary>
        /// <param name="other"></param>
        public override void Append(Notification other)
        {
            var skills = ((SkillCompletionNotification)other).m_skills;
            foreach (var skill in skills)
            {
                if (!m_skills.Contains(skill))
                    m_skills.Add(skill);
            }
            UpdateDescription();
        }


        /// <summary>
        /// Updates the description.
        /// </summary>
        private void UpdateDescription()
        {
            if (m_skills.Count == 1)
            {
                m_description = String.Format(CultureConstants.DefaultCulture, "{0} {1} completed.", m_skills[0].SkillName, Skill.GetRomanForInt(m_skills[0].Level));
            }
            else
            {
                m_description = String.Format(CultureConstants.DefaultCulture, "{0} skill{1} completed.", m_skills.Count, m_skills.Count > 1 ? "s" : String.Empty);
            }
        }
    }
}
