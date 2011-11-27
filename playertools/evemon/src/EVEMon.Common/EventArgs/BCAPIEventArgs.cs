using System;

namespace EVEMon.Common
{
    public sealed class BCAPIEventArgs : EventArgs
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="errorMessage">The error message.</param>
        public BCAPIEventArgs(string errorMessage)
        {
            HasError = !String.IsNullOrEmpty(errorMessage);
            ErrorMessage = errorMessage;
        }

        /// <summary>
        /// Gets or sets a value indicating whether this event has error.
        /// </summary>
        /// <value><c>true</c> if this event has error; otherwise, <c>false</c>.</value>
        public bool HasError { get; private set; }

        /// <summary>
        /// Gets or sets the error message.
        /// </summary>
        /// <value>The error message.</value>
        public string ErrorMessage { get; private set; }
    }
}
