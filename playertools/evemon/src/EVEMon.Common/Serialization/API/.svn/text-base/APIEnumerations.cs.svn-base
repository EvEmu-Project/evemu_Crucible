namespace EVEMon.Common.Serialization.API
{
    public class APIEnumerations
    {
        #region API Errors

        /// <summary>
        /// Represents the category of error which can occur with the API.
        /// </summary>
        public enum APIErrors
        {
            /// <summary>
            /// There was no error.
            /// </summary>
            None,

            /// <summary>
            /// The error was caused by the network.
            /// </summary>
            Http,

            /// <summary>
            /// The error occurred during the XSL transformation.
            /// </summary>
            Xslt,

            /// <summary>
            /// The error occurred during the XML deserialization.
            /// </summary>
            Xml,

            /// <summary>
            /// It was a managed CCP error.
            /// </summary>
            CCP
        }

        #endregion

        #region API Order State

        /// <summary>
        /// The status of a market order.
        /// </summary>
        public enum CCPOrderState
        {
            Opened = 0,
            Closed = 1,
            ExpiredOrFulfilled = 2,
            Canceled = 3,
            Pending = 4,
            CharacterDeleted = 5
        }

        #endregion

        #region API Job Completed Status

        public enum CCPJobCompletedStatus
        {
            Failed = 0,
            Delivered = 1,
            Aborted = 2,
            GM_Aborted = 3,
            Inflight_Unanchored = 4,
            Destroyed = 5
        }

        #endregion
    }
}
