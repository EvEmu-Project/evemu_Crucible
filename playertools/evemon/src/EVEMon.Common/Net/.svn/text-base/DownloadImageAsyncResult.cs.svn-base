using System.Drawing;

namespace EVEMon.Common.Net
{
    /// <summary>
    /// Container class to return the result of an asynchronous image download
    /// </summary>
    public class DownloadImageAsyncResult
    {
        private readonly Image _result;
        private readonly HttpWebServiceException _error;

        public DownloadImageAsyncResult(Image image, HttpWebServiceException error)
        {
            _error = error;
            _result = image;
        }

        public Image Result
        {
            get { return _result; }
        }

        public HttpWebServiceException Error
        {
            get { return _error; }
        }

    }
}
