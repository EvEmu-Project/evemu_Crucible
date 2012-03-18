using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common.Collections;
using EVEMon.Common.Serialization;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// A collection of market orders.
    /// </summary>
    public sealed class MarketOrderCollection : ReadonlyCollection<MarketOrder>
    {
        private readonly Character m_character;

        /// <summary>
        /// Internal constructor.
        /// </summary>
        internal MarketOrderCollection(Character character)
        {
            m_character = character;
        }

        /// <summary>
        /// Imports an enumeration of serialization objects.
        /// </summary>
        /// <param name="src"></param>
        internal void Import(IEnumerable<SerializableOrderBase> src)
        {
            m_items.Clear();
            foreach (SerializableOrderBase srcOrder in src)
            {
                if (srcOrder is SerializableBuyOrder)
                {
                    m_items.Add(new BuyOrder((SerializableBuyOrder)srcOrder));
                }
                else
                {
                    m_items.Add(new SellOrder((SerializableSellOrder)srcOrder));
                }
            }
        }

        /// <summary>
        /// Imports an enumeration of API objects.
        /// </summary>
        /// <param name="src"></param>
        /// <returns>The list of expired orders.</returns>
        internal void Import(IEnumerable<SerializableOrderListItem> src, List<MarketOrder> endedOrders)
        {
            // Mark all orders for deletion 
            // If they are found again on the API feed, they won't be deleted
            // and those set as ignored will be left as ignored
            foreach (MarketOrder order in m_items)
            {
                order.MarkedForDeletion = true;
            }

            // Import the orders from the API
            List<MarketOrder> newOrders = new List<MarketOrder>();
            foreach (var srcOrder in src)
            {
                // Skip long expired orders
                var limit = srcOrder.Issued.AddDays(srcOrder.Duration + MarketOrder.MaxExpirationDays);
                if (limit < DateTime.UtcNow)
                    continue;

                // First check whether it is an existing order
                // If it is, update it and remove the deletion candidate flag
                if (m_items.Any(x => x.TryImport(srcOrder, endedOrders)))
                    continue;

                // It's a new order, let's add it
                if (srcOrder.IsBuyOrder != 0)
                {
                    BuyOrder order = new BuyOrder(srcOrder);
                    if (order.Item != null)
                        newOrders.Add(order);
                }
                else
                {
                    SellOrder order = new SellOrder(srcOrder);
                    if (order.Item != null)
                        newOrders.Add(order);
                }
            }

            // Add the items that are no longer marked for deletion
            newOrders.AddRange(m_items.Where(x => !x.MarkedForDeletion));

            // Replace the old list with the new one
            m_items.Clear();
            m_items.AddRange(newOrders);
        }

        /// <summary>
        /// Exports the orders to a serialization object for the settings file.
        /// </summary>
        /// <returns></returns>
        internal List<SerializableOrderBase> Export()
        {
            List<SerializableOrderBase> serial = new List<SerializableOrderBase>(m_items.Count);

            foreach (MarketOrder order in m_items)
            {
                serial.Add(order.Export());
            }

            return serial;
        }
    }
}
