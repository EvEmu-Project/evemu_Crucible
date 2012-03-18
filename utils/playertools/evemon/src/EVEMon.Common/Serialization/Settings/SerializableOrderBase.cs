using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents a market sell order.
    /// </summary>
    public class SerializableOrderBase
    {
        /// <summary>
        /// True if the user choose to delete this order.
        /// </summary>
        [XmlAttribute("ignored")]
        public bool Ignored
        {
            get;
            set;
        }

        /// <summary>
        /// Unique order ID for this order. Note that these are not guaranteed to be unique forever, they can recycle. 
        /// But they are unique for the purpose of one data pull. 
        /// </summary>
        [XmlAttribute("orderID")]
        public long OrderID
        {
            get;
            set;
        }

        [XmlAttribute("itemID")]
        public long ItemID
        {
            get;
            set;
        }

        [XmlAttribute("item")]
        public string Item
        {
            get;
            set;
        }

        [XmlAttribute("stationID")]
        public long StationID
        {
            get;
            set;
        }

        [XmlAttribute("volEntered")]
        public int InitialVolume
        {
            get;
            set;
        }

        [XmlAttribute("volRemaining")]
        public int RemainingVolume
        {
            get;
            set;
        }

        /// <summary>
        /// The minimum volume a buyer can buy.
        /// </summary>
        [XmlAttribute("minVolume")]
        public int MinVolume
        {
            get;
            set;
        }

        /// <summary>
        /// The state of the order.
        /// </summary>
        [XmlAttribute("orderState")]
        public OrderState State
        {
            get;
            set;
        }

        /// <summary>
        /// How many days this order is good for. Expiration is issued + duration in days. 
        /// </summary>
        [XmlAttribute("duration")]
        public int Duration
        {
            get;
            set;
        }

        /// <summary>
        /// The cost per unit for this order.
        /// </summary>
        [XmlAttribute("price")]
        public decimal UnitaryPrice
        {
            get;
            set;
        }

        /// <summary>
        /// The time this order was issued.
        /// </summary>
        [XmlAttribute("issued")]
        public DateTime Issued
        {
            get;
            set;
        }

        /// <summary>
        /// Which this order was issued for.
        /// </summary>
        [XmlAttribute("issuedFor")]
        public IssuedFor IssuedFor
        {
            get;
            set;
        }

        /// <summary>
        /// The time this order was issued.
        /// </summary>
        [XmlAttribute("lastStateChange")]
        public DateTime LastStateChange
        {
            get;
            set;
        }
    }
}
