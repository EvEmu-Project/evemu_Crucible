using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a skill definition. Characters use their own representation, through <see cref="Skill"/>.
    /// </summary>
    public sealed class StaticSkill : IStaticSkill
    {
        private readonly long m_id;
        private readonly int m_rank;
        private readonly long m_cost;
        private readonly bool m_public;
        private readonly string m_description;
        private readonly string m_name;
        private readonly int m_arrayIndex;

        private readonly StaticSkillGroup m_group;
        private readonly EveAttribute m_primaryAttribute;
        private readonly EveAttribute m_secondaryAttribute;
        private readonly List<StaticSkillLevel> m_prereqs = new List<StaticSkillLevel>();

        private bool m_trainableOnTrialAccount;
        private string m_descriptionNL;

        #region Constructors

        /// <summary>
        /// Deserialization constructor from datafiles.
        /// </summary>
        /// <param name="group"></param>
        /// <param name="src"></param>
        /// <param name="arrayIndex"></param>
        internal StaticSkill(StaticSkillGroup group, SerializableSkill src, int arrayIndex)
        {
            m_id = src.ID;
            m_cost = src.Cost;
            m_rank = src.Rank;
            m_public = src.Public;
            m_name = src.Name;
            m_description = src.Description;
            m_descriptionNL = null;
            m_primaryAttribute = src.PrimaryAttribute;
            m_secondaryAttribute = src.SecondaryAttribute;
            m_trainableOnTrialAccount = src.CanTrainOnTrial;
            m_arrayIndex = arrayIndex;
            m_group = group;
        }

        #endregion


        #region Initializer

        /// <summary>
        /// Completes the initialization by updating the prequisites and checking trainability on trial account
        /// </summary>
        internal void CompleteInitialization(SerializableSkillPrerequisite[] prereqs)
        {
            if (prereqs == null)
                return;

            // Create the prerequisites list
            m_prereqs.AddRange(prereqs.Select(x => new StaticSkillLevel(x.GetSkill(), x.Level)));

            // Check trainableOnTrialAccount on its childrens to be sure it's really trainable
            if (m_trainableOnTrialAccount)
            {
                foreach (var prereq in m_prereqs)
                {
                    if (!prereq.Skill.m_trainableOnTrialAccount)
                    {
                        m_trainableOnTrialAccount = false;
                        return;
                    }
                }
            }
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets the ID of this skill
        /// </summary>
        public long ID
        {
            get { return m_id; }
        }

        /// <summary>
        /// Gets a zero-based index for skills (allow the use of arrays to optimize computations)
        /// </summary>
        public int ArrayIndex
        {
            get { return m_arrayIndex; }
        }

        /// <summary>
        /// Gets the name of this skill (interned)
        /// </summary>
        public string Name
        {
            get { return m_name; }
        }

        /// <summary>
        /// Gets the description of this skill
        /// </summary>
        public string Description
        {
            get { return m_description; }
        }

        /// <summary>
        /// Gets the description of this skill with a special formatting (what's the point ?).
        /// </summary>
        public string DescriptionNL
        {
            get
            {
                if (m_descriptionNL == null)
                    m_descriptionNL = WordWrap(m_description, 100);

                return m_descriptionNL;
            }
        }

        /// <summary>
        /// Gets the rank of this skill.
        /// </summary>
        public int Rank
        {
            get { return m_rank; }
        }

        /// <summary>
        /// Gets the skill's cost
        /// </summary>
        public long Cost
        {
            get { return m_cost; }
        }

        /// <summary>
        /// Gets the skill group this skill is part of.
        /// </summary>
        public StaticSkillGroup Group
        {
            get { return m_group; }
        }

        /// <summary>
        /// Gets false when the skill is not for sale by any NPC (CCP never published it or removed it from the game, it's inactive)
        /// </summary>
        public bool IsPublic
        {
            get { return m_public; }
        }

        /// <summary>
        /// Gets the primary attribute of this skill.
        /// </summary>
        public EveAttribute PrimaryAttribute
        {
            get { return m_primaryAttribute; }
        }

        /// <summary>
        /// Gets the secondary attribute of this skill.
        /// </summary>
        public EveAttribute SecondaryAttribute
        {
            get { return m_secondaryAttribute; }
        }

        /// <summary>
        /// Get whether skill is trainable on a trial account
        /// </summary>
        public bool IsTrainableOnTrialAccount
        {
            get { return m_trainableOnTrialAccount; }
        }

        /// <summary>
        /// Gets all the prerequisites. I.e, for eidetic memory, it will return <c>{ instant recall IV }</c>. 
        /// The order matches the hirerarchy but skills are not duplicated and are systematically trained to the highest required level.
        /// For example, if some skill is required to lv3 and, later, to lv4, this first time it is encountered, lv4 is returned.
        /// </summary>
        /// <remarks>Please note they may be redundancies.</remarks>
        public IEnumerable<StaticSkillLevel> AllPrerequisites
        {
            get
            {
                int[] highestLevels = new int[StaticSkills.ArrayIndicesCount];
                List<StaticSkillLevel> list = new List<StaticSkillLevel>();

                // Fill the array
                foreach (var prereq in this.Prerequisites)
                {
                    StaticSkillEnumerableExtensions.FillPrerequisites(highestLevels, list, prereq, true);
                }

                // Return the result
                foreach (var newItem in list)
                {
                    if (highestLevels[newItem.Skill.ArrayIndex] != 0)
                    {
                        yield return new StaticSkillLevel(newItem.Skill, highestLevels[newItem.Skill.ArrayIndex]);
                        highestLevels[newItem.Skill.ArrayIndex] = 0;
                    }
                }
            }
        }

        /// <summary>
        /// Gets the prerequisites a character must satisfy before it can be trained
        /// </summary>
        public IEnumerable<StaticSkillLevel> Prerequisites
        {
            get { return m_prereqs; }
        }

        /// <summary>
        /// Gets a formatted representation of the price
        /// </summary>
        public string FormattedCost
        {
            get
            {
                if (m_cost == 0)
                    return "0";

                return String.Format(CultureConstants.DefaultCulture, "{0:0,0,0}", m_cost);
            }
        }

        #endregion


        #region Public Methods - Computations

        /// <summary>
        /// Calculates the cumulative points required for a level of this skill (starting from a zero level).
        /// </summary>
        /// <param name="level">The level.</param>
        /// <returns>The required nr. of points.</returns>
        public int GetPointsRequiredForLevel(int level)
        {
            // Much faster than the old formula. This one too may have 1pt difference here and there, only on the lv2 skills.
            switch (level)
            {
                case -1:
                case 0:
                    return 0;
                case 1:
                    return 250 * m_rank;
                case 2:
                    switch (m_rank)
                    {
                        case 1: 
                            return 1415;
                        default:
                            return (int)(m_rank * 1414.3f + 0.5f);
                    }
                case 3:
                    return 8000 * m_rank;
                case 4:
                    return (int)(Convert.ToInt32(Math.Ceiling(Math.Pow(2, (2.5 * level) - 2.5) * 250 * m_rank)));
                case 5:
                    return 256000 * m_rank;
                default:
                    throw new NotImplementedException(String.Format("One of our devs messed up. Skill level was {0} ?!", level));
            }
        }

        /// <summary>
        /// Calculates the cumulative points required for a level of this skill (starting from a zero level).
        /// </summary>
        /// <param name="level">The level.</param>
        /// <returns>The required nr. of points.</returns>
        public int GetPointsRequiredForLevelOnly(int level)
        {
            if (level == 0)
                return 0;

            return GetPointsRequiredForLevel(level) - GetPointsRequiredForLevelOnly(level - 1);
        }

        #endregion


        #region Private Methods

        /// <summary>
        /// Remove line feeds and some other characters to format the string. Honestly, I don't understand the point.
        /// </summary>
        /// <param name="text"></param>
        /// <param name="maxLength"></param>
        /// <returns></returns>
        private string WordWrap(string text, int maxLength)
        {
            text = text.Replace("\n", " ");
            text = text.Replace("\r", " ");
            text = text.Replace(".", ". ");
            text = text.Replace(">", "> ");
            text = text.Replace("\t", " ");
            text = text.Replace(",", ", ");
            text = text.Replace(";", "; ");

            string[] words = text.Split(' ');
            List<string> lines = new List<string>(text.Length / maxLength);
            int currentLineLength = 0;
            string currentLine = String.Empty;
            bool InTag = false;

            foreach (string currentWord in words)
            {
                //Ignore html
                if (currentWord.Length > 0)
                {
                    if (currentWord.Substring(0, 1) == "<")
                        InTag = true;

                    if (InTag)
                    {
                        //Handle filenames inside html tags
                        if (currentLine.EndsWith("."))
                        {
                            currentLine += currentWord;
                        }
                        else
                        {
                            currentLine += " " + currentWord;
                        }

                        if (currentWord.IndexOf(">") > -1)
                            InTag = false;
                    }
                    else
                    {
                        if (currentLineLength + currentWord.Length + 1 < maxLength)
                        {
                            currentLine += " " + currentWord;
                            currentLineLength += (currentWord.Length + 1);
                        }
                        else
                        {
                            lines.Add(currentLine);
                            currentLine = currentWord;
                            currentLineLength = currentWord.Length;
                        }
                    }
                }
            }

            if (currentLine != String.Empty)
                lines.Add(currentLine);

            string[] textLinesStr = new string[lines.Count];
            lines.CopyTo(textLinesStr, 0);

            string strWrapped = String.Empty;
            foreach (string line in textLinesStr)
            {
                strWrapped += line + "\n";
            }

            return strWrapped;
        }

        #endregion


        #region Public Methods

        /// <summary>
        /// Gets this skill's representation for the provided character
        /// </summary>
        /// <param name="character"></param>
        /// <returns></returns>
        public Skill ToCharacter(Character character)
        {
            return character.Skills.GetByArrayIndex(m_arrayIndex);
        }

        #endregion


        #region Overridden Methods

        /// <summary>
        /// Gets a string representation for this skill (the name of the skill).
        /// </summary>
        /// <returns>Name of the Static Skill.</returns>
        public override string ToString()
        {
            return m_name;
        }

        #endregion

    }
}
