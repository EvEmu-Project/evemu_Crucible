using System;
using System.Collections.Generic;
using EVEMon.Common;

namespace EVEMon.Sales
{
    /// <summary>
    /// Defines an interface to a Mineral Parser.
    /// </summary>
    public interface IMineralParser
    {
        /// <summary>
        /// Gets the title.
        /// </summary>
        /// <value>The title.</value>
        string Title { get; }

        /// <summary>
        /// Gets the courtesy URL.
        /// </summary>
        /// <value>The courtesy URL.</value>
        string CourtesyUrl { get; }

        /// <summary>
        /// Gets the courtesy text.
        /// </summary>
        /// <value>The courtesy text.</value>
        string CourtesyText { get; }

        /// <summary>
        /// Gets the prices.
        /// </summary>
        /// <returns>An enumerable collection of Minerals and Prices.</returns>
        IEnumerable<Pair<string, Decimal>> GetPrices();
    }
}