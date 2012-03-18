using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.SettingsObjects;

using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;

namespace EVEMon.APIErrorHandling
{
    /// <summary>
    /// Http Timeout Troubleshooter displays when a HTTP Timeout has occurred.
    /// </summary>
    public partial class HttpTimeoutTroubleshooter : ApiErrorTroubleshooter
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="HttpTimeoutTroubleshooter"/> class.
        /// </summary>
        public HttpTimeoutTroubleshooter()
        {
            InitializeComponent();

            var Options = new List<TimeoutOption>();
            var updateSettings = new UpdateSettings();
            
            // lets add 10 - 60 to the list.
            for (int i = 10; i <= 60; i += 10)
            {
                string text = String.Empty;

                if (i == updateSettings.HttpTimeout)
                    text = "Default";

                if (i == Settings.Updates.HttpTimeout)
                    text = "Current";

                Options.Add(new TimeoutOption(i, text));
            }

            // if the current is set to something odd we add it and sort by Seconds
            if (!Options.Any(x => x.Seconds == Settings.Updates.HttpTimeout))
                Options.Add(new TimeoutOption(Settings.Updates.HttpTimeout, "Current"));

            // if the default is not in the list we add it
            if (!Options.Any(x => x.Seconds == updateSettings.HttpTimeout))
                Options.Add(new TimeoutOption(updateSettings.HttpTimeout, "Default"));

            Options.Sort((a, b) => a.Seconds.CompareTo(b.Seconds));


            // databind
            TimeoutDropDown.DataSource = Options;
            TimeoutDropDown.DisplayMember = "Label";
            TimeoutDropDown.ValueMember = "Seconds";
        }

        /// <summary>
        /// Handles the Click event of the SetTimeoutButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void SetTimeoutButton_Click(object sender, EventArgs e)
        {
            Settings.Updates.HttpTimeout = (int)TimeoutDropDown.SelectedValue;
            OnErrorResolved(ResolutionAction.Close);
        }

        /// <summary>
        /// Private class to store and format suitable time spans.
        /// </summary>
        private class TimeoutOption
        {
            /// <summary>
            /// Initializes a new instance of the <see cref="TimeoutOption"/> class.
            /// </summary>
            /// <param name="seconds">The seconds.</param>
            public TimeoutOption(int seconds)
            {
                Seconds = seconds;
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="TimeoutOption"/> class.
            /// </summary>
            /// <param name="seconds">The seconds.</param>
            /// <param name="text">The text.</param>
            public TimeoutOption(int seconds, string text)
            {
                Seconds = seconds;
                Text = text;
            }

            /// <summary>
            /// Gets or sets the seconds.
            /// </summary>
            /// <value>The seconds.</value>
            public int Seconds { get; set; }

            /// <summary>
            /// Gets or sets the text.
            /// </summary>
            /// <value>The text.</value>
            private string Text { get; set; }

            /// <summary>
            /// Gets the label.
            /// </summary>
            /// <value>The label.</value>
            public String Label
            {
                get
                {
                    var builder = new StringBuilder();

                    if (Seconds % 60 == 0)
                    {
                        int minutes = Seconds / 60;
                        builder.AppendFormat("{0} Minute{1}", minutes, minutes == 1 ? String.Empty : "s");
                    }
                    else
                    {
                        builder.AppendFormat("{0} Seconds", Seconds);
                    }

                    if (!String.IsNullOrEmpty(Text))
                        builder.AppendFormat(" ({0})", Text);

                    return builder.ToString();
                }
            }
        }
    }
}
