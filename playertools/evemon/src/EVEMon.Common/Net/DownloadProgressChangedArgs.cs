namespace EVEMon.Common.Net
{
    /// <summary>
    /// Event args class to return download progress information
    /// </summary>
    public class DownloadProgressChangedArgs
    {
        private readonly long _totalBytesToReceive;
        private readonly long _bytesReceived;
        private readonly int _progressPercentage;

        internal DownloadProgressChangedArgs(long totalBytesToReceive, long bytesReceived, int progressPercentage)
        {
            _totalBytesToReceive = totalBytesToReceive;
            _progressPercentage = progressPercentage;
            _bytesReceived = bytesReceived;
        }

        public long TotalBytesToReceive
        {
            get { return _totalBytesToReceive; }
        }

        public long BytesReceived
        {
            get { return _bytesReceived; }
        }

        public int ProgressPercentage
        {
            get { return _progressPercentage; }
        }
    }
}
