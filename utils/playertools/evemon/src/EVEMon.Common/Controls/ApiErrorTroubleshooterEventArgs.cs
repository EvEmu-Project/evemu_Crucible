using System;

namespace EVEMon.Common.Controls
{
    /// <summary>
    /// Provides event arguments for API Error troubleshooters to notify their parents.
    /// </summary>
    public sealed class ApiErrorTroubleshooterEventArgs : EventArgs
    {
        private bool m_resolved;
        private ResolutionAction m_action;

        /// <summary>
        /// Initializes a new instance of the <see cref="ApiErrorTroubleshooterEventArgs"/> class with the error unresolved.
        /// </summary>
        public ApiErrorTroubleshooterEventArgs()
        {
            m_action = ResolutionAction.None;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ApiErrorTroubleshooterEventArgs"/> class with the error resolved.
        /// </summary>
        /// <param name="action">The action.</param>
        public ApiErrorTroubleshooterEventArgs(ResolutionAction action)
        {
            m_resolved = true;
            m_action = action;
        }

        /// <summary>
        /// Gets a value indicating whether this <see cref="ApiErrorTroubleshooterEventArgs"/> is resolved.
        /// </summary>
        /// <value><c>true</c> if resolved; otherwise, <c>false</c>.</value>
        public bool Resolved
        {
            get
            {
                return m_resolved;
            }
        }

        /// <summary>
        /// Gets the action.
        /// </summary>
        /// <value>The action.</value>
        public ResolutionAction Action
        {
            get
            {
                return m_action;
            }
        }
    }
}