using System;
using System.Windows.Forms;

namespace EVEMon.Common.Controls
{
    public partial class ApiErrorTroubleshooter : UserControl
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="ApiErrorTroubleshooter"/> class.
        /// </summary>
        public ApiErrorTroubleshooter()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Occurs when a resolution has been completed.
        /// </summary>
        public event EventHandler<ApiErrorTroubleshooterEventArgs> ErrorResolved;

        /// <summary>
        /// Called when the error is unresolved.
        /// </summary>
        protected void OnErrorUnresolved()
        {
            if (ErrorResolved != null)
                ErrorResolved(null, new ApiErrorTroubleshooterEventArgs());
        }

        /// <summary>
        /// Called when the error is resolved.
        /// </summary>
        /// <param name="action">The action.</param>
        protected void OnErrorResolved(ResolutionAction action)
        {
            if (ErrorResolved != null)
                ErrorResolved(null, new ApiErrorTroubleshooterEventArgs(action));
        }
    }
}