using System;
using System.Collections.Generic;
using System.Text;
using System.Globalization;

using EVEMon.Common.Serialization.API;

namespace EVEMon.Common.Notifications
{
    /// <summary>
    /// Provides notification services for IndustryJobs.
    /// </summary>
    public sealed class IndustryJobsNotification : Notification
    {
        private readonly List<IndustryJob> m_jobs;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="jobs">The jobs.</param>
        public IndustryJobsNotification(Object sender, IEnumerable<IndustryJob> jobs)
            : base(NotificationCategory.IndustryJobsCompletion, sender)
        {
            m_jobs = new List<IndustryJob>(jobs);
            UpdateDescription();
        }

        /// <summary>
        /// Gets the associated API result.
        /// </summary>
        public IEnumerable<IndustryJob> Jobs
        {
            get
            {
                foreach (var job in m_jobs)
                {
                    yield return job;
                }
            }
        }

        /// <summary>
        /// Gets true if the notification has details.
        /// </summary>
        public override bool HasDetails
        {
            get { return true; }
        }

        /// <summary>
        /// Enqueue the jobs from the given notification at the end of this notification.
        /// </summary>
        /// <param name="other"></param>
        public override void Append(Notification other)
        {
            var jobs = ((IndustryJobsNotification)other).m_jobs;
            m_jobs.AddRange(jobs);
            UpdateDescription();
        }

        /// <summary>
        /// Updates the description.
        /// </summary>
        private void UpdateDescription()
        {
            m_description = String.Format(CultureConstants.DefaultCulture, "{0} industry job{1} completed.", m_jobs.Count, (m_jobs.Count > 1 ? "s" : String.Empty));
        }
    }
}
