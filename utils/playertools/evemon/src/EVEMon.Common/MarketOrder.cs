using System;
using System.Collections.Generic;

using EVEMon.Common.Data;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    #region MarketOrder
    /// <summary>
    /// A base class for market orders.
    /// </summary>
    public abstract class MarketOrder
    {
        /// <summary>
        /// The maximum number of days after expiration. Beyond this limit, we do not import orders anymore.
        /// </summary>
        public const int MaxExpirationDays = 7;
        protected bool m_markedForDeletion;
        protected DateTime m_lastStateChange;
        protected OrderState m_state;
        protected DateTime m_issued;

        protected readonly long m_orderID;
        protected readonly long m_itemID;
        protected readonly Item m_item;
        protected readonly Station m_station;
        protected readonly int m_initialVolume;
        protected readonly int m_minVolume;
        protected readonly int m_duration;
        protected readonly IssuedFor m_issuedFor;

        protected int m_remainingVolume;
        protected decimal m_unitaryPrice;


        /// <summary>
        /// Constructor from the API.
        /// </summary>
        /// <param name="src"></param>
        protected MarketOrder(SerializableOrderListItem src)
        {
            m_state = GetState(src);
            m_orderID = src.OrderID;
            m_itemID = src.ItemID;
            m_item = StaticItems.GetItemByID(src.ItemID);
            m_station = GetStationByID(src.StationID);
            m_unitaryPrice = src.UnitaryPrice;
            m_initialVolume = src.InitialVolume;
            m_remainingVolume = src.RemainingVolume;
            m_lastStateChange = DateTime.UtcNow;
            m_minVolume = src.MinVolume;
            m_duration = src.Duration;
            m_issued = src.Issued;
            m_issuedFor = src.IssuedFor;
        }

        /// <summary>
        /// Constructor from an object deserialized from the settings file.
        /// </summary>
        /// <param name="src"></param>
        protected MarketOrder(SerializableOrderBase src)
        {
            Ignored = src.Ignored;
            m_orderID = src.OrderID;
            m_state = src.State;
            m_itemID = GetItemID(src);
            m_item = GetItem(src);
            m_station = GetStationByID(src.StationID);
            m_unitaryPrice = src.UnitaryPrice;
            m_initialVolume = src.InitialVolume;
            m_remainingVolume = src.RemainingVolume;
            m_lastStateChange = src.LastStateChange;
            m_minVolume = src.MinVolume;
            m_duration = src.Duration;
            m_issued = src.Issued;
            m_issuedFor = (src.IssuedFor == IssuedFor.None ? IssuedFor.Character : src.IssuedFor);
        }

        /// <summary>
        /// Exports the given object to a serialization object.
        /// </summary>
        /// <returns></returns>
        public abstract SerializableOrderBase Export();

        /// <summary>
        /// Fetches the data to the given source.
        /// </summary>
        /// <param name="src"></param>
        protected void Export(SerializableOrderBase src)
        {
            src.Ignored = Ignored;
            src.OrderID = m_orderID;
            src.State = m_state;
            src.ItemID = m_itemID;
            src.Item = (m_item != null ? m_item.Name : "Unknown Item");
            src.StationID = m_station.ID;
            src.UnitaryPrice = m_unitaryPrice;
            src.InitialVolume = m_initialVolume;
            src.RemainingVolume = m_remainingVolume;
            src.LastStateChange = m_lastStateChange;
            src.MinVolume = m_minVolume;
            src.Duration = m_duration;
            src.Issued = m_issued;
            src.IssuedFor = m_issuedFor;
        }

        /// <summary>
        /// Try to update this order with a serialization object from the API.
        /// </summary>
        /// <param name="src"></param>
        /// <returns></returns>
        internal bool TryImport(SerializableOrderListItem src, List<MarketOrder> endedOrders)
        {
            // Note that, before a match is found, all orders have been marked for deletion : m_markedForDeletion == true

            // Checks whether ID is the same (IDs can be recycled ?)
            if (!MatchesWith(src))
                return false;

            // Prevent deletion
            m_markedForDeletion = false;

            // Update infos (if ID is the same it may have been modified either by the market 
            // or by the user [modify order] so we update the orders info that are changeable)
            if (IsModified(src))
            {
                // If it's a buying order, escrow may have changed
                if (src.IsBuyOrder != 0)
                    ((BuyOrder)this).Escrow = src.Escrow;

                m_unitaryPrice = src.UnitaryPrice;
                m_remainingVolume = src.RemainingVolume;
                m_issued = src.Issued;
                m_state = OrderState.Modified;
            }
            else if (m_state == OrderState.Modified)
            {
                m_state = OrderState.Active;
            }

            // Update state
            OrderState state = GetState(src);
            if (m_state != OrderState.Modified && state != m_state) // it has either expired or fulfilled
            {
                m_state = state;
                m_lastStateChange = DateTime.UtcNow;

                // Should we notify it to the user ?
                if ((state == OrderState.Expired || state == OrderState.Fulfilled) && !Ignored)
                    endedOrders.Add(this);
            }

            return true;
        }

        /// <summary>
        /// Gets an items ID either by source or by name.
        /// </summary>
        /// <param name="src"></param>
        /// <returns></returns>
        private static long GetItemID(SerializableOrderBase src)
        {
            // Try get item ID by source
            var itemID = src.ItemID;

            // We failed? Try get item ID by name
            if (itemID == 0)
            {
                var item = StaticItems.GetItemByName(src.Item);
                itemID = (item == null ? 0 : item.ID);
            }
            
            return itemID;
        }

        /// <summary>
        /// Gets an item by its ID or its name.
        /// </summary>
        /// <param name="src"></param>
        /// <returns></returns>
        private static Item GetItem(SerializableOrderBase src)
        {
            // Try get item by its ID
            Item item = StaticItems.GetItemByID(src.ItemID);

            // We failed? Try get item by its name
            if (item == null)
                item = StaticItems.GetItemByName(src.Item);

            return item;
        }

        /// <summary>
        /// Gets the station of an order.
        /// </summary>
        /// <param name="src"></param>
        /// <returns></returns>
        private static Station GetStationByID(long id)
        {
            // Look for the station in datafile
            Station station = StaticGeography.GetStation(id);

            // We failed ? Then it's a conquerable outpost station
            if (station == null)
                station = ConquerableStation.GetStation(id);

            // We failed again ? It's not in any data we can access
            // We set it to a fixed one and notify about it in the trace file
            if (station == null)
            {
                station = StaticGeography.GetStation(60013747);
                EveClient.Trace("Could not find station id {0}", id);
                EveClient.Trace("Setting to {0}", station.Name);
            }

            return station;
        }

        /// <summary>
        /// Gets the state of an order.
        /// </summary>
        /// <param name="src"></param>
        /// <returns></returns>
        private static OrderState GetState(SerializableOrderListItem src)
        {
            switch ((APIEnumerations.CCPOrderState)src.State)
            {
                case APIEnumerations.CCPOrderState.Closed:
                case APIEnumerations.CCPOrderState.Canceled:
                case APIEnumerations.CCPOrderState.CharacterDeleted:
                    return OrderState.Canceled;

                case APIEnumerations.CCPOrderState.Pending:
                case APIEnumerations.CCPOrderState.Opened:
                    return OrderState.Active;

                case APIEnumerations.CCPOrderState.ExpiredOrFulfilled:
                    return (src.RemainingVolume == 0 ? OrderState.Fulfilled : OrderState.Expired);

                default:
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        /// When true, the order will be deleted unless it was found on the API feed.
        /// </summary>
        internal bool MarkedForDeletion
        {
            get { return m_markedForDeletion; }
            set { m_markedForDeletion = value; }
        }

        /// <summary>
        /// Gets or sets whether an expired order has been deleted by the user.
        /// </summary>
        public bool Ignored { get; set; }

        /// <summary>
        /// Gets the order state.
        /// </summary>
        public OrderState State
        {
            get 
            {
                if (IsExpired && (m_state == OrderState.Active || m_state == OrderState.Modified))
                    return OrderState.Expired;

                return m_state; 
            }
        }

        /// <summary>
        /// Gets the order ID.
        /// </summary>
        public long ID
        {
            get { return m_orderID; }
        }

        /// <summary>
        /// Gets the item. May be null for some items like trade goods and such. In such a case, use <see cref="ItemName"/>.
        /// </summary>
        public Item Item
        {
            get { return m_item; }
        }

        /// <summary>
        /// Gets the station where this order is located.
        /// </summary>
        public Station Station
        {
            get { return m_station; }
        }

        /// <summary>
        /// Gets the intial volume.
        /// </summary>
        public int InitialVolume
        {
            get { return m_initialVolume; }
        }

        /// <summary>
        /// Gets the remaining volume.
        /// </summary>
        public int RemainingVolume
        {
            get { return m_remainingVolume; }
        }

        /// <summary>
        /// Gets the minimum sell/buy threshold volume.
        /// </summary>
        public int MinVolume
        {
            get { return m_minVolume; }
        }

        /// <summary>
        /// Gets the duration, in days, of the order.
        /// </summary>
        public int Duration
        {
            get { return m_duration; }
        }

        /// <summary>
        /// Gets the unitary price.
        /// </summary>
        public decimal UnitaryPrice
        {
            get { return m_unitaryPrice; }
        }

        /// <summary>
        /// Gets the total price.
        /// </summary>
        public decimal TotalPrice
        {
            get { return m_unitaryPrice * m_remainingVolume; }
        }

        /// <summary>
        /// Gets the time (UTC) this order was expired.
        /// </summary>
        public DateTime Issued
        {
            get { return m_issued; }
        }

        /// <summary>
        /// Gets for which the order was issued.
        /// </summary>
        public IssuedFor IssuedFor
        {
            get { return m_issuedFor; }
        }
        
        /// <summary>
        /// Gets the estimated expiration time.
        /// </summary>
        public DateTime Expiration
        {
            get { return m_issued.AddDays(m_duration); }
        }

        /// <summary>
        /// Gets the last state change.
        /// </summary>
        public DateTime LastStateChange
        {
            get { return m_lastStateChange; }
        }

        /// <summary>
        /// Gets true if order naturally expired because of its duration.
        /// </summary>
        public bool IsExpired
        {
            get { return m_issued.AddDays(m_duration) < DateTime.UtcNow; }
        }

        /// <summary>
        /// Gets true if the order is not fulfilled, canceled, expired, etc.
        /// </summary>
        public bool IsAvailable
        {
            get { return (m_state == OrderState.Active || m_state == OrderState.Modified) && !IsExpired; }
        }

        /// <summary>
        /// Checks whether the given API object matches with this order.
        /// </summary>
        /// <param name="src"></param>
        /// <returns></returns>
        internal bool MatchesWith(SerializableOrderListItem src)
        {
            return src.OrderID == m_orderID;
        }

        /// <summary>
        /// Checks whether the given API object matches with this order.
        /// </summary>
        /// <param name="src"></param>
        /// <returns></returns>
        internal bool IsModified (SerializableOrderListItem src)
        {
            return src.RemainingVolume != 0
                && ((src.UnitaryPrice != m_unitaryPrice && src.Issued != m_issued)
                || src.RemainingVolume != m_remainingVolume);
        }

        /// <summary>
        /// Formats the given price to a readable string.
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        public static string Format(decimal value, AbbreviationFormat format)
        {
            decimal abs = Math.Abs(value);
            if (format == AbbreviationFormat.AbbreviationWords)
            {
                if (abs >= 1E9M)
                    return Format("Billions", value / 1E9M);
                if (abs >= 1E6M)
                    return Format("Millions", value / 1E6M);
                if (abs >= 1E3M)
                    return Format("Thousands", value / 1E3M);
                
                return Format("", value);
            }
            else
            {
                if (abs >= 1E9M)
                    return Format("B", value / 1E9M);
                if (abs >= 1E6M)
                    return Format("M", value / 1E6M);
                if (abs >= 1E3M)
                    return Format("K", value / 1E3M);
                
                return Format("", value);
            }
        }

        /// <summary>
        /// Formats the given value and suffix the way we want.
        /// </summary>
        /// <param name="suffix"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        private static string Format(string suffix, decimal value)
        {
            // Explanations : 999.99 was displayed as 1000 because only three digits were required.
            // So we do the truncation at hand for the number of digits we exactly request.

            var abs = Math.Abs(value);
            if (abs < 1.0M)
                return (((int)value * 100) / 100.0M).ToString("0.##") + suffix;
            if (abs < 10.0M)
                return (((int)value * 1000) / 1000.0M).ToString("#.##") + suffix;
            if (abs < 100.0M)
                return (((int)value * 1000)/1000.0M).ToString("##.#") + suffix;

            return (((int)value * 1000) / 1000.0M).ToString("###") + suffix;
        }
    }
    #endregion
}
