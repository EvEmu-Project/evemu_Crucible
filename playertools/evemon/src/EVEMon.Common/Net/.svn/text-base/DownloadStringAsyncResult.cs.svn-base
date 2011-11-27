namespace EVEMon.Common.Net
{
    /// <summary>
    /// Container class to return the result of an asynchronous string download
    /// </summary>
    public class DownloadStringAsyncResult
    {
        private readonly string _result;
        private readonly HttpWebServiceException _error;

        public DownloadStringAsyncResult(string result, HttpWebServiceException error)
        {
            _error = error;
            _result = result;
        }

        public string Result
        {
            get { return _result; }
        }

        public HttpWebServiceException Error
        {
            get { return _error; }
        }

    }
}
