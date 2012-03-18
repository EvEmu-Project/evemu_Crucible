using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common.Scheduling
{
    /// <summary>
    /// Represents a schedule entry.
    /// </summary>
    public abstract class ScheduleEntry
    {
        protected string m_title = String.Empty;
        protected DateTime m_startDate = DateTime.MinValue;
        protected DateTime m_endDate = DateTime.MinValue;
        protected ScheduleEntryOptions m_options = ScheduleEntryOptions.None;

        /// <summary>
        /// Gets or sets the start date for the validity of this schedule entry
        /// </summary>
        public DateTime StartDate
        {
            get { return m_startDate; }
            set { m_startDate = value; }
        }

        /// <summary>
        /// Gets or sets the end date for the validity of this schedule entry
        /// </summary>
        public DateTime EndDate
        {
            get { return m_endDate; }
            set { m_endDate = value; }
        }

        /// <summary>
        /// Gets or sets this entry's options
        /// </summary>
        public ScheduleEntryOptions Options
        {
            get { return m_options; }
            set { m_options = value; }
        }

        /// <summary>
        /// Gets or sets this entry's title
        /// </summary>
        public string Title
        {
            get { return m_title; }
            set { m_title = value; }
        }

        /// <summary>
        /// Gets true if it is expired.
        /// </summary>
        public bool Expired
        {
            get { return (DateTime.UtcNow > m_endDate.ToUniversalTime()); }
        }

        /// <summary>
        /// Checks whether this entry forces EVEMon to run in silent mode (no tray tooltips nor sounds).
        /// </summary>
        /// <param name="timeToTest"></param>
        /// <returns></returns>
        public bool Silent(DateTime timeToTest)
        {
            if ((m_options & ScheduleEntryOptions.Quiet) != 0)
            {
                return Clash(timeToTest);
            }
            return false;
        }

        /// <summary>
        /// Checks whether this entry intersects with a blocking option with the given time while its options.
        /// </summary>
        /// <param name="timeToTest"></param>
        /// <returns></returns>
        public bool Blocking(DateTime timeToTest)
        {
            if ((m_options & ScheduleEntryOptions.Blocking) != 0)
            {
                return Clash(timeToTest);
            }
            return false;
        }

        /// <summary>
        /// Checks whether the given time is contained within this entry
        /// </summary>
        /// <param name="checkDateTime"></param>
        /// <returns></returns>
        public abstract bool Contains(DateTime checkDateTime);

        /// <summary>
        /// Gets an enumeration of ranges for this entry within the given interval.
        /// </summary>
        /// <param name="fromDt"></param>
        /// <param name="toDt"></param>
        /// <returns></returns>
        public abstract IEnumerable<ScheduleDateTimeRange> GetRangesInPeriod(DateTime fromDt, DateTime toDt);

        /// <summary>
        /// Checks whether the given time intersects with this entry.
        /// </summary>
        /// <param name="timeToTest"></param>
        /// <returns></returns>
        protected abstract bool Clash(DateTime timeToTest);

        /// <summary>
        /// Checks whether this entry occurs on the given day.
        /// </summary>
        /// <param name="timeToTest"></param>
        /// <returns></returns>
        public abstract bool IsToday(DateTime timeToTest);

        /// <summary>
        /// Exports the data to a serialization object.
        /// </summary>
        /// <returns></returns>
        internal abstract SerializableScheduleEntry Export();

        /// <summary>
        /// Gets the entry's title.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return m_title;
        }
    }


}
