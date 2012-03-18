using System;
using System.Net;
using System.Text;
using System.Xml;

namespace EVEMon.Common.Net
{
    /// <summary>
    /// Exception class for all exceptions thrown by HttpWebService requests
    /// </summary>
    public sealed class HttpWebServiceException : ApplicationException
    {
        private readonly HttpWebServiceExceptionStatus _status;
        private readonly string _url;

        private HttpWebServiceException(HttpWebServiceExceptionStatus status, string url, string message)
            : base(message)
        {
            _status = status;
            _url = url;
        }

        private HttpWebServiceException(HttpWebServiceExceptionStatus status, Exception ex, string url, string message)
            : base(message, ex)
        {
            _status = status;
            _url = url;
        }

        public HttpWebServiceExceptionStatus Status
        {
            get { return _status; }
        }

        public string Url
        {
            get { return _url; }
        }

        public string HostName
        {
            get { return new Uri(_url).Host; }
        }

        /// <summary>
        /// Factory method to create an HttpWebServiceException of type 'Exception'
        /// </summary>
        /// <param name="url">The url of the request that failed</param>
        /// <param name="ex">The exception that was thrown</param>
        /// <returns></returns>
        public static HttpWebServiceException Exception(string url, Exception ex)
        {
            return new HttpWebServiceException(HttpWebServiceExceptionStatus.Exception, ex, url, "An Exception occurred.");
        }

        /// <summary>
        /// Factory method to create an HttpWebServiceException of type 'RedirectsExceeded'
        /// </summary>
        /// <param name="url">The url of the request that failed</param>
        /// <returns></returns>
        public static HttpWebServiceException RedirectsExceededException(string url)
        {
            return new HttpWebServiceException(HttpWebServiceExceptionStatus.RedirectsExceeded, url, String.Format(ExceptionMessages.RedirectsExceeded, GetHostName(url)));
        }

        /// <summary>
        /// Factory method to create an HttpWebServiceException of type 'RequestsDisabled'
        /// </summary>
        /// <param name="url">The url of the request that failed</param>
        /// <returns></returns>
        public static HttpWebServiceException RequestsDisabledException(string url)
        {
            return new HttpWebServiceException(HttpWebServiceExceptionStatus.RequestsDisabled, url, String.Format(ExceptionMessages.RequestsDisabled, GetHostName(url)));
        }

        /// <summary>
        /// Factory method to create an HttpWebServiceException resulting from a WebException.
        /// Various different HttpWebServiceExceptionStatus types are applied, with appropriate messages, depending on the
        /// nature of the WebException.
        /// </summary>
        /// <param name="url">The url of the request that failed</param>
        /// <param name="webServiceState">The EVEMonWebClientState instance of the request</param>
        /// <param name="ex">The WebException that was thrown</param>
        /// <returns></returns>
        public static HttpWebServiceException WebException(string url, HttpWebServiceState webServiceState, WebException ex)
        {
            string proxyHost;
            if (webServiceState.Proxy.Enabled)
            {
                proxyHost = webServiceState.Proxy.Host;
            }
            else
            {
                proxyHost = HttpWebRequest.DefaultWebProxy.GetProxy(new Uri(url)).Host;
            }

            HttpWebServiceExceptionStatus status;

            var msg = ParseWebException(ex, url, proxyHost, out status);
            return new HttpWebServiceException(status, ex, url, msg);
        }

        /// <summary>
        /// Parses a web exception to get an error message and a <see cref="HttpWebServiceExceptionStatus"/> status code.
        /// </summary>
        /// <param name="ex"></param>
        /// <param name="url"></param>
        /// <param name="proxyHost"></param>
        /// <param name="status"></param>
        /// <returns></returns>
        public static string ParseWebException(WebException ex, string url, string proxyHost, out HttpWebServiceExceptionStatus status)
        {
            var messageBuilder = new StringBuilder();
            switch (ex.Status)
            {
                case WebExceptionStatus.ProtocolError:
                    HttpWebResponse response = (HttpWebResponse)ex.Response;
                    switch (response.StatusCode)
                    {
                        case HttpStatusCode.ProxyAuthenticationRequired:
                            status = HttpWebServiceExceptionStatus.ProxyError;
                            messageBuilder.AppendFormat(
                                ExceptionMessages.ProxyAuthenticationFailure,
                                proxyHost, GetHostName(url));
                            break;

                        default:
                            status = HttpWebServiceExceptionStatus.ServerError;
                            messageBuilder.AppendFormat(ExceptionMessages.ServerError,
                                GetHostName(url));
                            messageBuilder.AppendLine(response.StatusDescription);
                            break;
                    }
                    break;
                case WebExceptionStatus.ProxyNameResolutionFailure:
                    status = HttpWebServiceExceptionStatus.ProxyError;
                    messageBuilder.AppendFormat(
                        ExceptionMessages.ProxyNameResolutionFailure, proxyHost);
                    break;
                case WebExceptionStatus.RequestProhibitedByProxy:
                    status = HttpWebServiceExceptionStatus.ProxyError;
                    messageBuilder.AppendFormat(ExceptionMessages.RequestProhibitedByProxy,
                        GetHostName(url), proxyHost);
                    break;
                case WebExceptionStatus.NameResolutionFailure:
                    status = HttpWebServiceExceptionStatus.NameResolutionFailure;
                    messageBuilder.AppendFormat(ExceptionMessages.NameResolutionFailure,
                        proxyHost);
                    break;
                case WebExceptionStatus.ConnectFailure:
                    status = HttpWebServiceExceptionStatus.ConnectFailure;
                    messageBuilder.AppendFormat(ExceptionMessages.ConnectFailure, GetHostName(url));
                    break;
                case WebExceptionStatus.Timeout:
                    status = HttpWebServiceExceptionStatus.Timeout;
                    messageBuilder.AppendFormat(ExceptionMessages.Timeout, GetHostName(url));
                    break;
                default:
                    status = HttpWebServiceExceptionStatus.WebException;
                    messageBuilder.AppendFormat(ExceptionMessages.UnknownWebException,
                        GetHostName(url), ex.Status);
                    break;
            }

            return messageBuilder.ToString();
        }

        /// <summary>
        /// Factory method to create an HttpWebServiceException for Xml download requests that fail due to an XmlException
        /// </summary>
        /// <param name="url">The url of the request that failed</param>
        /// <param name="ex">The XmlException that was thrown</param>
        /// <returns></returns>
        public static HttpWebServiceException XmlException(string url, XmlException ex)
        {
            return new HttpWebServiceException(HttpWebServiceExceptionStatus.XmlException, ex, url, String.Format(ExceptionMessages.XmlException, GetHostName(url)));
        }

        /// <summary>
        /// Factory method to create an HttpWebServiceException for Image download requests that fail because a valid image was not returned
        /// </summary>
        /// <param name="url">The url of the request that failed</param>
        /// <param name="ex">The exception that was thrown loading the image</param>
        /// <returns></returns>
        public static HttpWebServiceException ImageException(string url, ArgumentException ex)
        {
            return new HttpWebServiceException(HttpWebServiceExceptionStatus.ImageException, ex, url, String.Format(ExceptionMessages.ImageException, GetHostName(url)));
        }

        /// <summary>
        /// Factory method to create an HttpWebServiceException for File download requests that fail because the file could not be written
        /// </summary>
        /// <param name="url">The url of the request that failed</param>
        /// <param name="ex">The exception that was thrown creating the file</param>
        /// <returns></returns>
        public static HttpWebServiceException FileError(string url, Exception ex)
        {
            return new HttpWebServiceException(HttpWebServiceExceptionStatus.FileError, ex, url, String.Format(ExceptionMessages.FileException, GetHostName(url)));
        }

        /// <summary>
        /// Helper method to return the host name of a url
        /// </summary>
        /// <param name="url"></param>
        /// <returns></returns>
        private static string GetHostName(string url)
        {
            return new Uri(url).Host;
        }
    }
}
