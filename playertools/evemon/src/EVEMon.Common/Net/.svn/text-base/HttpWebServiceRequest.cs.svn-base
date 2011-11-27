using System;
using System.IO;
using System.Net;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common.Threading;

namespace EVEMon.Common.Net
{
    /// <summary>
    /// The core class that retrieves data from the web via HTTP. Requests are carried out by the GetResponse methods. The content of
    /// the response is returned via the stream parameter passed to GetResponse
    /// </summary>
    internal class HttpWebServiceRequest
    {
        private int Timeout;

        private readonly object m_syncLock = new object();

        private readonly HttpWebServiceState m_webServiceState;
        private WebRequestAsyncState m_asyncState;
        private HttpPostData m_postData;
        private Stream m_responseStream;

        private string m_baseUrl;
        private string m_accept;
        private string m_url;
        private string m_referer = String.Empty;

        private int m_redirectsRemaining;
        private bool m_cancelled = false;

        /// <summary>
        /// Initialises a new instance of HttpWebServiceRequest to be submitted as a POST request.
        /// </summary>
        /// <param name="webServiceState">An <see cref="HttpWebServiceState"/> instance</param>
        internal HttpWebServiceRequest(HttpWebServiceState webServiceState)
        {
            m_webServiceState = webServiceState;
            m_redirectsRemaining = m_webServiceState.MaxRedirects;

            // Pull the timeout from the settings
            TimeSpan timeoutSetting = TimeSpan.FromSeconds(Settings.Updates.HttpTimeout);

            if (timeoutSetting < TimeSpan.FromSeconds(1) || timeoutSetting > TimeSpan.FromMinutes(5))
            {
                Timeout = 10000;
            }
            else
            {
                Timeout = (int)timeoutSetting.TotalMilliseconds;
            }
        }

        /// <summary>
        /// The <see cref="Stream"/> to which the reponse is written.
        /// </summary>
        internal Stream ResponseStream
        {
            get { return m_responseStream; }
        }

        /// <summary>
        /// The original url for the request.
        /// </summary>
        public string BaseUrl
        {
            get { return m_baseUrl; }
        }

        /// <summary>
        /// Returns true if an asynchronous request was cancelled. When set to true, cancels the current asynchronous request.
        /// </summary>
        public bool Cancelled
        {
            get
            {
                lock(m_syncLock)
                    return m_cancelled;
            }
            set
            { 
                lock(m_syncLock)
                    m_cancelled = value;
            }
        }

        /// <summary>
        /// Retrieve the response from the requested URL to the specified response stream as a GET request.
        /// </summary>
        internal void GetResponse(string url, Stream responseStream, string accept)
        {
             GetResponse(url, responseStream, accept, null);
        }

        /// <summary>
        /// Delegate for asynchronous invocation of GetResponse.
        /// </summary>
        private delegate void GetResponseDelegate(string url, Stream responseStream, string accept, HttpPostData postData);

        /// <summary>
        /// Retrieve the response from the reguested URL to the specified response stream
        /// If postData is supplied, the request is submitted as a POST request, otherwise it is submitted as a GET request
        /// The download process is broken into chunks for future implementation of asynchronous requests
        /// </summary>
        internal void GetResponse(string url, Stream responseStream, string accept, HttpPostData postData)
        {
            // Store params
            m_url = url;
            m_baseUrl = url;
            m_accept = accept;
            m_postData = postData;
            m_responseStream = responseStream;

            Stream webResponseStream = null;
            HttpWebResponse webResponse = null;
            try
            {
                webResponse = GetHttpResponse();
                webResponseStream = webResponse.GetResponseStream();
                int bytesRead;
                long totalBytesRead = 0;
                long rawBufferSize = webResponse.ContentLength / 100;
                int bufferSize = (int)(rawBufferSize > m_webServiceState.MaxBufferSize  ? m_webServiceState.MaxBufferSize : (rawBufferSize < m_webServiceState.MinBufferSize ? m_webServiceState.MinBufferSize : rawBufferSize));
                do
                {
                    byte[] buffer = new byte[bufferSize];
                    bytesRead = webResponseStream.Read(buffer, 0, bufferSize);
                    if (bytesRead > 0)
                    {
                        m_responseStream.Write(buffer, 0, bytesRead);
                        if (m_asyncState != null && m_asyncState.ProgressCallback != null)
                        {
                            totalBytesRead += bytesRead;
                            int progressPercentage = webResponse.ContentLength == 0 ? 0 : (int)((totalBytesRead * 100)/webResponse.ContentLength);
                            m_asyncState.ProgressCallback(new DownloadProgressChangedArgs(webResponse.ContentLength, totalBytesRead, progressPercentage));
                        }
                    }
                } while (bytesRead > 0 && !Cancelled);
            }
            catch (HttpWebServiceException)
            {
                throw;
            }
            catch (WebException ex)
            {
                // Aborted, time out or error while processing the request
                throw HttpWebServiceException.WebException(BaseUrl, m_webServiceState, ex);
            }
            catch (Exception ex)
            {
                throw HttpWebServiceException.Exception(url, ex);
            }
            finally
            {
                if (webResponseStream != null)
                    webResponseStream.Close();
                if (webResponse != null)
                    webResponse.Close();
            }
        }

        /// <summary>
        /// Asynchronously retrieve the response from the requested url to the specified response stream.
        /// </summary>
        public void GetResponseAsync(string url, Stream responseStream, string accept, HttpPostData postData, WebRequestAsyncState state)
        {
            m_asyncState = state;
            m_asyncState.Request = this;
            if (Dispatcher.IsMultiThreaded)
            {
                GetResponseDelegate caller = GetResponse;
                caller.BeginInvoke(url, responseStream, accept, postData, GetResponseAsyncCompleted, caller);
            }
            else
            {
                GetResponseAsyncCompletedCore(() => GetResponse(url, responseStream, accept, postData));
            }
        }

        /// <summary>
        /// Callback method for asynchronous requests.
        /// </summary>
        private void GetResponseAsyncCompleted(IAsyncResult ar)
        {
            GetResponseDelegate caller = (GetResponseDelegate)ar.AsyncState;
            GetResponseAsyncCompletedCore(() => caller.EndInvoke(ar));
        }

        /// <summary>
        /// Callback method for asynchronous requests.
        /// </summary>
        private void GetResponseAsyncCompletedCore(Action completion)
        {
            try
            {
                completion();
            }
            catch (HttpWebServiceException ex)
            {
                m_asyncState.Error = ex;
            }

            // Prevents invoking the callback on the UI thread when the application has been closed
            if (EveClient.Closed)
                return;
            m_asyncState.Callback(m_asyncState);
        }


        /// <summary>
        /// Get the HttpWebResponse for the specified URL.
        /// </summary>
        private HttpWebResponse GetHttpResponse()
        {
            HttpWebRequest request = GetHttpWebRequest(m_url, m_referer);

            // Prepare the POST request we're going to send
            if (request.Method == "POST")
            {
                Stream requestStream = request.GetRequestStream();
                requestStream.Write(m_postData.Content, 0, m_postData.Length);
                requestStream.Close();
            }

            // Query the web site
            HttpWebResponse response = (HttpWebResponse)request.GetResponse();

            // When the address has been redirected, connects to the redirection
            if (response.StatusCode == HttpStatusCode.Redirect || response.StatusCode == HttpStatusCode.Moved || response.StatusCode == HttpStatusCode.MovedPermanently)
            {
                string target = response.GetResponseHeader("Location");
                response.Close();

                return GetRedirectedHttpResponse(target);
            }

            return response;
        }

        /// <summary>
        /// Gets a redirected HttpWebResponse.
        /// </summary>
        private HttpWebResponse GetRedirectedHttpResponse(string target)
        {
            if (m_redirectsRemaining-- > 0)
            {
                Uri referer = new Uri(m_url);
                m_referer = referer.ToString();
                m_url = new Uri(referer, target).ToString();
                return GetHttpResponse();
            }
            else
            {
                throw HttpWebServiceException.RedirectsExceededException(BaseUrl);
            }
        }

        /// <summary>
        /// Constructs an HttpWebRequest for the specified url and referer.
        /// </summary>
        public HttpWebRequest GetHttpWebRequest(string url, string referer)
        {
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
            request.AllowAutoRedirect = false;
            request.Headers[HttpRequestHeader.AcceptLanguage] = "en-us,en;q=0.5";
            request.Headers[HttpRequestHeader.AcceptCharset] = "ISO-8859-1,utf-8;q=0.7,*;q=0.7";
            request.Headers[HttpRequestHeader.Pragma] = "no-cache";
            request.KeepAlive = true;
            request.UserAgent = m_webServiceState.UserAgent;
            request.Accept = m_accept;
            request.Timeout = Timeout;

            if (referer != null)
                request.Referer = referer;
            if (m_postData != null)
            {
                request.Method = "POST";
                request.ContentType = "application/x-www-form-urlencoded";
                request.ContentLength = m_postData.Length;
            }
            if (m_webServiceState.Proxy.Enabled)
            {
                WebProxy proxy = new WebProxy(m_webServiceState.Proxy.Host, m_webServiceState.Proxy.Port);
                switch (m_webServiceState.Proxy.Authentication)
                {
                    case ProxyAuthentication.None:
                        proxy.UseDefaultCredentials = false;
                        proxy.Credentials = null;
                        break;
                    case ProxyAuthentication.SystemDefault:
                        proxy.UseDefaultCredentials = true;
                        break;
                    case ProxyAuthentication.Specified:
                        proxy.UseDefaultCredentials = false;
                        proxy.Credentials = new NetworkCredential(m_webServiceState.Proxy.Username, m_webServiceState.Proxy.Password);
                        break;
                }
                request.Proxy = proxy;
            }
            return request;
        }
    }
}