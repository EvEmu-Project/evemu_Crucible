using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common.Scheduling
{
    /// <summary>
    /// Represents a recurring schedule entry
    /// </summary>
    public sealed class RecurringScheduleEntry : ScheduleEntry
    {
        public const int SecondsPerDay = 60 * 60 * 24;

        private MonthlyOverflowResolution m_overflowResolution = MonthlyOverflowResolution.ClipBack;
        private RecurringFrequency m_frequency = RecurringFrequency.Daily;
        private DayOfWeek m_dayOfWeek = DayOfWeek.Monday;
        private int m_weeksPeriod = 1;
        private int m_dayOfMonth = 1;
        private int m_startSecond;
        private int m_endSecond;

        /// <summary>
        /// Default constructor
        /// </summary>
        public RecurringScheduleEntry()
        {
            m_endDate = DateTime.MaxValue;
        }

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        internal RecurringScheduleEntry(SerializableRecurringScheduleEntry serial)
        {
            m_startDate = serial.StartDate;
            m_endDate = serial.EndDate;
            m_title = serial.Title;
            m_options = serial.Options;
            m_dayOfMonth = serial.DayOfMonth;
            m_dayOfWeek = serial.DayOfWeek;
            m_startSecond = serial.StartTimeInSeconds;
            m_endSecond = serial.EndTimeInSeconds;
            m_frequency = serial.Frequency;
            m_weeksPeriod = serial.WeeksPeriod;
            m_overflowResolution = serial.OverflowResolution;
        }

        /// <summary>
        /// Gets or sets the scheduling frequency (monthly, weekly, etc).
        /// </summary>
        public RecurringFrequency Frequency
        {
            get { return m_frequency; }
            set { m_frequency = value; }
        }

        /// <summary>
        /// Gets or sets the weeks period (for weekly frequency). <c>1</c> for every week, <c>2</c> for every two weeks, etc...
        /// </summary>
        public int WeeksPeriod
        {
            get { return m_weeksPeriod; }
            set { m_weeksPeriod = value; }
        }

        /// <summary>
        /// Gets or sets the day of week (for weekly frequency)
        /// </summary>
        public DayOfWeek DayOfWeek
        {
            get { return m_dayOfWeek; }
            set { m_dayOfWeek = value; }
        }

        /// <summary>
        /// Gets or sets the day of month (for monthly frequency)
        /// </summary>
        public int DayOfMonth
        {
            get { return m_dayOfMonth; }
            set { m_dayOfMonth = value; }
        }

        /// <summary>
        /// Gets or sets how overflow are dealt with (for monthly frequency).
        /// </summary>
        public MonthlyOverflowResolution OverflowResolution
        {
            get { return m_overflowResolution; }
            set { m_overflowResolution = value; }
        }

        /// <summary>
        /// Gets or sets the start time, in seconds, of an occurence of this entry
        /// </summary>
        public int StartTimeInSeconds
        {
            get { return m_startSecond; }
            set { m_startSecond = value; }
        }

        /// <summary>
        /// Gets or sets the end time, in seconds, of an occurence of this entry
        /// </summary>
        public int EndTimeInSeconds
        {
            get { return m_endSecond; }
            set { m_endSecond = value; }
        }

        /// <summary>
        /// Checks whether the given time is contained within this entry
        /// </summary>
        /// <param name="checkDateTime"></param>
        /// <returns></returns>
        public override bool Contains(DateTime checkDateTime)
        {
            IEnumerable<ScheduleDateTimeRange> ranges = GetRangesInPeriod(checkDateTime, checkDateTime);
            foreach (ScheduleDateTimeRange sdtr in ranges)
            {
                if (checkDateTime >= sdtr.From && checkDateTime < sdtr.To)
                    return true;
            }
            return false;
        }

        /// <summary>
        /// Gets an enumeration of the occurences ranges of this entry within the given interval.
        /// </summary>
        /// <param name="fromDt"></param>
        /// <param name="toDt"></param>
        /// <returns></returns>
        public override IEnumerable<ScheduleDateTimeRange> GetRangesInPeriod(DateTime fromDt, DateTime toDt)
        {
            DateTime startDt = fromDt.Date;
            DateTime endDt = toDt.Date + TimeSpan.FromDays(1);

            if (m_endSecond > SecondsPerDay)
                startDt -= TimeSpan.FromDays(1);

            DateTime wrkDt = startDt;
            for (DateTime wrkDT = startDt; wrkDt < endDt; wrkDt += TimeSpan.FromDays(1))
            {
                var range = GetRangeForDay(wrkDt);
                if (range != null)
                    yield return range;
            }
        }

        /// <summary>
        /// Gets the occurence of this entry for the given day.
        /// </summary>
        /// <param name="wrkDt"></param>
        /// <returns>The found occurence, or null when this entry does not occur on that day</returns>
        private ScheduleDateTimeRange GetRangeForDay(DateTime day)
        {
            switch (m_frequency)
            {
                case RecurringFrequency.Daily:
                default:
                    break;

                case RecurringFrequency.Weekdays:
                    if (day.DayOfWeek < DayOfWeek.Monday || day.DayOfWeek > DayOfWeek.Friday)
                        return null;
                    break;

                case RecurringFrequency.Weekends:
                    if (day.DayOfWeek != DayOfWeek.Saturday && day.DayOfWeek != DayOfWeek.Sunday)
                        return null;
                    break;

                case RecurringFrequency.Weekly:
                    DateTime firstInstance = m_startDate.AddDays((m_dayOfWeek - m_startDate.DayOfWeek + 7) % 7);
                    if (day.DayOfWeek != m_dayOfWeek || (day.Subtract(firstInstance).Days % (7 * m_weeksPeriod)) != 0)
                        return null;
                    break;

                case RecurringFrequency.Monthly:
                    if (day.Day != m_dayOfMonth)
                    {
                        if (day.Day <= 3 && m_dayOfMonth >= 29)
                        {
                            if (!IsOverflowDate(day))
                                return null;
                        }
                        else if (day.Day >= 28 && m_dayOfMonth >= 29)
                        {
                            if (!IsOverflowDate(day))
                                return null;
                        }
                        else
                        {
                            return null;
                        }
                    }
                    break;
            }

            return new ScheduleDateTimeRange(
                day + TimeSpan.FromSeconds(m_startSecond),
                day + TimeSpan.FromSeconds(m_endSecond));
        }

        /// <summary>
        /// Checks whether the given day matches the specified day of month according to the overflow options
        /// </summary>
        /// <param name="wrkDt"></param>
        /// <returns></returns>
        private bool IsOverflowDate(DateTime day)
        {
            switch (m_overflowResolution)
            {
                default:
                case MonthlyOverflowResolution.Drop:
                    return false;

                case MonthlyOverflowResolution.OverlapForward:
                    DateTime lastDayOfPreviousMonthDt = day - TimeSpan.FromDays(day.Day);
                    int lastDayOfPreviousMonth = lastDayOfPreviousMonthDt.Day;
                    int dayOfThisMonth = day.Day;
                    return (m_dayOfMonth - lastDayOfPreviousMonth == dayOfThisMonth);

                case MonthlyOverflowResolution.ClipBack:
                    DateTime searchForward = day + TimeSpan.FromDays(1);
                    if (day.Month == searchForward.Month)
                        return false;
                    if (m_dayOfMonth > day.Day)
                        return true;
                    return false;
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

            var range = GetRangeForDay(testtime.Date);
            if (range == null)
                return false;

            var startDate = m_startDate.Add(range.From.TimeOfDay);

            // in the event m_endDate is set to Forever (DateTime.MaxValue) we can't add anything to it
            var endDate = m_endDate == DateTime.MaxValue ? m_endDate : m_endDate.Add(range.From.TimeOfDay);

            if (startDate < testtime && testtime < endDate)
            {
                return range.From < testtime && testtime < range.To;
            }

            return false;
        }

        /// <summary>
        /// Checks whether this entry occurs on the given day.
        /// </summary>
        /// <param name="timeToTest"></param>
        /// <returns></returns>
        public override bool IsToday(DateTime timeToTest)
        {
            if (timeToTest >= m_startDate && timeToTest <= m_endDate)
            {
                var range = this.GetRangeForDay(timeToTest.Date);
                return range != null;
            }

            return false;
        }

        /// <summary>
        /// Exports the data to a serialization object.
        /// </summary>
        /// <returns></returns>
        internal override SerializableScheduleEntry Export()
        {
            SerializableRecurringScheduleEntry serial = new SerializableRecurringScheduleEntry();
            serial.StartDate = m_startDate;
            serial.EndDate = m_endDate;
            serial.Title = m_title;
            serial.Options = m_options;
            serial.DayOfMonth = m_dayOfMonth;
            serial.DayOfWeek = m_dayOfWeek;
            serial.StartTimeInSeconds = m_startSecond;
            serial.EndTimeInSeconds = m_endSecond;
            serial.Frequency = m_frequency;
            serial.WeeksPeriod = m_weeksPeriod;
            serial.OverflowResolution = m_overflowResolution;
            return serial;
        }
    }
}
