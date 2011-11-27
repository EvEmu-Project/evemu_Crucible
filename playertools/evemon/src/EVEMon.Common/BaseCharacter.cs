using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    public abstract class BaseCharacter
    {

        #region Abstract methods and properties

        protected abstract int GetTotalSkillPoints();
        protected abstract ICharacterAttribute GetAttribute(EveAttribute attribute);

        public abstract int GetSkillLevel(StaticSkill skill);
        public abstract int GetSkillPoints(StaticSkill skill);

        #endregion


        #region Computation methods

        /// <summary>
        /// Gets the total skill points for this character
        /// </summary>
        public int SkillPoints
        {
            get { return GetTotalSkillPoints(); }
        }

        /// <summary>
        /// Computes the SP per hour for the given skill, without factoring out the newbies bonus.
        /// </summary>
        /// <param name="skill"></param>
        /// <returns></returns>
        public float GetBaseSPPerHour(StaticSkill skill)
        {
            if (skill.PrimaryAttribute == EveAttribute.None || skill.SecondaryAttribute == EveAttribute.None)
                return 0.0f;

            float primAttr = GetAttribute(skill.PrimaryAttribute).EffectiveValue;
            float secondaryAttr = GetAttribute(skill.SecondaryAttribute).EffectiveValue;
            return primAttr * 60.0f + secondaryAttr * 30.0f;
        }

        /// <summary>
        /// Gets a character scratchpad representing this character after the provided skill levels trainings.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="trainings"></param>
        /// <returns></returns>
        public CharacterScratchpad After<T>(IEnumerable<T> trainings)
            where T : ISkillLevel
        {
            var scratchpad = new CharacterScratchpad(this);
            scratchpad.Train(trainings);
            return scratchpad;
        }

        /// <summary>
        /// Gets a character scratchpad representing this character after a switch to the provided implant set.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="trainings"></param>
        /// <returns></returns>
        public CharacterScratchpad After(ImplantSet set)
        {
            var scratchpad = new CharacterScratchpad(this);
            for (int i = 0; i < 5; i++)
            {
                var attribute = (EveAttribute)i;
                scratchpad[attribute].ImplantBonus = set[attribute].Bonus;
            }
            return scratchpad;

        }

        /// <summary>
        /// Gets the time span for a specific number of skill points.
        /// </summary>
        /// <param name="points">The points to calculate points.</param>
        /// <param name="skill">The skill to train.</param>
        /// <returns></returns>
        public TimeSpan GetTimeSpanForPoints(StaticSkill skill, int points)
        {
            return GetTrainingTime(points, GetBaseSPPerHour(skill));
        }

        #endregion


        #region GetSPToTrain

        /// <summary>
        /// Computes the number of SP to train
        /// </summary>
        /// <param name="level"></param>
        /// <returns></returns>
        public int GetSPToTrain(ISkillLevel level)
        {
            return GetSPToTrain(level.Skill, level.Level, TrainingOrigin.FromCurrent);
        }

        /// <summary>
        /// Computes the number of SP to train
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        public int GetSPToTrain(StaticSkill skill, int level)
        {
            return GetSPToTrain(skill, level, TrainingOrigin.FromCurrent);
        }

        /// <summary>
        /// Computes the number of SP to train
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        /// <param name="origin"></param>
        /// <returns></returns>
        public int GetSPToTrain(StaticSkill skill, int level, TrainingOrigin origin)
        {
            if (level == 0)
                return 0;
            int sp = skill.GetPointsRequiredForLevel(level);

            // Deals with the origin
            int result;
            switch (origin)
            {
                // Include current SP
                case TrainingOrigin.FromCurrent:
                    result = sp - GetSkillPoints(skill);
                    break;

                // This level only (previous are known)
                case TrainingOrigin.FromPreviousLevel:
                    result = sp - skill.GetPointsRequiredForLevel(level - 1);
                    break;

                case TrainingOrigin.FromPreviousLevelOrCurrent:
                    result = sp - Math.Max(GetSkillPoints(skill), skill.GetPointsRequiredForLevel(level - 1));
                    break;

                // Include nothing
                default:
                    result = sp;
                    break;
            }

            // Returns result
            if (result < 0)
                return 0;

            return result;
        }

        #endregion


        #region GetTrainingTime & GetTrainingTimeToMultipleSkills

        /// <summary>
        /// Computes the training time for the given skill
        /// </summary>
        /// <param name="level"></param>
        /// <returns></returns>
        public TimeSpan GetTrainingTime(ISkillLevel level)
        {
            return GetTrainingTime(level.Skill, level.Level, TrainingOrigin.FromCurrent);
        }

        /// <summary>
        /// Computes the training time for the given skill
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        public TimeSpan GetTrainingTime(StaticSkill skill, int level)
        {
            return GetTrainingTime(skill, level, TrainingOrigin.FromCurrent);
        }

        /// <summary>
        /// Computes the training time for the given skill
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        /// <param name="origin"></param>
        /// <returns></returns>
        public TimeSpan GetTrainingTime(StaticSkill skill, int level, TrainingOrigin origin)
        {
            var spPerHour = GetBaseSPPerHour(skill);
            int sp = GetSPToTrain(skill, level, origin);
            return GetTrainingTime(sp, spPerHour);
        }

        /// <summary>
        /// Gets the time to train the given SP at the provided speed.
        /// </summary>
        /// <param name="sp"></param>
        /// <param name="spPerHour"></param>
        /// <returns></returns>
        public TimeSpan GetTrainingTime(int sp, float spPerHour)
        {
            if (spPerHour == 0.0f)
                return TimeSpan.FromDays(999.0);
            return TimeSpan.FromHours(sp / spPerHour);
        }

        /// <summary>
        /// Gets the time require to train the given skills and their prerequisites
        /// </summary>
        /// <param name="trainings">A sequence of pairs of skills and the target levels.</param>
        /// <returns></returns>
        public TimeSpan GetTrainingTimeToMultipleSkills<T>(IEnumerable<T> trainings)
            where T : ISkillLevel
        {
            CharacterScratchpad scratchpad = this.After(trainings);
            return scratchpad.TrainingTime;
        }

        #endregion


        #region ICharacter non-abstract explicit members

        public ICharacterAttribute this[EveAttribute attribute]
        {
            get { return GetAttribute(attribute); }
        }

        public ICharacterAttribute Intelligence
        {
            get { return GetAttribute(EveAttribute.Intelligence); }
        }

        public ICharacterAttribute Perception
        {
            get { return GetAttribute(EveAttribute.Perception); }
        }

        public ICharacterAttribute Willpower
        {
            get { return GetAttribute(EveAttribute.Willpower); }
        }

        public ICharacterAttribute Charisma
        {
            get { return GetAttribute(EveAttribute.Charisma); }
        }

        public ICharacterAttribute Memory
        {
            get { return GetAttribute(EveAttribute.Memory); }
        }

        #endregion
    
    }
}
