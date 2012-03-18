using System;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents an argument for server changes
    /// </summary>
    public sealed class EveServerEventArgs : EventArgs
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="server"></param>
        /// <param name="previousStatus"></param>
        /// <param name="status"></param>
        public EveServerEventArgs(EveServer server, ServerStatus previousStatus, ServerStatus status)
        {
            Server = server;
            Status = status;
            PreviousStatus = previousStatus;
        }

        /// <summary>
        /// Gets or sets the updated server
        /// </summary>
        public EveServer Server { get; private set; }

        /// <summary>
        /// Gets or sets the current status
        /// </summary>
        public ServerStatus Status { get; private set; }

        /// <summary>
        /// Gets or sets the previous status
        /// </summary>
        public ServerStatus PreviousStatus { get; private set; }
    }
}
