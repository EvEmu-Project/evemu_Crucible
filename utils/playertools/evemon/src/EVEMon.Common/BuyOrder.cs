using System;

using EVEMon.Common.Serialization;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// This class represents a buy order.
    /// </summary>
    public sealed class BuyOrder : MarketOrder
    {
        private readonly int m_range;

        /// <summary>
        /// Constructor from the API.
        /// </summary>
        /// <param name="src"></param>
        internal BuyOrder(SerializableOrderListItem src)
            : base(src)
        {
            Escrow = src.Escrow;
            m_range = src.Range;
        }

        /// <summary>
        /// Constructor from an object deserialized from the settings file.
        /// </summary>
        /// <param name="src"></param>
        internal BuyOrder(SerializableBuyOrder src)
            : base(src)
        {
            Escrow = src.Escrow;
            m_range = src.Range;
        }

        /// <summary>
        /// Gets the amount currently invested in escrow.
        /// </summary>
        public decimal Escrow { get; internal set; }

        /// <summary>
        /// Gets the range of this order.
        /// </summary>
        public int Range
        {
            get { return m_range; }
        }

        /// <summary>
        /// Gets the description of the range.
        /// </summary>
        public string RangeDescription
        {
            get
            {
                switch (m_range)
                {
                    case -1:
                        return "Station";
                    case 0:
                        return "Solar System";
                    case 1:
                        return String.Format("{0} jump", m_range.ToString());
                    case EveConstants.RegionRange:
                        return "Region";
                    default:
                        return String.Format("{0} jumps", m_range.ToString());
                }
            }
        }

        /// <summary>
        /// Exports the given object to a serialization object.
        /// </summary>
        /// <returns></returns>
        public override SerializableOrderBase Export()
        {
            var serial = new SerializableBuyOrder();
            serial.Escrow = Escrow;
            serial.Range = m_range;
            Export(serial);
            return serial;
        }
    }
}
