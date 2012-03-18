using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a skill and level tuple
    /// </summary>
    public struct StaticSkillLevel : ISkillLevel
    {

        #region Constructors

        /// <summary>
        /// Constructor from the skill id.
        /// </summary>
        /// <param name="id"></param>
        /// <param name="level"></param>
        public StaticSkillLevel(int id, int level)
            : this()
        {
            this.Skill = StaticSkills.GetSkillById(id);
            this.Level = level;
        }

        /// <summary>
        /// Constructor from the skill name.
        /// </summary>
        /// <param name="skillName"></param>
        /// <param name="level"></param>
        public StaticSkillLevel(string name, int level)
            : this()
        {
            this.Skill = StaticSkills.GetSkillByName(name);
            this.Level = level;
        }

        /// <summary>
        /// Constructor from the static skill object.
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        public StaticSkillLevel(StaticSkill skill, int level)
            : this()
        {
            this.Skill = skill;
            this.Level = level;
        }

        /// <summary>
        /// Constructor from an <see cref="ISkillLevel"/> object.
        /// </summary>
        /// <param name="level"></param>
        public StaticSkillLevel(ISkillLevel level)
            : this()
        {
            this.Skill = level.Skill;
            this.Level = level.Level;
        }

        /// <summary>
        /// Constructor from the skill id with activity.
        /// </summary>
        /// <param name="id"></param>
        /// <param name="level"></param>
        /// <param name="activityId"></param>
        public StaticSkillLevel(int id, int level, int activityId)
            : this()
        {
            this.Skill = StaticSkills.GetSkillById(id);
            this.Level = level;
            this.Activity = (BlueprintActivity)Enum.ToObject(typeof(BlueprintActivity), activityId);
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets or sets the skill.
        /// </summary>
        public StaticSkill Skill
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the skill level.
        /// </summary>
        public int Level
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the activity for the skill.
        /// </summary>
        public BlueprintActivity Activity
        {
            get;
            set;
        }

        #endregion


        #region Public Methods

        /// <summary>
        /// Gets true if this skill level is, in any way, dependent of the provided skill level. Checks prerequisites but also same skill's lower levels.
        /// </summary>
        /// <param name="level"><see cref="ISkillLevel">ISkillLevel</see> to check if current skill is a dependant of the SkillLevel pass</param>
        /// <returns>True if the given item's skill is a prerequisite of this one or if it is a lower level of the same skill.</returns>
        public bool IsDependentOf(ISkillLevel skillLevel)
        {
            // Same skill, lower level ?
            if (this.Skill == skillLevel.Skill)
            {
                return this.Level > skillLevel.Level;
            }

            // Prerequisite
            int neededLevel;
            this.Skill.HasAsPrerequisite(skillLevel.Skill, out neededLevel);
            return skillLevel.Level <= neededLevel;
        }

        /// <summary>
        /// Gets all the dependencies, in a way matching the hirarchical order and without redudancies.
        /// I.e, for eidetic memory II, it will return <c>{ instant recall I, instant recall II, instant recall III, instant recall IV,  eidetic memory I, eidetic memory II }</c>.
        /// </summary>
        public IEnumerable<StaticSkillLevel> AllDependencies
        {
            get
            {
                var set = new SkillLevelSet<StaticSkillLevel>();
                var list = new List<StaticSkillLevel>();

                // Fill the set and list
                StaticSkillLevelEnumerableExtensions.FillDependencies(set, list, this, false);

                // Return the results
                foreach (var item in list)
                {
                    yield return item;
                }
            }
        }

        #endregion


        #region Public Operators

        /// <summary>
        /// Implicitly converts fromm a non-static training
        /// </summary>
        /// <param name="training"></param>
        /// <returns></returns>
        public static implicit operator StaticSkillLevel(SkillLevel training)
        {
            return new StaticSkillLevel(training.Skill.StaticData, training.Level);
        }

        #endregion


        #region Overridden Methods

        /// <summary>
        /// Gets a string representation of this prerequisite
        /// </summary>
        /// <returns>Skill Name and Level</returns>
        public override string ToString()
        {
            return String.Format(CultureConstants.DefaultCulture, "{0} {1}", Skill.Name, EVEMon.Common.Skill.GetRomanForInt(Level));
        }

        #endregion

    }
}
