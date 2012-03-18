using System.Reflection;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common.Net
{
    /// <summary>
    /// Conainer class for HttpWebService settings and state
    /// </summary>
    public class HttpWebServiceState
    {
        private const int MAX_BUFFER_SIZE = 8192;
        private const int MIN_BUFFER_SIZE = 1024;
        private const int MAX_REDIRECTS = 5;
        private readonly object m_syncLock = new object();
        private readonly string m_userAgent = "EVEMon/" + Assembly.GetExecutingAssembly().GetName().Version;
        private ProxySettings m_proxy = new ProxySettings();

        internal HttpWebServiceState()
        {
        }

        /// <summary>
        /// The maximum size of a download section
        /// </summary>
        public int MaxBufferSize
        {
            get { return MAX_BUFFER_SIZE; }
        }

        /// <summary>
        /// The minimum size if a download section
        /// </summary>
        public int MinBufferSize
        {
            get { return MIN_BUFFER_SIZE; }
        }

        /// <summary>
        /// The user agent string for requests
        /// </summary>
        public string UserAgent
        {
            get { return m_userAgent; }
        }

        /// <summary>
        /// The maximum redirects allowed for a request
        /// </summary>
        public int MaxRedirects
        {
            get { return MAX_REDIRECTS; }
        }

        /// <summary>
        /// A ProxySetting instance for the custom proxy to be used
        /// </summary>
        public ProxySettings Proxy
        {
            get { lock(m_syncLock) {return m_proxy;} }
            set
            {
                 lock(m_syncLock)
                 {
                     m_proxy = value.Clone();
                 }
             }
        }
    }
}
