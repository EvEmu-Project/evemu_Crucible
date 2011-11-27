using System;
using System.IO;
using System.Xml;

namespace EVEMon.Common.Net
{
    public delegate void DownloadXmlCompletedCallback(DownloadXmlAsyncResult e, object userState);

    /// <summary>
    /// HttpWebService Xml download implementation
    /// </summary>
    partial class HttpWebService
    {
        private const string XML_ACCEPT =
            "text/xml,application/xml,application/xhtml+xml;q=0.8,*/*;q=0.5";

        /// <summary>
        /// Downloads an Xml document from the specified url
        /// </summary>
        /// <param name="url"></param>
        /// <returns></returns>
        public XmlDocument DownloadXml(string url)
        {
            return DownloadXml(url, null);
        }

        /// <summary>
        /// Downloads an Xml document from the specified url using the specified POST data
        /// </summary>
        /// <param name="url"></param>
        /// <param name="postData"></param>
        /// <returns></returns>
        public XmlDocument DownloadXml(string url, HttpPostData postData)
        {
            string urlValidationError;
            if (!IsValidURL(url, out urlValidationError))
                throw new ArgumentException(urlValidationError);
            HttpWebServiceRequest request = GetRequest();
            try
            {
                request.GetResponse(url, new MemoryStream(), XML_ACCEPT, postData);
                XmlDocument result = new XmlDocument();
                if (request.ResponseStream != null)
                {
                    request.ResponseStream.Seek(0, SeekOrigin.Begin);
                    result.Load(request.ResponseStream);
                }
                return result;
            }
            finally
            {
                if (request.ResponseStream != null)
                    request.ResponseStream.Close();
            }
        }

        /// <summary>
        /// Asynchronously downloads an xml file from the specified url
        /// </summary>
        /// <param name="url"></param>
        /// <param name="callback">A <see cref="DownloadXmlCompletedCallback"/> to be invoked when the request is completed</param>
        /// <param name="userState">A state object to be returned to the callback</param>
        /// <returns></returns>
        public object DownloadXmlAsync(string url, HttpPostData postData, DownloadXmlCompletedCallback callback, object userState)
        {
            string urlValidationError;
            if (!IsValidURL(url, out urlValidationError))
            {
                throw new ArgumentException(urlValidationError);
            }

            XmlRequestAsyncState state = new XmlRequestAsyncState(callback, DownloadXmlAsyncCompleted, userState);
            HttpWebServiceRequest request = GetRequest();
            request.GetResponseAsync(url, new MemoryStream(), XML_ACCEPT, postData, state);
            return request;
        }

        /// <summary>
        /// Callback method for asynchronous requests
        /// </summary>
        private void DownloadXmlAsyncCompleted(WebRequestAsyncState state)
        {
            XmlRequestAsyncState requestState = (XmlRequestAsyncState)state;
            XmlDocument xdocResult = new XmlDocument();
            if (!requestState.Request.Cancelled && requestState.Error == null && requestState.Request.ResponseStream != null)
            {
                try
                {
                    requestState.Request.ResponseStream.Seek(0, SeekOrigin.Begin);
                    xdocResult.Load(requestState.Request.ResponseStream);
                }
                catch (XmlException ex)
                {
                    requestState.Error = HttpWebServiceException.XmlException(requestState.Request.BaseUrl, ex);
                }
            }
            if (requestState.Request.ResponseStream != null)
            {
                requestState.Request.ResponseStream.Close();
            }
            requestState.DownloadXmlCompleted(new DownloadXmlAsyncResult(xdocResult, requestState.Error), requestState.UserState);
        }

        /// <summary>
        /// Helper class to retain the original callback and return data for asynchronous requests
        /// </summary>
        private class XmlRequestAsyncState : WebRequestAsyncState
        {
            private readonly DownloadXmlCompletedCallback _downloadXmlCompleted;
            private readonly object _userState;

            public XmlRequestAsyncState(DownloadXmlCompletedCallback callback, WebRequestAsyncCallback webRequestCallback, object userState)
                : base(webRequestCallback)
            {
                _downloadXmlCompleted = callback;
                _userState = userState;
            }

            public DownloadXmlCompletedCallback DownloadXmlCompleted
            {
                get { return _downloadXmlCompleted; }
            }

            public object UserState
            {
                get { return _userState; }
            }
        }
    }
}
