using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    public sealed class SerializableJobListItem
    {
        [XmlAttribute("jobID")]
        public long JobID
        {
            get;
            set;
        }

        [XmlAttribute("installedItemLocationID")]
        public long InstalledItemLocationID
        {
            get;
            set;
        }

        [XmlAttribute("outputLocationID")]
        public long OutputLocationID
        {
            get;
            set;
        }

        [XmlAttribute("installedInSolarSystemID")]
        public long SolarSystemID
        {
            get;
            set;
        }

        [XmlAttribute("installedItemProductivityLevel")]
        public int InstalledItemProductivityLevel
        {
            get;
            set;
        }

        [XmlAttribute("installedItemMaterialLevel")]
        public int InstalledItemMaterialLevel
        {
            get;
            set;
        }

        [XmlAttribute("installerID")]
        public int InstallerID
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

        [XmlAttribute("installedItemTypeID")]
        public long InstalledItemTypeID
        {
            get;
            set;
        }

        [XmlAttribute("outputTypeID")]
        public long OutputTypeID
        {
            get;
            set;
        }

        [XmlAttribute("installedItemCopy")]
        public int InstalledItemCopy
        {
            get;
            set;
        }

        [XmlAttribute("completed")]
        public int Completed
        {
            get;
            set;
        }

        [XmlAttribute("activityID")]
        public int ActivityID
        {
            get;
            set;
        }

        [XmlAttribute("completedStatus")]
        public int CompletedStatus
        {
            get;
            set;
        }

        [XmlAttribute("installTime")]
        public string InstallTimeXml
        {
            get { return InstallTime.DateTimeToTimeString(); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    InstallTime = value.TimeStringToDateTime();
            }
        }

        [XmlAttribute("beginProductionTime")]
        public string BeginProductionTimeXml
        {
            get { return BeginProductionTime.DateTimeToTimeString(); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    BeginProductionTime = value.TimeStringToDateTime();
            }
        }

        [XmlAttribute("endProductionTime")]
        public string EndProductionTimeXml
        {
            get { return EndProductionTime.DateTimeToTimeString(); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    EndProductionTime = value.TimeStringToDateTime();
            }
        }

        [XmlAttribute("pauseProductionTime")]
        public string PauseProductionTimeXml
        {
            get { return PauseProductionTime.DateTimeToTimeString(); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    PauseProductionTime = value.TimeStringToDateTime();
            }
        }

        /// <summary>
        /// The time this job was installed.
        /// </summary>
        [XmlIgnore]
        public DateTime InstallTime
        {
            get;
            set;
        }

        /// <summary>
        /// The time this job began.
        /// </summary>
        [XmlIgnore]
        public DateTime BeginProductionTime
        {
            get;
            set;
        }

        /// <summary>
        /// The time this job will finish.
        /// </summary>
        [XmlIgnore]
        public DateTime EndProductionTime
        {
            get;
            set;
        }

        /// <summary>
        /// The time this job was paused.
        /// </summary>
        [XmlIgnore]
        public DateTime PauseProductionTime
        {
            get;
            set;
        }

        /// <summary>
        /// Which this job was issued for.
        /// </summary>
        [XmlIgnore]
        public IssuedFor IssuedFor
        {
            get;
            set;
        }
    }
}
