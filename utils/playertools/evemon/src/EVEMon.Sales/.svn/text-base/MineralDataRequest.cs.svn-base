using System;
using System.Collections.Generic;
using System.Reflection;
using EVEMon.Common;

namespace EVEMon.Sales
{
    /// <summary>
    /// Gets mineral data from parsers.
    /// </summary>
    public static class MineralDataRequest
    {
        /// <summary>
        /// Initializes the <see cref="MineralDataRequest"/> class.
        /// </summary>
        static MineralDataRequest()
        {
            Assembly asm = Assembly.GetExecutingAssembly();
            foreach (Type tt in asm.GetTypes())
            {
                foreach (
                    DefaultMineralParserAttribute dmpa in
                        tt.GetCustomAttributes(typeof(DefaultMineralParserAttribute), false))
                {
                    IMineralParser mp = Activator.CreateInstance(tt) as IMineralParser;
                    if (mp != null)
                    {
                        RegisterDataSource(dmpa.Name, mp);
                    }
                }
            }
        }

        private static Dictionary<string, IMineralParser> s_parsers = new Dictionary<string, IMineralParser>();

        /// <summary>
        /// Gets the parsers.
        /// </summary>
        /// <value>The parsers.</value>
        public static IEnumerable<Pair<string, IMineralParser>> Parsers
        {
            get
            {
                foreach (KeyValuePair<string, IMineralParser> kvp in s_parsers)
                {
                    yield return new Pair<string, IMineralParser>(kvp.Key, kvp.Value);
                }
            }
        }

        /// <summary>
        /// Registers the data source.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="parser">The parser.</param>
        public static void RegisterDataSource(string name, IMineralParser parser)
        {
            s_parsers.Add(name, parser);
        }

        /// <summary>
        /// Gets the prices.
        /// </summary>
        /// <param name="source">The source.</param>
        /// <returns>An enumerable collection of Minerals and Prices.</returns>
        public static IEnumerable<Pair<string, Decimal>> GetPrices(string source)
        {
            if (!s_parsers.ContainsKey(source))
            {
                throw new ArgumentException("that is not a registered mineraldatasource", "source");
            }

            IMineralParser p = s_parsers[source];
            return p.GetPrices();
        }

        /// <summary>
        /// Gets the courtesy text.
        /// </summary>
        /// <param name="source">The source.</param>
        /// <returns>The courtesy text.</returns>
        public static string GetCourtesyText(string source)
        {
            if (!s_parsers.ContainsKey(source))
            {
                throw new ArgumentException("that is not a registered mineraldatasource", "source");
            }

            IMineralParser p = s_parsers[source];
            return p.CourtesyText;
        }

        /// <summary>
        /// Gets the courtesy URL.
        /// </summary>
        /// <param name="source">The source.</param>
        /// <returns>The courtesy URL.</returns>
        public static string GetCourtesyUrl(string source)
        {
            if (!s_parsers.ContainsKey(source))
            {
                throw new ArgumentException("that is not a registered mineraldatasource", "source");
            }

            IMineralParser p = s_parsers[source];
            return p.CourtesyUrl;
        }
    }
}
