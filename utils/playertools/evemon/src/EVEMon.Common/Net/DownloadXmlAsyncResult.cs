using System.Xml;

namespace EVEMon.Common.Net
{
    /// <summary>
    /// Container class to return the result of an asynchronous xml download
    /// </summary>
    public class DownloadXmlAsyncResult
    {
        private readonly XmlDocument _result;
        private readonly HttpWebServiceException _error;

        public DownloadXmlAsyncResult(XmlDocument xdoc, HttpWebServiceException error)
        {
            _error = error;
            _result = xdoc;
        }

        public XmlDocument Result
        {
            get { return _result; }
        }

        public HttpWebServiceException Error
        {
            get { return _error; }
        }

    }
}
