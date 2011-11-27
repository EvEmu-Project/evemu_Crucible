using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a characters' eve mailing lists. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPIMailingLists
    {
        [XmlArray("mailingLists")]
        [XmlArrayItem("mailingList")]
        public List<SerializableMailingListsListItem> MailingLists
        {
            get;
            set;
        }
    }
}
