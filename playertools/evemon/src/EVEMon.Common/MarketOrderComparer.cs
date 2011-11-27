using System.Collections.Generic;

using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    /// <summary>
    /// Performs a comparison between two <see cref="MarketOrder" /> types.
    /// </summary>
    public sealed class MarketOrderComparer : Comparer<MarketOrder>
    {
        private MarketOrderColumn m_column;
        private bool m_isAscending;

        /// <summary>
        /// Initializes a new instance of the <see cref="MarketOrderComparer"/> class.
        /// </summary>
        /// <param name="column">The market order column.</param>
        /// <param name="isAscending">Is ascending flag.</param>
        public MarketOrderComparer(MarketOrderColumn column, bool isAscending)
        {
            m_column = column;
            m_isAscending = isAscending;
        }

        /// <summary>
        /// Performs a comparison of two objects of the <see cref="MarketOrder" /> type and returns a value
        /// indicating whether one object is less than, equal to, or greater than the other.
        /// </summary>
        /// <param name="x">The first object to compare.</param>
        /// <param name="y">The second object to compare.</param>
        /// <returns>
        /// Less than zero
        /// <paramref name="x"/> is less than <paramref name="y"/>.
        /// Zero
        /// <paramref name="x"/> equals <paramref name="y"/>.
        /// Greater than zero
        /// <paramref name="x"/> is greater than <paramref name="y"/>.
        /// </returns>
        public override int Compare(MarketOrder x, MarketOrder y)
        {
            if (m_isAscending)
                return CompareCore(x, y);

            return -CompareCore(x, y);
        }

        /// <summary>
        /// Performs a comparison of two objects of the <see cref="MarketOrder" /> type and returns a value
        /// indicating whether one object is less than, equal to, or greater than the other.
        /// </summary>
        /// <param name="x">The first object to compare.</param>
        /// <param name="y">The second object to compare.</param>
        /// <returns>
        /// Less than zero
        /// <paramref name="x"/> is less than <paramref name="y"/>.
        /// Zero
        /// <paramref name="x"/> equals <paramref name="y"/>.
        /// Greater than zero
        /// <paramref name="x"/> is greater than <paramref name="y"/>.
        /// </returns>
        private int CompareCore(MarketOrder x, MarketOrder y)
        {
            switch (m_column)
            {
                case MarketOrderColumn.Duration:
                    return x.Duration.CompareTo(y.Duration);

                case MarketOrderColumn.Expiration:
                    return x.Expiration.CompareTo(y.Expiration);

                case MarketOrderColumn.InitialVolume:
                    return x.InitialVolume.CompareTo(y.InitialVolume);

                case MarketOrderColumn.Issued:
                    return x.Issued.CompareTo(y.Issued);

                case MarketOrderColumn.IssuedFor:
                    return x.IssuedFor.CompareTo(y.IssuedFor);

                case MarketOrderColumn.Item:
                    return x.Item.Name.CompareTo(y.Item.Name);

                case MarketOrderColumn.ItemType:
                    return x.Item.MarketGroup.Name.CompareTo(y.Item.MarketGroup.Name);

                case MarketOrderColumn.Location:
                    return x.Station.CompareTo(y.Station);

                case MarketOrderColumn.MinimumVolume:
                    return x.MinVolume.CompareTo(y.MinVolume);

                case MarketOrderColumn.Region:
                    return x.Station.SolarSystem.Constellation.Region.CompareTo(y.Station.SolarSystem.Constellation.Region);

                case MarketOrderColumn.RemainingVolume:
                    return x.RemainingVolume.CompareTo(y.RemainingVolume);

                case MarketOrderColumn.SolarSystem:
                    return x.Station.SolarSystem.CompareTo(y.Station.SolarSystem);

                case MarketOrderColumn.Station:
                    return x.Station.CompareTo(y.Station);

                case MarketOrderColumn.TotalPrice:
                    return x.TotalPrice.CompareTo(y.TotalPrice);

                case MarketOrderColumn.UnitaryPrice:
                    return x.UnitaryPrice.CompareTo(y.UnitaryPrice);

                case MarketOrderColumn.Volume:
                    // Compare the percent left
                    return (x.InitialVolume * y.RemainingVolume - x.RemainingVolume * y.InitialVolume);

                case MarketOrderColumn.LastStateChange:
                    return x.LastStateChange.CompareTo(y.LastStateChange);

                case MarketOrderColumn.OrderRange:
                    // Compare applies only to BuyOrder 
                    return (x is BuyOrder && y is BuyOrder ? ((BuyOrder)x).Range.CompareTo(((BuyOrder)y).Range) : 0);

                case MarketOrderColumn.Escrow:
                    // Compare applies only to BuyOrder 
                    return (x is BuyOrder && y is BuyOrder ? ((BuyOrder)x).Escrow.CompareTo(((BuyOrder)y).Escrow) : 0);

                default:
                    return 0;
            }
        }
    }
}
