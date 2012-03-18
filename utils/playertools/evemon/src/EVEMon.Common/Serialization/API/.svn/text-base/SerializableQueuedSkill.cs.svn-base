using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    public sealed class SerializableQueuedSkill : ISynchronizableWithLocalClock
    {
        [XmlAttribute("typeID")]
        public long ID
        {
            get;
            set;
        }

        [XmlAttribute("level")]
        public int Level
        {
            get;
            set;
        }

        [XmlAttribute("startSP")]
        public int StartSP
        {
            get;
            set;
        }

        [XmlAttribute("endSP")]
        public int EndSP
        {
            get;
            set;
        }

        [XmlAttribute("startTime")]
        public string CCPStartTime
        {
            get;
            set;
        }

        [XmlAttribute("endTime")]
        public string CCPEndTime
        {
            get;
            set;
        }

        [XmlIgnore]
        public DateTime StartTime
        {
            get { return CCPStartTime.TimeStringToDateTime(); }
            set { CCPStartTime = value.DateTimeToTimeString(); }
        }

        [XmlIgnore]
        public DateTime EndTime
        {
            get { return CCPEndTime.TimeStringToDateTime(); }
            set { CCPEndTime = value.DateTimeToTimeString(); }
        }

        #region ISynchronizableWithLocalClock Members
        /// <summary>
        /// Synchronizes the stored times with local clock
        /// </summary>
        /// <param name="drift"></param>
        void ISynchronizableWithLocalClock.SynchronizeWithLocalClock(TimeSpan drift)
        {
            if (!String.IsNullOrEmpty(CCPStartTime))
                StartTime -= drift;

            if (!String.IsNullOrEmpty(CCPEndTime))
                EndTime -= drift;
        }
        #endregion
    }
}
