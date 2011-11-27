using System.IO;

namespace EVEMon.Common.Net
{
    /// <summary>
    /// Container class to return the result of an asynchronous file download
    /// </summary>
    public class DownloadFileAsyncResult
    {
        private readonly FileInfo _result;
        private readonly HttpWebServiceException _error;
        private readonly bool _cancelled;

        public DownloadFileAsyncResult(FileInfo file, HttpWebServiceException error, bool cancelled)
        {
            _error = error;
            _cancelled = cancelled;
            _result = file;
        }

        public FileInfo Result
        {
            get { return _result; }
        }

        public HttpWebServiceException Error
        {
            get { return _error; }
        }

        public bool Cancelled
        {
            get { return _cancelled; }
        }
    }
}
