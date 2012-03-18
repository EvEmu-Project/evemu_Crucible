using System;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Provides conversions to durations.
    /// </summary>
    public static class UpdatePeriodExtensions
    {
        public static TimeSpan ToDuration(this UpdatePeriod period)
        {
            switch (period)
            {
                case UpdatePeriod.Never:
                    return TimeSpan.MaxValue;
                case UpdatePeriod.Minutes5:
                    return TimeSpan.FromMinutes(5);
                case UpdatePeriod.Minutes15:
                    return TimeSpan.FromMinutes(15);
                case UpdatePeriod.Minutes30:
                    return TimeSpan.FromMinutes(30);
                case UpdatePeriod.Hours1:
                    return TimeSpan.FromHours(1);
                case UpdatePeriod.Hours2:
                    return TimeSpan.FromHours(2);
                case UpdatePeriod.Hours3:
                    return TimeSpan.FromHours(3);
                case UpdatePeriod.Hours6:
                    return TimeSpan.FromHours(6);
                case UpdatePeriod.Hours12:
                    return TimeSpan.FromHours(12);
                case UpdatePeriod.Day:
                    return TimeSpan.FromDays(1);
                case UpdatePeriod.Week:
                    return TimeSpan.FromDays(7);
                default:
                    throw new NotImplementedException();
            }
        }
    }
}
