using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common.Scheduling
{
    /// <summary>
    /// Represents a schedule entry which occurs once only.
    /// </summary>
    public class SimpleScheduleEntry : ScheduleEntry
    {
        /// <summary>
        /// Default constructor.
        /// </summary>
        public SimpleScheduleEntry()
        {
        }

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="serial"></param>
        internal SimpleScheduleEntry(SerializableSimpleScheduleEntry serial)
        {
            m_startDate = serial.StartDateTime;
            m_endDate = serial.EndDateTime;
            m_title = serial.Title;
            m_options = serial.Options;
        }

        /// <summary>
        /// Checks whether the given time is contained within this entry
        /// </summary>
        /// <param name="checkDateTime"></param>
        /// <returns></returns>
        public override bool Contains(DateTime checkDateTime)
        {
            return (checkDateTime >= m_startDate && checkDateTime < m_endDate);
        }

        /// <summary>
        /// Gets an enumeration of ranges for this entry within the given interval. It will return at most one range.
        /// </summary>
        /// <param name="fromDt"></param>
        /// <param name="toDt"></param>
        /// <returns></returns>
        public override IEnumerable<ScheduleDateTimeRange> GetRangesInPeriod(DateTime fromDt, DateTime toDt)
        {
            if ((m_startDate < fromDt && m_endDate > fromDt) || (m_startDate >= fromDt && m_startDate <= toDt))
            {
                yield return new ScheduleDateTimeRange(m_startDate, m_endDate);
            }
        }

        /// <summary>
        /// Checks whether the given time intersects with this entry.
        /// </summary>
        /// <param name="timeToTest"></param>
        /// <returns></returns>
        protected override bool Clash(DateTime timeToTest)
        {
            DateTime testtime;
            if ((m_options & ScheduleEntryOptions.EVETime) != 0)
            {
                testtime = timeToTest.ToUniversalTime();
            }
            else
            {
                testtime = timeToTest;
            }

            if (m_startDate <= testtime && testtime <= m_endDate)
                return true;

            return false;
        }

        /// <summary>
        /// Checks whether this entry occurs on the given day.
        /// </summary>
        /// <param name="timeToTest"></param>
        /// <returns></returns>
        public override bool IsToday(DateTime timeToTest)
        {
            if (m_startDate.DayOfYear <= timeToTest.DayOfYear && m_endDate.DayOfYear >= timeToTest.DayOfYear &&
                m_startDate.Year <= timeToTest.Year && m_endDate.Year >= timeToTest.Year)
            {
                return true;
            }

            return false;
        }

        /// <summary>
        /// Exports the data to a serialization object.
        /// </summary>
        /// <returns></returns>
        internal override SerializableScheduleEntry Export()
        {
            SerializableSimpleScheduleEntry serial = new SerializableSimpleScheduleEntry();
            serial.StartDateTime = m_startDate;
            serial.EndDateTime = m_endDate;
            serial.Title = m_title;
            serial.Options = m_options;
            return serial;
        }
    }
}
