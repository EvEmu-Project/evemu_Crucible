using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// A collection of wallets balances.
    /// </summary>
    public sealed class SerializableAPIAccountBalance
    {
        /// <summary>
        /// Gets the list of balance accounts for every account on this account (one for character balance, one per division for corporations).
        /// </summary>
        [XmlArray("accounts")]
        [XmlArrayItem("account")]
        public SerializableAccountBalanceListItem[] Accounts
        {
            get;
            set;
        }
    }
}
