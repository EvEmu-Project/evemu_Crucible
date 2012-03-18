using System;
using System.Xml;
using System.Xml.Serialization;
using System.Xml.Xsl;

using EVEMon.Common.Net;

namespace EVEMon.Common.Serialization.API
{
    [XmlRoot("eveapi")]
    public sealed class APIResult<T> : IAPIResult
    {
        private APIEnumerations.APIErrors m_error = APIEnumerations.APIErrors.None;
        private readonly string m_errorMessage;
        private readonly Exception m_exception;

        #region Constructors
        /// <summary>
        /// Default constructor.
        /// </summary>
        public APIResult()
        {
            m_error = APIEnumerations.APIErrors.None;
            m_errorMessage = String.Empty;
            m_exception = null;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="APIResult&lt;T&gt;"/> class.
        /// </summary>
        /// <param name="exc">The exception.</param>
        private APIResult(Exception exc)
        {
            m_errorMessage = exc.Message;
            m_exception = exc;
        }

        /// <summary>
        /// Constructor from an http exception
        /// </summary>
        /// <param name="exc">The exception.</param>
        public APIResult(HttpWebServiceException exc)
            : this(exc as Exception)
        {
            m_error = APIEnumerations.APIErrors.Http;
        }

        /// <summary>
        /// Constructor from an XML exception
        /// </summary>
        /// <param name="exc">The exception.</param>
        public APIResult(XmlException exc)
            : this((Exception)exc)
        {
            m_error = APIEnumerations.APIErrors.Xml;
        }

        /// <summary>
        /// Constructor from an XSLT exception
        /// </summary>
        /// <param name="exc">The exception.</param>
        public APIResult(XsltException exc)
            : this(exc as Exception)
        {
            m_error = APIEnumerations.APIErrors.Xslt;
        }

        /// <summary>
        /// Constructor from an XML serialization exception wrapped into an InvalidOperationException
        /// </summary>
        /// <param name="exc">The exception.</param>
        public APIResult(InvalidOperationException exc)
        {
            m_error = APIEnumerations.APIErrors.Xml;
            m_errorMessage = (exc.InnerException == null ? exc.Message : exc.InnerException.Message);
            m_exception = exc;
        }

        /// <summary>
        /// Constructor from a custom exception.
        /// </summary>
        /// <param name="error">The error.</param>
        /// <param name="message">The message.</param>
        public APIResult(APIEnumerations.APIErrors error, string message)
        {
            m_error = error;
            m_errorMessage = message;
            m_exception = null;
        }
        #endregion


        #region Errors handling

        public bool EVEBackendDatabaseDisabled
        {
            get
            {
                if (CCPError != null && CCPError.IsEVEBackendDatabaseDisabled)
                {
                    EveClient.Notifications.NotifyEVEBackendDatabaseDisabled(this);
                    return true;
                }

                EveClient.Notifications.InvalidateAPIError();
                return false;
            }
        }

        /// <summary>
        /// Gets true if the information is outdated.
        /// </summary>
        public bool IsOutdated
        {
            get
            {
                return DateTime.UtcNow > CachedUntil;
            }
        }

        /// <summary>
        /// Gets true if there is an error.
        /// </summary>
        public bool HasError
        {
            get 
            {
                if (CCPError != null)
                    return true;

                return m_error != APIEnumerations.APIErrors.None; 
            }
        }

        /// <summary>
        /// Gets the type of the error or <see cref="APIErrors.None"/> when there was no error.
        /// </summary>
        public APIEnumerations.APIErrors ErrorType
        {
            get 
            {
                if (CCPError != null)
                    return APIEnumerations.APIErrors.CCP;

                return m_error; 
            }
        }

        /// <summary>
        /// Gets the exception.
        /// </summary>
        /// <value>The exception.</value>
        public Exception Exception
        {
            get { return m_exception; }
        }

        /// <summary>
        /// Gets the error message without bothering about its nature.
        /// </summary>
        public string ErrorMessage
        {
            get 
            {
                if (CCPError != null)
                    return CCPError.ErrorMessage;

                return m_errorMessage; 
            }
        }

        /// <summary>
        /// Gets / sets the XML document when there's no HTTP error.
        /// </summary>
        [XmlIgnore]
        public XmlDocument XmlDocument
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the time left before a new version is available.
        /// </summary>
        public TimeSpan RemainingTime
        {
            get { return CachedUntil.Subtract(DateTime.UtcNow); }
        }
        #endregion


        #region CCP Mapping
        [XmlAttribute("version")]
        public int APIVersion
        {
            get;
            set;
        }

        [XmlElement("currentTime")]
        public string CurrentTimeXml
        {
            get { return CurrentTime.DateTimeToTimeString(); }
            set
            {
                if (String.IsNullOrEmpty(value))
                    return;

                CurrentTime = value.TimeStringToDateTime(); 
            }
        }

        [XmlElement("cachedUntil")]
        public string CachedUntilXml
        {
            get { return CachedUntil.DateTimeToTimeString(); }
            set
            {
                if (String.IsNullOrEmpty(value))
                    return;

                CachedUntil = value.TimeStringToDateTime();
            }
        }

        [XmlIgnore]
        public DateTime CurrentTime
        {
            get;
            set;
        }

        [XmlIgnore]
        public DateTime CachedUntil
        {
            get;
            set;
        }

        [XmlElement("error")]
        public APICCPError CCPError
        {
            get;
            set;
        }

        [XmlElement("result")]
        public T Result
        {
            get;
            set;
        }
        #endregion


        #region Time fixing
        /// <summary>
        /// Fixup the currentTime and cachedUntil time to match the user's clock.
        /// This should ONLY be called when the xml is first received from CCP
        /// </summary>
        /// <param name="millisecondsDrift"></param>
        public void SynchronizeWithLocalClock(double millisecondsDrift)
        {
            // Convert the drift between webserver time and local time
            // to a timespan. It is possible for millisecondsDrift to
            // be erroniously outside of the range of an int thus we 
            // need to catch an overflow exception and reset to 0.
            TimeSpan drift;
            try
            {
                drift = new TimeSpan(0, 0, 0, 0, Convert.ToInt32(millisecondsDrift));
            }
            catch (OverflowException)
            {
                drift = new TimeSpan(0, 0, 0, 0);
            }

            // Now fix the server time to align with local time
            if (CurrentTime != DateTime.MinValue)
                CurrentTime -= drift;
            if (CachedUntil != DateTime.MinValue)
                CachedUntil -= drift;

            // Fix the TQ start/end times first
            ISynchronizableWithLocalClock synchronizable = ((Object)Result) as ISynchronizableWithLocalClock;
            
            if (synchronizable != null)
                synchronizable.SynchronizeWithLocalClock(drift);
        }
        #endregion
    }
}
