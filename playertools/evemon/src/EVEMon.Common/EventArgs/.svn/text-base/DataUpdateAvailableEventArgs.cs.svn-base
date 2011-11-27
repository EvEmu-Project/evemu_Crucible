using System;
using System.Collections.Generic;
using EVEMon.Common.Serialization.BattleClinic;

namespace EVEMon.Common
{
    public sealed class DataUpdateAvailableEventArgs : EventArgs
    {
        public DataUpdateAvailableEventArgs(string updateUrl, List<SerializableDatafile> changedFiles)
        {
            UpdateUrl = updateUrl;
            ChangedFiles = changedFiles;
        }

        /// <summary>
        /// Gets or sets the update URL.
        /// </summary>
        /// <value>The update URL.</value>
        public string UpdateUrl { get; private set; }

        /// <summary>
        /// Gets or sets the changed files.
        /// </summary>
        /// <value>The changed files.</value>
        public List<SerializableDatafile> ChangedFiles { get; private set; }
    }
}
