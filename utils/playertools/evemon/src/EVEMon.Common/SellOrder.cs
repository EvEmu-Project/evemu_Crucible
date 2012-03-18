
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// This class represents a sell order.
    /// </summary>
    public sealed class SellOrder : MarketOrder
    {
        /// <summary>
        /// Constructor from the API.
        /// </summary>
        /// <param name="src"></param>
        internal SellOrder(SerializableOrderListItem src)
            : base(src)
        {
        }

        /// <summary>
        /// Constructor from an object deserialized from the settings file.
        /// </summary>
        /// <param name="src"></param>
        internal SellOrder(SerializableSellOrder src)
            : base(src)
        {
        }

        /// <summary>
        /// Exports the given object to a serialization object.
        /// </summary>
        /// <returns></returns>
        public override SerializableOrderBase Export()
        {
            var serial = new SerializableSellOrder();
            Export(serial);
            return serial;
        }
    }
}
