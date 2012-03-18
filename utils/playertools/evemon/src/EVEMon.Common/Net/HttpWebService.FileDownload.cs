using System;
using System.IO;

namespace EVEMon.Common.Net
{
    public delegate void DownloadFileCompletedCallback(DownloadFileAsyncResult e);

    partial class HttpWebService
    {
        private const string FILE_ACCEPT = "*/*;q=0.5";

        /// <summary>
        /// Downloads an file from the specified url to the specified path
        /// </summary>
        /// <param name="url"></param>
        /// <param name="filePath"></param>
        /// <returns></returns>
        public FileInfo DownloadFile(string url, string filePath)
        {
            string urlValidationError;
            if (!IsValidURL(url, out urlValidationError))
                throw new ArgumentException(urlValidationError);
            HttpWebServiceRequest request = GetRequest();
            try
            {
                FileStream responseStream;
                try
                {
                    responseStream = new FileStream(filePath, FileMode.Create, FileAccess.Write, FileShare.None);
                }
                catch(Exception ex)
                {
                    throw HttpWebServiceException.FileError(url, ex);
                }
                request.GetResponse(url, responseStream, FILE_ACCEPT);
                return new FileInfo(filePath);
            }
            finally
            {
                if (request.ResponseStream != null) request.ResponseStream.Close();
            }
        }

        /// <summary>
        /// Asynchronously downloads file from the specified url to the specified path
        /// </summary>
        /// <param name="url"></param>
        /// <param name="filePath"></param>
        /// <param name="callback">A <see cref="DownloadImageCompletedCallback"/> to be invoked when the request is completed</param>
        /// <param name="progressCallback"></param>
        /// <returns></returns>
        public object DownloadFileAsync(string url, string filePath, DownloadFileCompletedCallback callback, DownloadProgressChangedCallback progressCallback)
        {
            string urlValidationError;
            if (!IsValidURL(url, out urlValidationError))
                throw new ArgumentException(urlValidationError);
            FileRequestAsyncState state = new FileRequestAsyncState(filePath, callback, progressCallback, DownloadFileAsyncCompleted);
            HttpWebServiceRequest request = GetRequest();
            request.GetResponseAsync(url, new FileStream(filePath, FileMode.Create, FileAccess.Write, FileShare.None), IMAGE_ACCEPT, null, state);
            return request;
        }

        /// <summary>
        /// Callback method for asynchronous requests
        /// </summary>
        private void DownloadFileAsyncCompleted(WebRequestAsyncState state)
        {
            FileRequestAsyncState requestState = (FileRequestAsyncState)state;
            FileInfo fileResult = null;
            if (!requestState.Request.Cancelled && requestState.Error == null)
            {
                try
                {
                    fileResult = new FileInfo(requestState.FilePath);
                }
                catch (Exception ex)
                {
                    requestState.Error = HttpWebServiceException.FileError(requestState.Request.BaseUrl, ex);
                }
            }
            if (requestState.Request.ResponseStream != null)
            {
                requestState.Request.ResponseStream.Close();
            }
            requestState.DownloadFileCompleted(new DownloadFileAsyncResult(fileResult, requestState.Error, requestState.Request.Cancelled));
        }

        /// <summary>
        /// Helper class to retain the original callback and return data for asynchronous requests
        /// </summary>
        private class FileRequestAsyncState : WebRequestAsyncState
        {
            private readonly DownloadFileCompletedCallback _downloadFileCompleted;
            private readonly string _filePath;

            public FileRequestAsyncState(string filepath, DownloadFileCompletedCallback callback, DownloadProgressChangedCallback progressCallback, WebRequestAsyncCallback webRequestCallback)
                : base(webRequestCallback, progressCallback)
            {
                _filePath = filepath;
                _downloadFileCompleted = callback;
            }

            public DownloadFileCompletedCallback DownloadFileCompleted
            {
                get { return _downloadFileCompleted; }
            }

            public string FilePath
            {
                get { return _filePath; }
            }
        }
    }
}
