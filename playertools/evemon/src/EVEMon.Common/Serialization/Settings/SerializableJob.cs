using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents an industry job.
    /// </summary>
    public class SerializableJob
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
        /// Unique job ID for this job. Note that these are not guaranteed to be unique forever, they can recycle. 
        /// But they are unique for the purpose of one data pull. 
        /// </summary>
        [XmlAttribute("jobID")]
        public long JobID
        {
            get;
            set;
        }

        [XmlAttribute("installedItemID")]
        public long InstalledItemID
        {
            get;
            set;
        }

        [XmlAttribute("installedItem")]
        public string InstalledItem
        {
            get;
            set;
        }

        [XmlAttribute("outputItemID")]
        public long OutputItemID
        {
            get;
            set;
        }

        [XmlAttribute("outputItem")]
        public string OutputItem
        {
            get;
            set;
        }

        [XmlAttribute("blueprintType")]
        public BlueprintType BlueprintType
        {
            get;
            set;
        }

        [XmlAttribute("runs")]
        public int Runs
        {
            get;
            set;
        }

        [XmlAttribute("installedItemME")]
        public int InstalledItemME
        {
            get;
            set;
        }


        [XmlAttribute("installedItemPE")]
        public int InstalledItemPE
        {
            get;
            set;
        }

        [XmlAttribute("activity")]
        public BlueprintActivity Activity
        {
            get;
            set;
        }

        [XmlAttribute("jobState")]
        public JobState State
        {
            get;
            set;
        }

        [XmlAttribute("installedItemLocation")]
        public string ItemLocation
        {
            get;
            set;
        }

        [XmlAttribute("installedSolarSystemID")]
        public long SolarSystemID
        {
            get;
            set;
        }

        [XmlAttribute("installedTime")]
        public DateTime InstalledTime
        {
            get;
            set;
        }

        [XmlAttribute("beginProductionTime")]
        public DateTime BeginProductionTime
        {
            get;
            set;
        }

        [XmlAttribute("endProductionTime")]
        public DateTime EndProductionTime
        {
            get;
            set;
        }

        [XmlAttribute("pauseProductionTime")]
        public DateTime PauseProductionTime
        {
            get;
            set;
        }

        /// <summary>
        /// Which this job was issued for.
        /// </summary>
        [XmlAttribute("issuedFor")]
        public IssuedFor IssuedFor
        {
            get;
            set;
        }

        /// <summary>
        /// The time this job state was changed.
        /// </summary>
        [XmlAttribute("lastStateChange")]
        public DateTime LastStateChange
        {
            get;
            set;
        }
    }
}
