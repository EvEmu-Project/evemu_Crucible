using System;

using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon
{
    public partial class EveMessageWindow : EVEMonForm
    {
        private IEveMessage m_message;

        /// <summary>
        /// Initializes a new instance of the <see cref="EveMessageWindow"/> class.
        /// </summary>
        internal EveMessageWindow()
        {
            InitializeComponent();

            RememberPositionKey = "EVEMessageWindow";
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="EveMessageWindow"/> class.
        /// </summary>
        /// <param name="message">The message.</param>
        public EveMessageWindow(IEveMessage message)
            : this()
        {
            EVEMessage = message;
        }

        /// <summary>
        /// Gets or sets the EVE message.
        /// </summary>
        /// <value>The message.</value>
        internal IEveMessage EVEMessage
        {
            get { return m_message; }
            set
            {
                if (m_message == value)
                    return;

                m_message = value;
                Tag = value;

                readingPane.SelectedObject = m_message;
                UpdateWindowHeaderText();
            }
        }

        /// <summary>
        /// Updates the window header text.
        /// </summary>
        private void UpdateWindowHeaderText()
        {
            Text = String.Format("{0} - EVE Message", m_message.Title);
        }
    }
}
