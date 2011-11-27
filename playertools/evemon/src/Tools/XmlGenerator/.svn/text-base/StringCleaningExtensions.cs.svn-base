using System;
using System.Text.RegularExpressions;

using EVEMon.XmlGenerator.StaticData;

namespace EVEMon.XmlGenerator
{
    /// <summary>
    /// Series of extension methods to cleanup and format strings for use in the data files.
    /// </summary>
    public static class StringCleaningExtensions
    {
        /// <summary>
        /// Cleans up and normalizes a string by passing it through the following filters.
        /// </summary>
        /// <param name="input"><c>string</c> to be cleaned.</param>
        /// <returns>cleaned <c>string</c></returns>
        public static string Clean(this string input)
        {
            string output;

            output = input.TrimWhitespace();
            output = output.ReplaceTabs();
            output = output.CleanXmlTags();
            output = output.CollapseSpaces();
            output = output.Normalize();

            return output;
        }

        /// <summary>
        /// Colapses any sequence of spaces into a single space.
        /// </summary>
        /// <param name="output"></param>
        /// <returns></returns>
        public static string CollapseSpaces(this string input)
        {
            Regex collapseSpace = new Regex(@"[ ]{2,}", RegexOptions.Compiled);
            return collapseSpace.Replace(input, @" ");
        }

        /// <summary>
        /// Removes any text between opposing angle brackets (i.e. XML or HTML tags).
        /// </summary>
        /// <param name="output"></param>
        /// <returns></returns>
        public static string CleanXmlTags(this string input)
        {
            // Remove markup
            Regex htmlClean = new Regex("<.+?>", RegexOptions.Singleline | RegexOptions.Compiled);
            return htmlClean.Replace(input, String.Empty);
        }

        /// <summary>
        /// Switches any occurance of a tab with a single space.
        /// </summary>
        /// <param name="input"></param>
        /// <returns></returns>
        public static string ReplaceTabs(this string input)
        {
            // Replace tab characters with spaces
            return input.Replace('\t', ' ');
        }

        /// <summary>
        /// Trims whitespace from the beginning and end of a string.
        /// </summary>
        /// <param name="input"></param>
        /// <returns></returns>
        public static string TrimWhitespace(this string input)
        {
            // Remove whitespace from the beginning and end of a string
            return input.Trim();
        }

        /// <summary>
        /// Formats a properties value in a human friendly manner.
        /// </summary>
        /// <param name="property"></param>
        /// <returns></returns>
        public static string FormatPropertyValue(this DgmTypeAttribute property)
        {
            if (property.ValueInt.HasValue)
                return property.ValueInt.ToString();

            // Is it actually an integer stored as a float?
            if (Math.Truncate(property.ValueFloat.Value) == property.ValueFloat.Value)
                return Convert.ToInt32(property.ValueFloat.Value).ToString();
            
            return property.ValueFloat.ToString();
        }

        /// <summary>
        /// Formats a decimal in a human friendly manner.
        /// </summary>
        /// <param name="input"></param>
        /// <returns></returns>
        public static string FormatDecimal(this decimal input)
        {
            // Is it actually an integer stored as a double?
            if (Math.Truncate(input) == input)
                return Convert.ToInt64(input).ToString();

            return input.ToString();
        }
    }
}
