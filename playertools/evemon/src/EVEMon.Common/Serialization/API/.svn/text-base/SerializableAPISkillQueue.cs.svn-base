using System;
using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of the training queue.  Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPISkillQueue : ISynchronizableWithLocalClock
    {
        [XmlArray("queue")]
        [XmlArrayItem("skill")]
        public List<SerializableQueuedSkill> Queue
        {
            get;
            set;
        }

        #region ISynchronizableWithLocalClock Members
        /// <summary>
        /// Synchronizes the stored times with local clock
        /// </summary>
        /// <param name="drift"></param>
        void ISynchronizableWithLocalClock.SynchronizeWithLocalClock(TimeSpan drift)
        {
            foreach (ISynchronizableWithLocalClock synch in Queue)
            {
                synch.SynchronizeWithLocalClock(drift);
            }
        }
        #endregion
    }
}
