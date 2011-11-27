using System;
using EVEMon.Common.Attributes;
using EVEMon.Common.Data;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a skill training
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class QueuedSkill
    {
        private readonly Character m_owner;
        private int m_level;
        private Skill m_skill;
        private DateTime m_startTime;
        private DateTime m_endTime;
        private int m_startSP;
        private int m_endSP;

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="character">The character for this training</param>
        /// <param name="serial">The serialization object for this training</param>
        /// <param name="isPaused">When true, the training is currently paused.</param>
        /// <param name="startTimeWhenPaused">Training starttime when the queue is actually paused. Indeed, in such case, CCP returns empty start and end time, so we compute a "what if we start now" scenario.</param>
        internal QueuedSkill(Character character, SerializableQueuedSkill serial, bool isPaused, ref DateTime startTimeWhenPaused)
        {
            m_owner = character;
            m_startSP = serial.StartSP;
            m_endSP = serial.EndSP;
            m_level = serial.Level;
            m_skill = character.Skills[serial.ID];

            if (!isPaused)
            {
                // Not paused, we should trust CCP
                m_startTime = serial.StartTime;
                m_endTime = serial.EndTime;
            }
            else
            {
                // StartTime and EndTime were empty on the serialization object if the skill was paused
                // So we compute a "what if we start now" scenario
                m_startTime = startTimeWhenPaused;
                if (m_skill != null)
                    startTimeWhenPaused += m_skill.GetLeftTrainingTimeForLevelOnly(m_level);
                m_endTime = startTimeWhenPaused;
            }
        }

        /// <summary>
        /// Gets the character training this.
        /// </summary>
        public Character Owner
        {
            get { return m_owner; }
        }

        /// <summary>
        /// Gets the trained level
        /// </summary>
        public int Level
        {
            get { return m_level; }
        }

        /// <summary>
        /// Gets the trained skill. May be null if the skill is not in our datafiles.
        /// </summary>
        public Skill Skill
        {
            get { return m_skill; }
        }

        /// <summary>
        /// Gets the skill name, or "Unknown skill" if the skill was not in our datafiles.
        /// </summary>
        public string SkillName
        {
            get { return (m_skill != null ? m_skill.Name : "Unknown Skill"); }
        }

        /// <summary>
        /// Gets the training start time (UTC)
        /// </summary>
        public DateTime StartTime
        {
            get { return m_startTime; }
        }

        /// <summary>
        /// Gets the time this training will be completed (UTC)
        /// </summary>
        public DateTime EndTime
        {
            get { return m_endTime; }
        }

        /// <summary>
        /// Gets the number of SP this skill had when the training started
        /// </summary>
        public int StartSP
        {
            get { return m_startSP; }
        }

        /// <summary>
        /// Gets the number of SP this skill will have once the training is over
        /// </summary>
        public int EndSP
        {
            get { return m_endSP; }
        }

        /// <summary>
        /// Gets the fraction completed, between 0 and 1
        /// </summary>
        public float FractionCompleted
        {
            get 
            {
                return (m_skill == null ? 0 : m_skill.FractionCompleted);
            }
        }

        /// <summary>
        /// Computes an estimation of the current SP 
        /// </summary>
        public int CurrentSP
        {
            get
            {
                // Computes the total SP after this training
                int totalSP = 0;
                foreach (var skill in StaticSkills.AllSkills)
                {
                    if (m_skill.StaticData == skill && m_skill.IsTraining)
                    {
                        totalSP += m_endSP;
                    }
                    else
                    {
                        totalSP += m_owner.GetSkillPoints(skill);
                    }
                }

                // Computes estimated current SP
                var spPerHour = m_owner.GetBaseSPPerHour(m_skill);
                var estimatedSP = m_endSP - (m_endTime - DateTime.UtcNow).TotalHours * spPerHour;
                return (m_skill.IsTraining ? Math.Max((int)estimatedSP, m_startSP) : m_startSP);
            }
        }

        /// <summary>
        /// Computes the remaining time. Returns <see cref="TimeSpan.Zero"/> if already completed;
        /// </summary>
        public TimeSpan RemainingTime
        {
            get
            {
                TimeSpan left = m_endTime.Subtract(DateTime.UtcNow);
                if (left < TimeSpan.Zero)
                    return TimeSpan.Zero;
                return left;
            }
        }

        /// <summary>
        /// Gets true if the training has been completed, false otherwise.
        /// </summary>
        public bool IsCompleted
        {
            get
            {
                if (m_endTime <= DateTime.UtcNow)
                    return true;
                return false;
            }
        }

        /// <summary>
        /// Generates a deserialization object.
        /// </summary>
        /// <returns></returns>
        internal SerializableQueuedSkill Export()
        {
            var skill = new SerializableQueuedSkill
            {
                ID = (m_skill == null ? 0 : m_skill.ID),
                Level = m_level,
                StartSP = m_startSP,
                EndSP = m_endSP,
            };

            // CCP's API indicates paused training with missing skill
            // start and end times. Mimicing them is ugly but necessary
            if (m_owner.IsTraining)
            {
                skill.StartTime = m_startTime;
                skill.EndTime = m_endTime;
            }

            return skill;
        }

        /// <summary>
        /// Gets a string representation of this skill
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return String.Format("{0} {1}", SkillName, Skill.GetRomanForInt(m_level));
        }
    }
}
