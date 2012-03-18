using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents the balance of a wallet
    /// </summary>
    public sealed class SerializableAccountBalanceListItem
    {
        [XmlAttribute("accountID")]
        public long AccountID
        {
            get;
            set;
        }

        [XmlAttribute("accountKey")]
        public long AccountKey
        {
            get;
            set;
        }

        [XmlAttribute("balance")]
        public decimal Balance
        {
            get;
            set;
        }
    }
}
