using System;
using System.Collections.Generic;
using System.Linq;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    public static class SkillEnumerableExtensions
    {
        /// <summary>
        /// Rreturns an enumeration of static equivalent of those character skills.
        /// </summary>
        /// <param name="src"></param>
        /// <param name="character"></param>
        /// <returns></returns>
        public static IEnumerable<StaticSkill> ToStatic(this IEnumerable<Skill> src)
        {
            foreach (var item in src)
            {
                yield return item.StaticData;
            }
        }

        /// <summary>
        /// Gets all the prerequisites. I.e, for eidetic memory, it will return <c>{ instant recall IV }</c>. 
        /// The order matches the hirerarchy but skills are not duplicated and are systematically trained to the highest required level.
        /// For example, if some skill is required to lv3 and, later, to lv4, this first time it is encountered, lv4 is returned.
        /// </summary>
        /// <remarks>Please note they may be redundancies.</remarks>
        public static IEnumerable<SkillLevel> GetAllPrerequisites(this IEnumerable<Skill> src)
        {
            var first = src.FirstOrDefault();
            if (first == null) return EmptyEnumerable<SkillLevel>.Instance;
            return src.ToStatic().GetAllPrerequisites().ToCharacter(first.Character);
        }

    }
}
