using System.Collections.Generic;
using System.Xml.Serialization;

using EVEMon.Common.Serialization.API;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents a serializable character defined from the API
    /// </summary>
    public sealed class SerializableCCPCharacter : SerializableSettingsCharacter
    {
        public SerializableCCPCharacter()
        {
            SkillQueue = new List<SerializableQueuedSkill>();
            LastUpdates = new List<SerializableAPIUpdate>();
            MarketOrders = new List<SerializableOrderBase>();
            IndustryJobs = new List<SerializableJob>();
            ResearchPoints = new List<SerializableResearchPoint>();
        }

        [XmlArray("queue")]
        [XmlArrayItem("skill")]
        public List<SerializableQueuedSkill> SkillQueue
        {
            get;
            set;
        }

        [XmlArray("marketOrders")]
        [XmlArrayItem("buy", typeof(SerializableBuyOrder))]
        [XmlArrayItem("sell", typeof(SerializableSellOrder))]
        public List<SerializableOrderBase> MarketOrders
        {
            get;
            set;
        }

        [XmlArray("industryJobs")]
        [XmlArrayItem("job")]
        public List<SerializableJob> IndustryJobs
        {
            get;
            set;
        }

        [XmlArray("researchPoints")]
        [XmlArrayItem("researchPoint")]
        public List<SerializableResearchPoint> ResearchPoints
        {
            get;
            set;
        }

        [XmlElement("eveMailMessages")]
        public string EveMailMessagesIDs
        {
            get;
            set;
        }

        [XmlElement("eveNotifications")]
        public string EveNotificationsIDs
        {
            get;
            set;
        }

        [XmlArray("lastUpdates")]
        [XmlArrayItem("apiUpdate")]
        public List<SerializableAPIUpdate> LastUpdates
        {
            get;
            set;
        }
    }
}
