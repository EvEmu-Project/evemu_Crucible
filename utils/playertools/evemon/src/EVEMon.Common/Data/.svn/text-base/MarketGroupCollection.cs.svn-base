using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a read-only collection of items
    /// </summary>
    public sealed class MarketGroupCollection : ReadonlyCollection<MarketGroup>
    {
        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="src"></param>
        internal MarketGroupCollection(MarketGroup cat, SerializableMarketGroup[] src)
            : base(src == null ? 0 : src.Length)
        {
            if (src == null)
                return;

            foreach (var subCat in src)
            {
                m_items.Add(new MarketGroup(cat, subCat));
            }
        }
    }
}
