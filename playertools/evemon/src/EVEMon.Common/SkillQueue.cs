using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using EVEMon.Common.Serialization;
using EVEMon.Common.Attributes;
using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a character's skills queue
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class SkillQueue : ReadonlyCollection<QueuedSkill>
    {
        public CCPCharacter m_character;
        private QueuedSkill m_lastCompleted;
        private DateTime startTime = DateTime.UtcNow;
        private bool m_isPaused;

        /// <summary>
        /// Default constructor, only used by <see cref="Character"/>
        /// </summary>
        /// <param name="character">The character this collection is bound to.</param>
        public SkillQueue(CCPCharacter character)
        {
            m_character = character;
        }

        /// <summary>
        /// Gets true when the character is currently training (non-empty and non-paused skill queue), false otherwise
        /// </summary>
        public bool IsTraining
        {
            get
            {
                if (m_isPaused)
                    return false;

                return m_items.Count != 0;
            }
        }

        /// <summary>
        /// Gets the last completed skill
        /// </summary>
        public QueuedSkill LastCompleted
        {
            get { return m_lastCompleted; }
        }

        /// <summary>
        /// Gets the training end time (UTC).
        /// </summary>
        public DateTime EndTime
        {
            get
            {
                if (m_items.IsEmpty())
                    return DateTime.UtcNow;

                return m_items[m_items.Count - 1].EndTime;
            }
        }

        /// <summary>
        /// Gets the skill currently in training
        /// </summary>
        public QueuedSkill CurrentlyTraining
        {
            get 
            {
                if (m_items.Count == 0)
                    return null;
                
                return m_items[0];
            }
        }

        /// <summary>
        /// Gets true whether the skill queue is currently paused.
        /// </summary>
        public bool IsPaused
        {
            get { return m_isPaused; }
        }

        /// <summary>
        /// When the timer ticks, on every second, we update the skill
        /// </summary>
        internal void UpdateOnTimerTick()
        {
            if (m_isPaused)
                return;

            List<QueuedSkill> skillsCompleted = new List<QueuedSkill>();

            // Pops all the completed skills
            while (m_items.Count != 0)
            {
                QueuedSkill skill = m_items[0];

                // If the skill is not completed, we jump out of the loop
                if (skill.EndTime > DateTime.UtcNow)
                    break;

                // The skill has been completed
                if (skill.Skill != null)
                    skill.Skill.MarkAsCompleted();

                skillsCompleted.Add(skill);
                m_lastCompleted = skill;
                m_items.RemoveAt(0);

                // Sends an email alert
                if (!Settings.IsRestoringSettings && Settings.Notifications.SendMailAlert)
                    Emailer.SendSkillCompletionMail(m_items, skill, m_character);

                // Sends a notification
                EveClient.Notifications.NotifySkillCompletion(m_character, skillsCompleted);
            }

            // At least one skill completed ?
            if (skillsCompleted.Count != 0)
                EveClient.OnCharacterQueuedSkillsCompleted(m_character, skillsCompleted);
        }

        /// <summary>
        /// Generates a deserialization object
        /// </summary>
        /// <returns></returns>
        internal List<SerializableQueuedSkill> Export()
        {
            var serial = new List<SerializableQueuedSkill>();
            foreach (var skill in m_items)
            {
                serial.Add(skill.Export());
            }
            return serial;
        }

        /// <summary>
        /// Imports data from a serialization object
        /// </summary>
        /// <param name="serial"></param>
        internal void Import(IEnumerable<SerializableQueuedSkill> serial)
        {
            m_isPaused = false;

            // If the queue is paused, CCP sends empty start and end time.
            // So we base the start time on when the skill queue was started.
            var startTimeWhenPaused = startTime;

            // Imports the queued skills and checks whether they are paused
            m_items.Clear();
            foreach (var serialSkill in serial)
            {
                // When the skill queue is paused, startTime and endTime are empty in the XML document.
                // As a result, the serialization leaves the DateTime with its default value.
                if (serialSkill.EndTime == default(DateTime))
                    m_isPaused = true;

                // Creates the skill queue
                m_items.Add(new QueuedSkill(m_character, serialSkill, m_isPaused, ref startTimeWhenPaused));
            }

            // Fires the event regarding the character skill queue update.
            EveClient.OnCharacterSkillQueueChanged(m_character);
        }
    }
}