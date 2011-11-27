using System;
using System.Drawing;
using System.IO;

namespace EVEMon.Common.Net
{
    public delegate void DownloadImageCompletedCallback(DownloadImageAsyncResult e, object userState);

    /// <summary>
    /// HttpWebService Image download implementation
    /// </summary>
    partial class HttpWebService
    {
        private const string IMAGE_ACCEPT = "image/png,*/*;q=0.5";

        /// <summary>
        /// Downloads an image from the specified url
        /// </summary>
        /// <param name="url"></param>
        /// <returns></returns>
        public Image DownloadImage(string url)
        {
            string urlValidationError;
            if (!IsValidURL(url, out urlValidationError))
                throw new ArgumentException(urlValidationError);
            HttpWebServiceRequest request = GetRequest();
            try
            {
                request.GetResponse(url, new MemoryStream(), IMAGE_ACCEPT);
                return GetImage(request);
            }
            catch (Exception)
            {
                if (request.ResponseStream != null) request.ResponseStream.Close();
                throw;
            }
        }

        /// <summary>
        /// Asynchronously downloads an image from the specified url
        /// </summary>
        /// <param name="url"></param>
        /// <param name="callback">A <see cref="DownloadImageCompletedCallback"/> to be invoked when the request is completed</param>
        /// <param name="userState">A state object to be returned to the callback</param>
        /// <returns></returns>
        public object DownloadImageAsync(string url, DownloadImageCompletedCallback callback, object userState)
        {
            string urlValidationError;
            if (!IsValidURL(url, out urlValidationError))
                throw new ArgumentException(urlValidationError);
            ImageRequestAsyncState state = new ImageRequestAsyncState(callback, DownloadImageAsyncCompleted, userState);
            HttpWebServiceRequest request = GetRequest();
            request.GetResponseAsync(url, new MemoryStream(), IMAGE_ACCEPT, null, state);
            return request;
        }

        /// <summary>
        /// Callback method for asynchronous requests
        /// </summary>
        private void DownloadImageAsyncCompleted(WebRequestAsyncState state)
        {
            ImageRequestAsyncState requestState = (ImageRequestAsyncState)state;
            Image imageResult = null;
            try
            {
                if (!requestState.Request.Cancelled && requestState.Error == null)
                {
                    imageResult = GetImage(requestState.Request);
                }
            }
            catch(HttpWebServiceException ex)
            {
                requestState.Error = ex;
                if (requestState.Request.ResponseStream != null)
                {
                    requestState.Request.ResponseStream.Close();
                }
            }
            requestState.DownloadImageCompleted(new DownloadImageAsyncResult(imageResult, requestState.Error), requestState.UserState);
        }

        /// <summary>
        /// Helper method to return an Image from the completed request
        /// </summary>
        private Image GetImage(HttpWebServiceRequest request)
        {
            Image result = null;
            if (request.ResponseStream != null)
            {
                request.ResponseStream.Seek(0, SeekOrigin.Begin);
                try
                {
                    result = Image.FromStream(request.ResponseStream, true);
                }
                catch (ArgumentException ex)
                {
                    throw HttpWebServiceException.ImageException(request.BaseUrl, ex);
                }
            }
            return result;
        }

        /// <summary>
        /// Helper class to retain the original callback and return data for asynchronous requests
        /// </summary>
        private class ImageRequestAsyncState : WebRequestAsyncState
        {
            private readonly DownloadImageCompletedCallback _downloadImageCompleted;
            private readonly object _userState;

            public ImageRequestAsyncState(DownloadImageCompletedCallback callback, WebRequestAsyncCallback webRequestCallback, object userState)
                :base(webRequestCallback)
            {
                _downloadImageCompleted = callback;
                _userState = userState;
            }

            public DownloadImageCompletedCallback DownloadImageCompleted
            {
                get { return _downloadImageCompleted; }
            }

            public object UserState
            {
                get { return _userState; }
            }
        }
    }
}
