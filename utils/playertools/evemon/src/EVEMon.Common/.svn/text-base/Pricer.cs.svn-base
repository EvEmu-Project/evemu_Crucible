using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common.Data;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Threading;

namespace EVEMon.Common
{
    public sealed class PricerMaketOrdersQueryResult
    {
        public string ErrorMessage
        {
            get;
            set;
        }

        public IEnumerable<MarketOrder> Orders
        {
            get 
            {
                foreach(var station in StaticGeography.AllSystems.First(x => x.Name == "Jita").Constellation)
                {
                    yield return new SellOrder(new SerializableOrderListItem 
                    { 
                        Range = 0, 
                        MinVolume = 1, 
                        ItemID = 35, 
                        StationID = station.ID, 
                        RemainingVolume = 5000, 
                        UnitaryPrice = 10.0M
                    });
                }
            }
        }
    }

    public static class Pricer
    {
        /// <summary>
        /// Asynchronously queries the market orders
        /// </summary>
        /// <param name="itemIDs">The IDs of the types to query, see the items datafile.</param>
        /// <param name="regions">The IDs of the regions to query.</param>
        /// <param name="callback">A callback invoked on the UI thread.</param>
        public static void QueryRegionalMarketOrdersAsync(IEnumerable<int> itemIDs, IEnumerable<int> regions, Action<PricerMaketOrdersQueryResult> callback)
        {
            Dispatcher.Schedule(TimeSpan.FromSeconds(1.0), () => callback(new PricerMaketOrdersQueryResult()));
        }
    }
}
