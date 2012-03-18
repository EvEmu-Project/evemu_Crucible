using System;
using System.Collections.Generic;
using System.Text;
using System.Globalization;

namespace EVEMon.Common
{
    public static class TimeExtensions
    {
        /// <summary>
        /// Converts a UTC DateTime to the API date/time string.
        /// </summary>
        /// <param name="timeUTC"></param>
        /// <returns></returns>
        public static string DateTimeToTimeString(this DateTime timeUTC)
        {
            // timeUTC = yyyy-MM-dd HH:mm:ss
            string result = String.Format(CultureConstants.DefaultCulture, "{0:d4}-{1:d2}-{2:d2} {3:d2}:{4:d2}:{5:d2}",
                                          timeUTC.Year,
                                          timeUTC.Month,
                                          timeUTC.Day,
                                          timeUTC.Hour,
                                          timeUTC.Minute,
                                          timeUTC.Second);
            return result;
        }

        /// <summary>
        /// Converts an API date/time string to a UTC DateTime.
        /// </summary>
        /// <param name="timeUTC"></param>
        /// <returns></returns>
        public static DateTime TimeStringToDateTime(this String timeUTC)
        {
            DateTime dt = DateTime.MinValue;

            // timeUTC = yyyy-MM-dd HH:mm:ss
            if (String.IsNullOrEmpty(timeUTC))
                return dt;

            DateTime.TryParse(timeUTC, CultureConstants.DefaultCulture, DateTimeStyles.AdjustToUniversal, out dt);

            return dt;
        }
        
        /// <summary>
        /// Returns a string representation for the time left to the given date, using the following formats : 
        /// <list type="bullet">
        /// <item>1d3h5m6s</item>
        /// <item>3h5m</item>
        /// <item>Done</item>
        /// </list>
        /// </summary>
        /// <param name="t">The time (in the future) for which you want a span.</param>
        /// <returns></returns>
        public static string ToRemainingTimeShortDescription(this DateTime t, DateTimeKind dateTimeKind)
        {
            DateTime now = (dateTimeKind == DateTimeKind.Local ? DateTime.Now : DateTime.UtcNow);
            
            // Small chance that the function could cross over the
            // second boundry, and have an inconsistent result.
            StringBuilder sb = new StringBuilder();
            if (t > now)
            {
                TimeSpan ts = t.Subtract(now);
                if (ts.Days > 0)
                {
                    sb.Append(ts.Days.ToString());
                    sb.Append("d ");
                }
                ts -= TimeSpan.FromDays(ts.Days);
                if (ts.Hours > 0)
                {
                    sb.Append(ts.Hours.ToString());
                    sb.Append("h ");
                }
                ts -= TimeSpan.FromHours(ts.Hours);
                if (ts.Minutes > 0)
                {
                    sb.Append(ts.Minutes.ToString());
                    sb.Append("m ");
                }
                ts -= TimeSpan.FromMinutes(ts.Minutes);
                if (ts.Seconds > 0)
                {
                    sb.Append(ts.Seconds.ToString());
                    sb.Append("s");
                }
                return sb.ToString();
            }
            else
            {
                return "Done";
            }
        }

        /// <summary>
        /// Returns a string representation for the time left to the given date, using the following formats :
        /// <list type="bullet">
        /// 		<item>2 days, 3 hours, 1 minute, 5seconds</item>
        /// 		<item>3 hours, 1 minute</item>
        /// 		<item>Completed</item>
        /// 	</list>
        /// </summary>
        /// <param name="t">The time (in the future) for which you want a span.</param>
        /// <param name="dateTimeKind">The kind of the dateTime (UTC or Local) being converted.</param>
        /// <remarks>DateTimeKind.Unspecified will be treated as UTC</remarks>
        /// <returns></returns>
        public static string ToRemainingTimeDescription(this DateTime t, DateTimeKind dateTimeKind)
        {
            DateTime now = (dateTimeKind == DateTimeKind.Local ? DateTime.Now : DateTime.UtcNow);
            
            StringBuilder sb = new StringBuilder();
            if (t > now)
            {
                TimeSpan ts = t.Subtract(now);
                if (ts.Days > 0)
                {
                    sb.Append(ts.Days.ToString());
                    sb.Append(" day");
                    if (ts.Days > 1)
                        sb.Append("s");
                }
                ts -= TimeSpan.FromDays(ts.Days);
                if (ts.Hours > 0)
                {
                    if (sb.Length > 0)
                        sb.Append(", ");

                    sb.Append(ts.Hours.ToString());
                    sb.Append(" hour");
                    if (ts.Hours > 1)
                        sb.Append("s");
                }
                ts -= TimeSpan.FromHours(ts.Hours);
                if (ts.Minutes > 0)
                {
                    if (sb.Length > 0)
                        sb.Append(", ");

                    sb.Append(ts.Minutes.ToString());
                    sb.Append(" minute");
                    if (ts.Minutes > 1)
                        sb.Append("s");
                }
                ts -= TimeSpan.FromMinutes(ts.Minutes);
                if (ts.Seconds > 0)
                {
                    if (sb.Length > 0)
                        sb.Append(", ");
                    sb.Append(ts.Seconds.ToString());
                    sb.Append(" second");
                    if (ts.Seconds > 1)
                        sb.Append("s");
                }
                return sb.ToString();
            }
            else
            {
                return "Completed";
            }
        }

        /// <summary>
        /// Generates an absolute string based upon the following format :
        /// <list type="bullet">
        /// 		<item>17:27 Tomorrow</item>
        /// 		<item>07:43 Wednesday</item>
        /// 		<item>03:12 23/04/2009</item>
        /// 	</list>
        /// </summary>
        /// <param name="absoluteDateTime">The absolute date time.</param>
        /// <param name="dateTimeKind">The kind of the dateTime (UTC or Local) being converted.</param>
        /// <remarks>DateTimeKind.Unspecified will be treated as UTC</remarks>
        /// <returns>String representation of the time and relative date.</returns>
        public static string ToAbsoluteDateTimeDescription(this DateTime absoluteDateTime, DateTimeKind dateTimeKind)
        {
            DateTime now = (dateTimeKind == DateTimeKind.Local ? DateTime.Now : DateTime.UtcNow);
            string shortTime = absoluteDateTime.ToCustomShortTimeString();

            // Yesterday (i.e. before 00:00 today)
            if (absoluteDateTime.Date == now.Date.AddDays(-1))
                return String.Format(CultureConstants.DefaultCulture, "{0} Yesterday", shortTime);

            // Today (i.e. before 00:00 tomorrow)
            if (absoluteDateTime.Date == now.Date)
                return String.Format(CultureConstants.DefaultCulture, "{0} Today", shortTime);

            // Tomorrow (i.e. after 23:59 today but before 00:00 the day after tomorrow)
            DateTime tomorrow = now.Date.AddDays(1);
            if (absoluteDateTime.Date == tomorrow)
                return String.Format(CultureConstants.DefaultCulture, "{0} Tomorrow", shortTime);

            // After tomorrow but within 7 days
            DateTime sevenDays = now.Date.AddDays(7);
            if (absoluteDateTime.Date > tomorrow)
            {
                string dayOfWeek = absoluteDateTime.DayOfWeek.ToString();
                if (absoluteDateTime.Date < sevenDays)
                    return String.Format(CultureConstants.DefaultCulture, "{0} This {1}", shortTime, dayOfWeek);

                if (absoluteDateTime.Date == sevenDays)
                    return String.Format(CultureConstants.DefaultCulture, "{0} Next {1}", shortTime, dayOfWeek);
            }

            // More than seven days away or more than one day ago
            string shortDate = absoluteDateTime.ToString("d", CultureConstants.DefaultCulture);
            return String.Format(CultureConstants.DefaultCulture, "{0} {1}", shortTime, shortDate);
        }

        /// <summary>
        /// Formats a DateTime object as a short string (HH:mm or hh:mm tt).
        /// </summary>
        /// <remarks>
        /// Due to a reason that escapes me, the ShortTimePattern
        /// function doesn't respect system settings, so we have to
        /// mangle LongTimePattern to track users preference.
        /// </remarks>
        /// <param name="ShortTimeString">DateTime to be formatted</param>
        /// <returns>String containing formatted text</returns>
        public static string ToCustomShortTimeString(this DateTime shortTimeString)
        {
            DateTimeFormatInfo dateTimeFormat = CultureConstants.DefaultCulture.DateTimeFormat;
            
            // Get the LongTimePattern and remove :ss and :s
            string shortTimePattern = dateTimeFormat.LongTimePattern.Replace(":ss", String.Empty);
            shortTimePattern = shortTimePattern.Replace(":s", String.Empty);

            return shortTimeString.ToString(shortTimePattern);
        }

        /// <summary>
        /// Convert a timespan into English text.
        /// </summary>
        /// <param name="ts">The timespan.</param>
        /// <param name="dto">Formatting options.</param>
        /// <returns>Timespan formatted as English text.</returns>
        public static string ToDescriptiveText(this TimeSpan ts, DescriptiveTextOptions dto)
        {
            return ToDescriptiveText(ts, dto, true);
        }

        /// <summary>
        /// Convert a timespan into English text.
        /// </summary>
        /// <param name="ts">The timespan.</param>
        /// <param name="dto">Formatting options.</param>
        /// <returns>Timespan formatted as English text.</returns>
        public static string ToDescriptiveText(this TimeSpan ts, DescriptiveTextOptions dto, bool includeSeconds)
        {
            StringBuilder sb = new StringBuilder();

            BuildDescriptiveFragment(sb, ts.Days, dto, "days");
            BuildDescriptiveFragment(sb, ts.Hours, dto, "hours");
            BuildDescriptiveFragment(sb, ts.Minutes, dto, "minutes");

            if (includeSeconds)
                BuildDescriptiveFragment(sb, ts.Seconds, dto, "seconds");

            if (sb.Length == 0)
                sb.Append("(none)");

            return sb.ToString();
        }

        /// <summary>
        /// Builds the string representation for this string, just for one part of the time
        /// (may be the days, the hours, the mins, etc).
        /// </summary>
        /// <param name="sb"></param>
        /// <param name="p"></param>
        /// <param name="dto"></param>
        /// <param name="dstr"></param>
        private static void BuildDescriptiveFragment(StringBuilder sb, int p, DescriptiveTextOptions dto, string dstr)
        {
            if (((dto & DescriptiveTextOptions.IncludeZeroes) == 0) && p == 0)
                return;

            if ((dto & DescriptiveTextOptions.IncludeCommas) != 0)
            {
                if (sb.Length > 0)
                    sb.Append(", ");
            }

            if ((dto & DescriptiveTextOptions.SpaceBetween) != 0)
                sb.Append(" ");

            sb.Append(p.ToString());

            if ((dto & DescriptiveTextOptions.SpaceText) != 0)
                sb.Append(' ');

            if ((dto & DescriptiveTextOptions.FirstLetterUppercase) != 0)
                dstr = char.ToUpper(dstr[0], CultureConstants.DefaultCulture) + dstr.Substring(1);

            if ((dto & DescriptiveTextOptions.UppercaseText) != 0)
                dstr = dstr.ToUpper(CultureConstants.DefaultCulture);

            if ((dto & DescriptiveTextOptions.FullText) != 0)
            {
                if (p == 1)
                    dstr = dstr.Substring(0, dstr.Length - 1);

                sb.Append(dstr);
            }
            else
            {
                sb.Append(dstr[0]);
            }
        }
    }
}
