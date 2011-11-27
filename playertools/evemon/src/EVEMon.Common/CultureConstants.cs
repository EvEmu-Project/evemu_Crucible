using System;
using System.Collections.Generic;
using System.Text;
using System.Globalization;

namespace EVEMon.Common
{
    public static class CultureConstants
    {
        public static readonly CultureInfo DefaultCulture = CultureInfo.CurrentCulture;

        /// <summary>
        /// Formats an integer with thousand seperators but without
        /// the decimal digits.
        /// </summary>
        /// <remarks>
        /// Useful for formatting integers as the format "n" will
        /// format as follows:
        ///   20923 = 20,923.00
        ///   
        /// This format will format as follows:
        ///   20923 = 20,923
        /// </remarks>
        public static NumberFormatInfo TidyInteger
        {
            get
            {
                var thousandFormat = (NumberFormatInfo)DefaultCulture.NumberFormat.Clone();
                thousandFormat.NumberDecimalDigits = 0;
                return thousandFormat;
            }
        }
    }
}
