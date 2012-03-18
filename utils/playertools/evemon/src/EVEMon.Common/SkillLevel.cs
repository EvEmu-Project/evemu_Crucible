using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a skill and level tuple
    /// </summary>
    public struct SkillLevel : ISkillLevel
    {
        /// <summary>
        /// Constructor from the skill object
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        public SkillLevel(Skill skill, int level)
            : this()
        {
            this.Skill = skill;
            this.Level = level;
        }

        /// <summary>
        /// Gets the static skill
        /// </summary>
        StaticSkill ISkillLevel.Skill
        {
            get { return Skill; }
        }

        /// <summary>
        /// Gets or sets the skill
        /// </summary>
        public Skill Skill
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the skill level
        /// </summary>
        public int Level
        {
            get;
            set;
        }

        /// <summary>
        /// Gets true if this skill level is already known
        /// </summary>
        public bool IsKnown
        {
            get { return Skill.Level >= Level; }
        }

        /// <summary>
        /// Gets true if this skill level is, in any way, dependent of the provided skill level. Checks prerequisites but also same skill's lower levels.
        /// </summary>
        /// <param name="level"></param>
        /// <returns>True if the given item's skill is a prerequisite of this one or if it is a lower level of the same skill.</returns>
        public bool IsDependentOf(ISkillLevel level)
        {
            return (new StaticSkillLevel(this)).IsDependentOf(level);
        }

        /// <summary>
        /// Gets all the dependencies, in a way matching the hirarchical order and without redudancies.
        /// I.e, for eidetic memory II, it will return <c>{ instant recall I, instant recall II, instant recall III, instant recall IV,  eidetic memory I, eidetic memory II }</c>.
        /// </summary>
        public IEnumerable<SkillLevel> AllDependencies
        {
            get
            {
                return new StaticSkillLevel(this).AllDependencies.ToCharacter(this.Skill.Character);
            }
        }

        /// <summary>
        /// Gets a hash code for this object.
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode()
        {
            // 3 bits for level (0 - 5) and the rest are for the skill name
            return (Skill.Name.GetHashCode() << 3) | Level;
        }

        /// <summary>
        /// Implicitly converts from a non-static training
        /// </summary>
        /// <param name="training"></param>
        /// <returns></returns>
        public static implicit operator StaticSkillLevel(SkillLevel training)
        {
            return new StaticSkillLevel(training.Skill.StaticData, training.Level);
        }

        /// <summary>
        /// Gets a string representation of this prerequisite
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return Skill.Name + " " + EVEMon.Common.Skill.GetRomanForInt(Level);
        }
    }
}
