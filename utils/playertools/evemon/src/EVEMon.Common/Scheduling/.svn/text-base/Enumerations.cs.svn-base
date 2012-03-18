using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Scheduling
{

    /// <summary>
    /// Describes the frequency at which a schedule entry occurs
    /// </summary>
    public enum RecurringFrequency
    {
        Daily,
        Weekdays,
        Weekends,
        Weekly,
        Monthly
    }

    /// <summary>
    /// Describes the options of a schedule entry
    /// </summary>
    [Flags]
    public enum ScheduleEntryOptions
    {
        None = 0,
        /// <summary>
        /// Blocks skills training starting
        /// </summary>
        Blocking = 1,
        /// <summary>
        /// No tooltip notifications.
        /// </summary>
        Quiet = 2,
        /// <summary>
        /// Uses EVETime
        /// </summary>
        EVETime = 4
    }

    /// <summary>
    /// Describes the behaviour when a month is overflowed
    /// </summary>
    public enum MonthlyOverflowResolution
    {
        /// <summary>
        /// April 31 becomes April 30
        /// </summary>
        ClipBack,
        /// <summary>
        /// April 31 is ignored
        /// </summary>
        Drop,
        /// <summary>
        /// Apris 31 becomes May 1
        /// </summary>
        OverlapForward
    }
}
