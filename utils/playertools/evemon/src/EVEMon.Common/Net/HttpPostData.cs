using System.Text;

namespace EVEMon.Common.Net
{
    /// <summary>
    /// Container class for data to be submitted to a url as a POST request
    /// </summary>
    public sealed class HttpPostData
    {
        private readonly byte[] m_content;
        private readonly string m_data;

        public HttpPostData(string data)
        {
            ASCIIEncoding encoding = new ASCIIEncoding();
            m_content = encoding.GetBytes(data);
            m_data = data;
        }

        /// <summary>
        /// Gets the content's bytes
        /// </summary>
        public byte[] Content
        {
            get { return m_content; }
        }

        /// <summary>
        /// Gets the number of bytes of the content
        /// </summary>
        public int Length
        {
            get { return m_content.Length; }
        }

        public override string ToString()
        {
            return m_data;
        }
    }
}
