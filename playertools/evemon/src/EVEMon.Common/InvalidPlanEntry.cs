using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Attributes;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a plan's invalid entry.
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class InvalidPlanEntry
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public InvalidPlanEntry()
        {
            Acknowledged = false;
        }

        /// <summary>
        /// Name of the skill that can not be identified.
        /// </summary>
        public string SkillName
        {
            get;
            set;
        }

        /// <summary>
        /// Planned level.
        /// </summary>
        public int PlannedLevel
        {
            get;
            set;
        }

        /// <summary>
        /// Has the user been notified that this entry has been marked as invalid.
        /// </summary>
        public bool Acknowledged
        {
            get;
            set;
        }
    }
}
