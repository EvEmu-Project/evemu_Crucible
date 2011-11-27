using System;
using System.Net.Sockets;

namespace EVEMon.Common.IgbService
{
    /// <summary>
    /// Event arguments triggered on client connect
    /// </summary>
    public class ClientConnectedEventArgs : EventArgs
    {
        private TcpClient m_client;

        public ClientConnectedEventArgs(TcpClient client)
        {
            m_client = client;
        }

        public TcpClient TcpClient
        {
            get { return m_client; }
        }
    }
}