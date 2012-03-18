using System;
using System.Linq;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Attributes;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a plan's entry.
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class PlanEntry : ISkillLevel
    {
        public const int DefaultPriority = 3;

        private readonly List<string> m_planGroups = new List<string>();
        private readonly StaticSkill m_skill;
        private readonly BasePlan m_owner;
        private readonly int m_level;

        private RemappingPoint m_remapping;
        private PlanEntryType m_entryType;
        private int m_priority;
        private string m_notes;

        // Statistics computed on Plan.UpdateTrainingTimes
        private TimeSpan m_naturalTrainingTime;
        private TimeSpan m_oldTrainingTime;
        private TimeSpan m_trainingTime;
        private DateTime m_startTime;
        private DateTime m_endTime;
        private int m_spPerHour;
        private int m_totalSP;
        private  int m_skillPointsRequired;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="owner"></param>
        /// <param name="skill"></param>
        public PlanEntry(BasePlan owner, StaticSkill skill, int level)
        {
            m_owner = owner;
            m_skill = skill;
            m_level = level;

            m_priority = DefaultPriority;
            m_notes = String.Empty;

            m_oldTrainingTime = TimeSpan.Zero;
            m_trainingTime = TimeSpan.Zero;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="owner"></param>
        /// <param name="skill"></param>
        public PlanEntry(StaticSkill skill, int level)
            : this(null, skill, level)
        {
        }

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="character"></param>
        /// <param name="serial"></param>
        internal PlanEntry(BasePlan owner, SerializablePlanEntry serial)
        {
            m_owner = owner;
            m_entryType = serial.Type;
            m_skill = GetSkill(serial);
            m_level = serial.Level;
            m_notes = serial.Notes;
            m_priority = serial.Priority;

            serial.PlanGroups.ForEach(x => m_planGroups.Add(x));

            if (serial.Remapping != null)
                m_remapping = new RemappingPoint(serial.Remapping);
        }

        /// <summary>
        /// Gets the character this entry is bound to.
        /// </summary>
        public BaseCharacter Character
        {
            get { return m_owner.Character; }
        }

        /// <summary>
        /// Gets the owner
        /// </summary>
        public BasePlan Plan
        {
            get { return m_owner; }
        }

        /// <summary>
        /// Gets the skill of this entry
        /// </summary>
        public StaticSkill Skill
        {
            get { return m_skill; }
        }

        /// <summary>
        /// Gets the character's skill of this entry
        /// </summary>
        public Skill CharacterSkill
        {
            get
            {
                var character = m_owner.Character as Character;
                if (character == null)
                    return null;

                return m_skill.ToCharacter(character);
            }
        }

        /// <summary>
        /// Gets the skill level of this plan entry.
        /// </summary>
        public int Level
        {
            get { return m_level; }
        }

        /// <summary>
        /// Gets the entry's priority
        /// </summary>
        public int Priority
        {
            get { return m_priority; }
            internal set { m_priority = value; }
        }

        /// <summary>
        /// Gets the entry type.
        /// </summary>
        public PlanEntryType Type
        {
            get { return m_entryType; }
            internal set { m_entryType = value; }
        }

        /// <summary>
        /// Gets or sets the notes
        /// </summary>
        public string Notes
        {
            get { return m_notes; }
            set
            {
                m_notes = value;
                if (m_owner != null)
                    m_owner.OnChanged(BasePlan.PlanChange.Notification);
            }
        }

        /// <summary>
        /// Gets or sets the remapping point to apply before that skill is trained
        /// </summary>
        public RemappingPoint Remapping
        {
            get { return m_remapping; }
            set
            {
                m_remapping = value;
                if (m_owner != null)
                    m_owner.OnChanged(BasePlan.PlanChange.Notification);
            }
        }

        /// <summary>
        /// Gets the names of the plans this entry was taken from when those plans were merged.
        /// </summary>
        public List<string> PlanGroups
        {
            get { return m_planGroups; }
        }

        /// <summary>
        /// Gets a description of the plans groups ("none", "multiple (2)" or the plan's name)
        /// </summary>
        public string PlanGroupsDescription
        {
            get
            {
                if (m_planGroups == null || m_planGroups.Count == 0)
                {
                    return "None";
                }
                else if (m_planGroups.Count == 1)
                {
                    return (string)m_planGroups[0];
                }
                else
                {
                    return String.Format(CultureConstants.DefaultCulture, "Multiple ({0})", m_planGroups.Count);
                }
            }
        }

        /// <summary>
        /// Gets true if the character already know all the prerequisites
        /// </summary>
        public bool CanTrainNow
        {
            get
            {
                var character = Character;

                // Checks all the prerequisites are trained
                bool Prereqmet = m_skill.Prerequisites.All(x => character.GetSkillLevel(x.Skill) >= x.Level);

                // Checks the skill has the previous level
                if (Prereqmet && m_level != 0 && character.GetSkillLevel(m_skill) >= m_level - 1)
                    return true;

                return false;
            }
        }

        /// <summary>
        /// Gets a skill by its ID or its name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static StaticSkill GetSkill(SerializablePlanEntry serial)
        {
            // Try get skill by its ID
            StaticSkill skill = StaticSkills.GetSkillById(serial.ID);

            // We failed? Try get skill by its name
            if (skill == null)
                skill = StaticSkills.GetSkillByName(serial.SkillName);

            return skill;
        }

        /// <summary>
        /// Gets true if this skill level is, in any way, dependent of the provided skill level. Checks prerequisites but also same skill's lower levels.
        /// </summary>
        /// <param name="level"></param>
        /// <returns>True if the given item's skill is a prerequisite of this one or if it is a lower level of the same skill.</returns>
        public bool IsDependentOf(ISkillLevel level)
        {
            return ((StaticSkillLevel)this).IsDependentOf(level);
        }

        #region Computations done when UpdateTrainingTime is called

        /// <summary>
        /// Gets the training time computed the last time the <see cref="Plan.UpdateStatistics"/> was called.
        /// </summary>
        public TimeSpan TrainingTime
        {
            get { return m_trainingTime; }
        }

        /// <summary>
        /// Gets the backup of the training time made just before <see cref="Plan.UpdateStatistics"/> was called.
        /// </summary>
        public TimeSpan OldTrainingTime
        {
            get { return m_oldTrainingTime; }
        }

        /// <summary>
        /// Gets the training time without implants, as computed the last time <see cref="Plan.UpdateStatistics"/> was called.
        /// </summary>
        public TimeSpan NaturalTrainingTime
        {
            get { return m_naturalTrainingTime; }
        }

        /// <summary>
        /// Gets the skill points total at the end of the training, as computed the last time <see cref="Plan.UpdateStatistics"/> was called.
        /// </summary>
        public int EstimatedTotalSkillPoints
        {
            get { return m_totalSP; }
        }

        /// <summary>
        /// Gets the SP/Hour, as computed the last time <see cref="Plan.UpdateStatistics"/> was called.
        /// </summary>
        public int SpPerHour
        {
            get { return m_spPerHour; }
        }

        /// <summary>
        /// Gets the training start time, as computed the last time <see cref="Plan.UpdateStatistics"/> was called.
        /// </summary>
        public DateTime StartTime
        {
            get { return m_startTime; }
        }

        /// <summary>
        /// Gets the training end time, as computed the last time <see cref="Plan.UpdateStatistics"/> was called.
        /// </summary>
        public DateTime EndTime
        {
            get { return m_endTime; }
        }

        /// <summary>
        /// Represents the progress towards completion
        /// </summary>
        public float FractionCompleted
        {
            get
            {
                if (m_level == (CharacterSkill.Level + 1))
                    return (float)CharacterSkill.FractionCompleted;

                //Not partially trained? Then it's 0.0
                return 0.0f;
            }
        }

        /// How many skill points are required to train this skill
        /// </summary>
        public int SkillPointsRequired
        {
            get { return m_skillPointsRequired; }
        }

        /// <summary>
        /// Updates the column statistics (with the exception of the <see cref="UpdateOldTrainingTime"/>) from the given scratchpad.
        /// </summary>
        /// <param name="character"></param>
        /// <param name="characterWithoutImplants"></param>
        /// <param name="time"></param>
        internal void UpdateStatistics(BaseCharacter character, BaseCharacter characterWithoutImplants, ref DateTime time)
        {
            m_skillPointsRequired = m_skill.GetPointsRequiredForLevel(m_level) - character.GetSkillPoints(m_skill);
            m_totalSP = character.SkillPoints + m_skillPointsRequired;
            m_trainingTime = character.GetTrainingTime(m_skill, m_level);
            m_naturalTrainingTime = characterWithoutImplants.GetTrainingTime(m_skill, m_level);
            m_spPerHour = (int)Math.Round(character.GetBaseSPPerHour(m_skill));
            m_endTime = time + m_trainingTime;
            m_startTime = time;
            time = m_endTime;
        }

        /// <summary>
        /// Updates the <see cref="OldTrainingTime"/> statistic.
        /// </summary>
        /// <param name="character"></param>
        internal void UpdateOldTrainingTime(BaseCharacter character)
        {
            m_oldTrainingTime = character.GetTrainingTime(m_skill, m_level);
        }
        #endregion

        /// <summary>
        /// Gets a hash code from the level and skill ID.
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode()
        {
            // after the switch to 64-bit integers this line was throwing a
            // warning. GetHashCode can't possibly be unique for every object
            // there is, additionally GetHashCode() should not be used for
            // equality only grouping; or at least Google says so...
            return (int)m_skill.ID << 3 | m_level;
        }

        /// <summary>
        /// Returns a string representation of entry - eases debugging.
        /// </summary>
        /// <returns>Hull Upgrades IV</returns>
        public override string ToString()
        {
            return String.Format(CultureConstants.DefaultCulture, "{0} {1}",
                m_skill.Name, EVEMon.Common.Skill.GetRomanForInt(m_level));
        }

        /// <summary>
        /// Creates a clone of this entry.
        /// </summary>
        /// <returns></returns>
        public PlanEntry Clone()
        {
            return Clone(m_owner);
        }

        /// <summary>
        /// Creates a clone of this entry for the given plan
        /// </summary>
        /// <param name="plan"></param>
        /// <returns></returns>
        internal PlanEntry Clone(BasePlan plan)
        {
            // We need a skill for the plan's character
            PlanEntry clone = new PlanEntry(plan, m_skill, m_level);
            clone.m_entryType = m_entryType;
            clone.m_priority = m_priority;
            clone.m_notes = m_notes;
            clone.m_planGroups.AddRange(m_planGroups);
            clone.m_remapping = (m_remapping != null ? m_remapping.Clone() : null);
            clone.m_oldTrainingTime = m_oldTrainingTime;
            clone.m_trainingTime = m_trainingTime;
            return clone;
        }

        /// <summary>
        /// Implicit conversion operator
        /// </summary>
        /// <param name="entry"></param>
        /// <returns></returns>
        public static implicit operator StaticSkillLevel(PlanEntry entry)
        {
            return new StaticSkillLevel(entry.Skill, entry.Level);
        }
    }
}
