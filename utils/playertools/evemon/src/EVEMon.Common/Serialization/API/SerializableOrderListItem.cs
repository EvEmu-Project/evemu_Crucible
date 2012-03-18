using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a market order
    /// </summary>
    public class SerializableOrderListItem
    {
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

        /// <summary>
        /// Only relevant for corporation's market orders.
        /// </summary>
        [XmlAttribute("charID")]
        public long OwnerID
        {
            get;
            set;
        }

        [XmlAttribute("typeID")]
        public int ItemID
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
        /// 0 = open/active, 1 = closed, 2 = expired (or fulfilled), 3 = cancelled, 4 = pending, 5 = character deleted.
        /// </summary>
        [XmlAttribute("orderState")]
        public int State
        {
            get;
            set;
        }

        /// <summary>
        /// The range this order is good for. For sell orders, this is always 32767. 
        /// For buy orders, allowed values are: -1 = station, 0 = solar system, 1 = 1 jump, 2 = 2 jumps, ..., 32767 = region.
        /// </summary>
        [XmlAttribute("range")]
        public int Range
        {
            get;
            set;
        }

        /// <summary>
        /// Which division this order is using as its account. 
        /// Always 1000 for characters, within 1000 to 1006 for corporations.
        /// </summary>
        [XmlAttribute("accountKey")]
        public int DivisionKey
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
        /// How much ISK is in escrow, for buy orders.
        /// </summary>
        [XmlAttribute("escrow")]
        public decimal Escrow
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
        /// 1 if this is a buy order, 0 otherwise.
        /// </summary>
        [XmlAttribute("bid")]
        public int IsBuyOrder
        {
            get;
            set;
        }

        /// <summary>
        /// The time this order was issued.
        /// </summary>
        [XmlIgnore]
        public DateTime Issued
        {
            get;
            set;
        }

        /// <summary>
        /// Which this order was issued for.
        /// </summary>
        [XmlIgnore]
        public IssuedFor IssuedFor
        {
            get;
            set;
        }

        [XmlAttribute("issued")]
        public string IssuedXml
        {
            get { return Issued.DateTimeToTimeString(); }
            set
            {
                if (String.IsNullOrEmpty(value))
                    return;

                Issued = value.TimeStringToDateTime();
            }
        }
    }
}
