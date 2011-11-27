using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    /// <summary>
    /// A collection of industry jobs.
    /// </summary>
    public sealed class IndustryJobCollection : ReadonlyCollection<IndustryJob>
    {
        private readonly Character m_character;

        /// <summary>
        /// Internal constructor.
        /// </summary>
        internal IndustryJobCollection(Character character)
        {
            m_character = character;
        }

        /// <summary>
        /// Imports an enumeration of serialization objects.
        /// </summary>
        /// <param name="src"></param>
        internal void Import(IEnumerable<SerializableJob> src)
        {
            m_items.Clear();
            foreach (var srcJob in src)
            {
                m_items.Add(new IndustryJob(srcJob));
            }
        }

        /// <summary>
        /// Imports an enumeration of API objects.
        /// </summary>
        /// <param name="src">The enumeration of serializable jobs from the API.</param>
        internal void Import(IEnumerable<SerializableJobListItem> src)
        {
            // Mark all jobs for deletion 
            // If they are found again on the API feed, they won't be deleted
            // and those set as ignored will be left as ignored
            foreach (var job in m_items)
            {
                job.MarkedForDeletion = true;
            }

            // Import the jobs from the API
            List<IndustryJob> newJobs = new List<IndustryJob>();
            foreach (var srcJob in src)
            {
                // Skip long ended jobs
                var limit = srcJob.EndProductionTime.AddDays(IndustryJob.MaxEndedDays);
                if (limit < DateTime.UtcNow)
                    continue;

                // First check whether it is an existing job
                // If it is, update it and remove the deletion candidate flag
                if (m_items.Any(x => x.TryImport(srcJob)))
                    continue;

                var job = new IndustryJob(srcJob);
                if (job.InstalledItem != null)
                    newJobs.Add(job);
            }

            // Add the items that are no longer marked for deletion
            newJobs.AddRange(m_items.Where(x => !x.MarkedForDeletion));

            // Replace the old list with the new one
            m_items.Clear();
            m_items.AddRange(newJobs);
        }

        /// <summary>
        /// Exports the orders to a serialization object for the settings file.
        /// </summary>
        /// <returns>List of serializable jobs.</returns>
        internal List<SerializableJob> Export()
        {
            List<SerializableJob> serial = new List<SerializableJob>(m_items.Count);

            foreach (var job in m_items)
            {
                serial.Add(job.Export());
            }

            return serial;
        }

        /// <summary>
        /// Notify the user on a job completion.
        /// </summary>
        internal void UpdateOnTimerTick()
        {
            // We exit if there are no jobs
            if (m_items.IsEmpty())
                return;

            List<IndustryJob> jobsCompleted = new List<IndustryJob>();

            // Add the not notified "Ready" jobs to the completed list
            foreach (var job in m_items.Where(x => x.ActiveJobState == ActiveJobState.Ready && !x.NotificationSend))
            {
                jobsCompleted.Add(job);
                job.NotificationSend = true;
            }

            // We exit if no jobs have been completed
            if (jobsCompleted.IsEmpty())
                return;

            // Sends a notification
            EveClient.Notifications.NotifyIndustryJobCompletion(m_character, jobsCompleted);

            // Fires the event regarding industry jobs completed
            EveClient.OnCharacterIndustryJobsCompleted(m_character, jobsCompleted);
        }
    }
}
