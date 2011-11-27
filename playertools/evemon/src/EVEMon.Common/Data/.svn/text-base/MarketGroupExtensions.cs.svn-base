using System.Collections.Generic;

namespace EVEMon.Common.Data
{
    public static class MarketGroupExtensions
    {
        /// <summary>
        /// Gets true if the item's market group belongs to the questioned group. 
        /// </summary>
        public static bool BelongsIn(this MarketGroup marketGroup, IEnumerable<int> group)
        {
            while (marketGroup != null)
            {
                foreach (var groupID in group)
                {
                    if (marketGroup.ID == groupID)
                        return true;
                }

                marketGroup = marketGroup.ParentGroup;
            }

            return false;
        }
    }
}
