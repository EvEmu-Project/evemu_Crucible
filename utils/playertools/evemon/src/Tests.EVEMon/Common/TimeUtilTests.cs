using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;
using EVEMon.Common;

namespace Tests.EVEMon.Common
{
    [TestFixture]
    public class TimeUtilTests
    {

        #region Helper Variables

        /// <summary>
        /// Valid time as returned by CCP.
        /// </summary>
        private string ValidCcpDateTime
        {
            get
            {
                return "2010-05-07 18:23:32";
            }
        }

        /// <summary>
        /// Invalid time, wrong format.
        /// </summary>
        private string InvalidCcpDateTime
        {
            get
            {
                return "18:23:32 2010-05-07";
            }
        }

        /// <summary>
        /// A point in time.
        /// </summary>
        private DateTime PointInTime
        {
            get
            {
                return new DateTime(2010, 05, 07, 18, 23, 32);;
            }
        }

        #endregion


        #region Tests

        /// <summary>
        /// Able to convert a <c>DateTime</c> to a CCPTime.
        /// </summary>
        [Test]
        public void ConvertDateTimeToCCPDateTime()
        {
            string result = PointInTime.DateTimeToTimeString();
            Assert.AreEqual(ValidCcpDateTime, result);
        }

        /// <summary>
        /// Able to convert a correctly formatted CCPDateTime to <c>DateTime</c>.
        /// </summary>
        [Test]
        public void ConvertValidCCPDateTimeToDateTime()
        {
            DateTime result = ValidCcpDateTime.TimeStringToDateTime();
            Assert.AreEqual(PointInTime, result);
        }

        /// <summary>
        /// Handles an incorrect input string appropiately.
        /// </summary>
        /// <remarks>
        /// Exception thrown is a .NET Exception so there is no need to test the exception itself.
        /// </remarks>
        [Test]
        public void ConvertInvalidCCPDateTimeToDateTime()
        {
            Assert.Throws<FormatException>
                (delegate()
                {
                    InvalidCcpDateTime.TimeStringToDateTime();
                }
            );
        }

        /// <summary>
        /// Handles an empty string by returning DateTime.MinValue.
        /// </summary>
        [Test]
        public void ConvertEmptyCCPDateTimeToDateTime()
        {
            DateTime result = String.Empty.TimeStringToDateTime();
            Assert.AreEqual(DateTime.MinValue, result);
        }

        /// <summary>
        /// If the time being tested is in the past expect "Done" to be returned.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsDone()
        {
            string result = DateTime.Now.AddHours(-1).ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("Done", result);
        }

        /// <summary>
        /// Test 1s is returned when there is 1 minute to go.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsSecond()
        {
            DateTime future = DateTime.Now.AddSeconds(1);
            string result = future.ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("1s", result);
        }
        
        /// <summary>
        /// Test 1m is returned when there is 1 minute to go.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsMinute()
        {
            DateTime future = DateTime.Now.AddMinutes(1);
            string result = future.ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("1m ", result);
        }

        /// <summary>
        /// Test 1h is returned when there is 1 hour to go.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsHour()
        {
            DateTime future = DateTime.Now.AddHours(1);
            string result = future.ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("1h ", result);
        }

        /// <summary>
        /// Test 1d is returned when there is 1 day to go.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsDay()
        {
            DateTime future = DateTime.Now.AddDays(1);
            string result = future.ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("1d ", result);
        }
        
        /// <summary>
        /// Test 1m 1s is returned when there is 1 minute, 1 second to go.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsMinuteSecond()
        {
            DateTime future = DateTime.Now.AddMinutes(1).AddSeconds(1);
            string result = future.ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("1m 1s", result);
        }

        /// <summary>
        /// Test 1h 1m 1s is returned when there is 1 hour, 1 minute, 1 second to go.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsHourMinuteSecond()
        {
            DateTime future = DateTime.Now.AddHours(1).AddMinutes(1).AddSeconds(1);
            string result = future.ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("1h 1m 1s", result);
        }

        /// <summary>
        /// Test 1d 1h 1m 1s is returned when there is 1 day, 1 hour, 1 minute, 1 second to go.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsDayHourMinuteSecond()
        {
            DateTime future = DateTime.Now.AddDays(1).AddHours(1).AddMinutes(1).AddSeconds(1);
            string result = future.ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("1d 1h 1m 1s", result);
        }
        
        /// <summary>
        /// Test 1d 1m 1s is returned when there is 1 hour, 1 minute, 1 second to go.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsDayMinuteSecond()
        {
            DateTime future = DateTime.Now.AddDays(1).AddMinutes(1).AddSeconds(1);
            string result = future.ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("1d 1m 1s", result);
        }

        /// <summary>
        /// Test 1d 1m is returned when there is 1 hour, 1 minute to go.
        /// </summary>
        [Test]
        public void ToRemainingTimeShortDescriptionReturnsDayMinute()
        {
            DateTime future = DateTime.Now.AddDays(1).AddMinutes(1);
            string result = future.ToRemainingTimeShortDescription(DateTimeKind.Local);
            Assert.AreEqual("1d 1m ", result);
        }

        #endregion

    }
}
