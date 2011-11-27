using System;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    public sealed class EveMailBody
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="EveMailBody"/> class.
        /// </summary>
        /// <param name="src">The SRC.</param>
        internal EveMailBody(SerializableMailBodiesListItem src)
        {
            MessageID = src.MessageID;
            BodyText = src.MessageText.Normalize();
        }

        /// <summary>
        /// Gets or sets the message ID.
        /// </summary>
        /// <value>The message ID.</value>
        private long MessageID { get; set; }

        /// <summary>
        /// Gets or sets the body text.
        /// </summary>
        /// <value>The body text.</value>
        public string BodyText { get; private set; }
    }
}
