using System;
using System.Net;

namespace EVEMon.Common.Net
{
    /// <summary>
    /// HttpWebService provides all HTTP-based download services. It is intended to be used as a singleton
    /// instance via the Singleton class.
    /// </summary>
    public partial class HttpWebService
    {
        private readonly HttpWebServiceState m_state = new HttpWebServiceState();

        internal HttpWebService()
        {
            ServicePointManager.Expect100Continue = false;
        }

        /// <summary>
        /// State is a read-only instance of HttpWebServiceState. Changes to web client settings should be made
        /// to properties of this instance.
        /// </summary>
        public HttpWebServiceState State
        {
            get { return m_state; }
        }

        /// <summary>
        /// Validates a Url as acceptable for an HttpWebServiceRequest
        /// </summary>
        /// <param name="url">A url <see cref="string"/> for the request. The string must specify HTTP as its scheme.</param>
        /// <param name="errorMsg">Is url is invalid, contains a descriptive message of the reason</param>
        public static bool IsValidURL(string url, out string errorMsg)
        {
            if (string.IsNullOrEmpty(url))
            {
                errorMsg = "Url may not be null or an empty string.";
                return false;
            }
            if (!Uri.IsWellFormedUriString(url, UriKind.Absolute))
            {
                errorMsg = String.Format(CultureConstants.DefaultCulture, "\"{0}\" is not a well-formed URL.", url);
                return false;
            }
            try
            {
                Uri tempUri = new Uri(url);
                if (tempUri.Scheme != Uri.UriSchemeHttp && tempUri.Scheme != Uri.UriSchemeHttps)
                {
                    errorMsg = String.Format(CultureConstants.DefaultCulture, "The specified scheme ({0}) is not supported.", tempUri.Scheme);
                    return false;
                }
            }
            catch (Exception)
            {
                errorMsg = String.Format(CultureConstants.DefaultCulture, "\"{0}\" is not a valid URL for an HTTP request.", url);
                return false;
            }
            errorMsg = String.Empty;
            return true;
        }

        /// <summary>
        /// Cancels an asynchronous request in progress
        /// </summary>
        public void CancelRequest(object request)
        {
            if (request.GetType() == typeof(HttpWebServiceRequest))
            {
                ((HttpWebServiceRequest) request).Cancelled = true;
            }
        }

        /// <summary>
        /// Factory method to construct an EVEMonWebRequest instance
        /// </summary>
        /// <returns></returns>
        private HttpWebServiceRequest GetRequest()
        {
            return new HttpWebServiceRequest(m_state);
        }
    }

    public delegate void DownloadProgressChangedCallback(DownloadProgressChangedArgs e);

}