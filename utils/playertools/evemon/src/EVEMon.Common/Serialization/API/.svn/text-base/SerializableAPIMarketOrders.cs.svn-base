using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a characters' market orders. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPIMarketOrders
    {
        [XmlArray("orders")]
        [XmlArrayItem("order")]
        public List<SerializableOrderListItem> Orders
        {
            get;
            set;
        }
    }
}
