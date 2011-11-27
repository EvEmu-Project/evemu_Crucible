using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a read-only collection of items
    /// </summary>
    public sealed class ItemCollection : ReadonlyCollection<Item>
    {
        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="group">Market Group for the item</param>
        /// <param name="src">One or more source serializable items</param>
        internal ItemCollection(MarketGroup group, SerializableItem[] src)
            : base(src == null ? 0 : src.Length)
        {
            if (src == null)
                return;

            foreach (var item in src)
            {
                switch (item.Family)
                {
                    default:
                        m_items.Add(new Item(group, item));
                        break;
                    case ItemFamily.Implant:
                        m_items.Add(new Implant(group, item));
                        break;
                    case ItemFamily.Ship:
                        m_items.Add(new Ship(group, item));
                        break;
                }
            }
        }
    }
}
