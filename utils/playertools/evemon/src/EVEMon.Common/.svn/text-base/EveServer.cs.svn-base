using System;

using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents the status of a server
    /// </summary>
    public sealed class EveServer
    {
        private int m_users;
        private ServerStatus m_status;
        private QueryMonitor<SerializableAPIServerStatus> m_monitor;

        /// <summary>
        /// Constructor.
        /// </summary>
        internal EveServer()
        {
            m_status = ServerStatus.Online;

            m_monitor = new QueryMonitor<SerializableAPIServerStatus>(APIMethods.ServerStatus);
            m_monitor.Updated += OnMonitorUpdated;
        }

        /// <summary>
        /// Gets the number of users online at the last update.
        /// </summary>
        public int Users 
        { 
            get { return m_users; } 
        }

        /// <summary>
        /// Gets the server's status.
        /// </summary>
        public ServerStatus Status
        {
            get { return m_status; }
        }

        /// <summary>
        /// Gets the server's name.
        /// </summary>
        public string Name
        {
            get 
            { 
                return (EveClient.APIProviders.CurrentProvider != APIProvider.TestProvider ?
                        "Tranquility" : "Sinqularity");
            }
        }

        /// <summary>
        /// Gets the server status message.
        /// </summary>
        public string StatusText
        {
            get
            {               
                switch(m_status)
                {
                    case ServerStatus.Online:
                        return String.Format(CultureConstants.TidyInteger,
                            "{0} Server Online ({1:n} Pilots)", Name, m_users);
                    case ServerStatus.Offline:
                        return String.Format("{0} Server Offline", Name);
                    case ServerStatus.CheckDisabled:
                        return "Server Status Check Disabled";
                    default:
                        return String.Format("{0} Server Status Unknown", Name);
                }
            }
        }

        /// <summary>
        /// Update on a time tick.
        /// </summary>
        public void UpdateOnOneSecondTick()
        {
            m_monitor.UpdateOnOneSecondTick();
        }

        /// <summary>
        /// Forces an update of the server status.
        /// </summary>
        public void ForceUpdate()
        {
            m_monitor.ForceUpdate(false);
        }

        /// <summary>
        /// Occurs when CCP returns new data.
        /// </summary>
        /// <param name="result"></param>
        private void OnMonitorUpdated(APIResult<SerializableAPIServerStatus> result)
        {
            // Was there an error ?
            var lastStatus = m_status;
            if (result.HasError)
            {
                // Checks if EVE Backend Database is temporarily disabled
                if (result.EVEBackendDatabaseDisabled)
                    return;

                m_status = ServerStatus.Unknown;
                EveClient.Notifications.NotifyServerStatusError(result);

                // Notify subscribers about update
                EveClient.OnServerStatusUpdated(this, lastStatus, m_status);
                return;
            }

            // Update status and users, notify no more errors
            m_users = result.Result.Players;
            m_status = (result.Result.Open ? ServerStatus.Online : ServerStatus.Offline);
            EveClient.Notifications.InvalidateServerStatusError();

            // Notify subscribers about update
            EveClient.OnServerStatusUpdated(this, lastStatus, m_status);

            // Send a notification
            if (lastStatus != m_status)
            {
                EveClient.Notifications.NotifyServerStatusChanged(Name, m_status);
                return;
            }

            EveClient.Notifications.InvalidateServerStatusChange();
        }
    }
}
