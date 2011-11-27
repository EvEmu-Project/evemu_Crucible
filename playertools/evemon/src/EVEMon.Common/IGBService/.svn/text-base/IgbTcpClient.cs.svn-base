using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;

using EVEMon.Common;

namespace EVEMon.Common.IgbService
{
    /// <summary>
    /// Manages a individual client connection
    /// </summary>
    public class IgbTcpClient
    {
        private readonly Object m_syncLock = new Object();

        private byte[] m_buffer;
        private int m_bufferSize = 4096;
        private bool m_running = false;
        private TcpClient m_client;
        private NetworkStream m_stream;

        public int BufferSize
        {
            get { return m_bufferSize; }
            set { m_bufferSize = value; }
        }

        #region Constructor and Close
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="client">Client information</param>
        public IgbTcpClient(TcpClient client)
        {
            m_client = client;
        }

        public void Close()
        {
            if (m_running)
            {
                m_running = false;
                try
                {
                    m_client.Close();
                }
                catch (SocketException e)
                {
                    ExceptionHandler.LogException(e, false);
                }
                OnClosed();
            }
        }

        private void OnClosed()
        {
            if (Closed != null)
            {
                Closed(this, new EventArgs());
            }
        }

        public event EventHandler<EventArgs> Closed;
        #endregion

        #region Reading
        /// <summary>
        /// Start reading from the client
        /// </summary>
        public void Start()
        {
            lock (m_syncLock)
            {
                m_running = true;
                m_stream = m_client.GetStream();
                m_buffer = new byte[m_bufferSize];
                BeginRead(false);
            }
        }

        /// <summary>
        /// Begin reading from the client
        /// </summary>
        /// <param name="acquireLock">lock the object</param>
        private void BeginRead(bool acquireLock)
        {
            if (acquireLock)
            {
                Monitor.Enter(m_syncLock);
            }
            try
            {
                IAsyncResult ar;
                do
                {
                    ar = null;
                    if (m_running)
                    {
                        ar = m_stream.BeginRead(m_buffer, 0, m_buffer.Length, new AsyncCallback(EndRead), null);
                    }
                } while (ar != null && ar.CompletedSynchronously);
            }
            finally
            {
                if (acquireLock)
                {
                    Monitor.Exit(m_syncLock);
                }
            }
        }

        /// <summary>
        /// Async called when reading has finished
        /// </summary>
        /// <param name="ar">result</param>
        private void EndRead(IAsyncResult ar)
        {
            try
            {
                int bytesRead = m_stream.EndRead(ar);
                if (bytesRead <= 0)
                {
                    Close();
                }
                else
                {
                    OnDataRead(m_buffer, 0, bytesRead);
                    if (!ar.CompletedSynchronously)
                    {
                        BeginRead(true);
                    }
                }
            }
            catch (Exception e)
            {
                ExceptionHandler.LogException(e, true);
                Close();
            }
        }

        /// <summary>
        /// Event triggered on data read
        /// </summary>
        /// <param name="buffer">buffer</param>
        /// <param name="offset">offset</param>
        /// <param name="count">bytes read</param>
        private void OnDataRead(byte[] buffer, int offset, int count)
        {
            if (DataRead != null)
            {
                DataRead(this, new IgbClientDataReadEventArgs(buffer, offset, count));
            }
        }

        public event EventHandler<IgbClientDataReadEventArgs> DataRead;
        #endregion

        #region Writing
        public void Write(string str)
        {
            byte[] outbuf = Encoding.UTF8.GetBytes(str);
            m_stream.Write(outbuf, 0, outbuf.Length);
        }
        #endregion
    }
}