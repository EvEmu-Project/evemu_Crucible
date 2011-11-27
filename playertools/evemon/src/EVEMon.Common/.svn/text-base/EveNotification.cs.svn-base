using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common.Data;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    public sealed class EveNotification : IEveMessage
    {
        private CCPCharacter m_ccpCharacter;
        private bool m_queryPending;


        #region Constructor

        /// <summary>
        /// Constructor from the API.
        /// </summary>
        /// <param name="src"></param>
        internal EveNotification(CCPCharacter ccpCharacter, SerializableNotificationsListItem src)
        {
            m_ccpCharacter = ccpCharacter;
            NotificationID = src.NotificationID;
            Type = EveNotificationType.GetType(src.TypeID);
            Sender = GetIDToName(src.SenderID);
            SentDate = src.SentDate;
            Recipient = GetRecipient();
        }

        #endregion


        #region Properties

        /// <summary>
        /// Gets or sets the EVE notification ID.
        /// </summary>
        /// <value>The notification ID.</value>
        public long NotificationID { get; private set; }

        /// <summary>
        /// Gets or sets the EVE notification type.
        /// </summary>
        /// <value>The type.</value>
        public string Type { get; private set; }

        /// <summary>
        /// Gets or sets the EVE notification sender name.
        /// </summary>
        /// <value>The sender.</value>
        public string Sender { get; private set; }

        /// <summary>
        /// Gets or sets the sent date of the EVE notification.
        /// </summary>
        /// <value>The sent date.</value>
        public DateTime SentDate { get; private set; }

        /// <summary>
        /// Gets or sets the EVE notification recipient.
        /// </summary>
        /// <value>The recipient.</value>
        public List<string> Recipient { get; private set; }

        /// <summary>
        /// Gets or sets the EVE notification text.
        /// </summary>
        /// <value>The EVE notification text.</value>
        public EveNotificationText EVENotificationText { get; private set; }

        /// <summary>
        /// Gets the EVE notification title.
        /// </summary>
        /// <value>The title.</value>
        public string Title { get { return Type; } }

        /// <summary>
        /// Gets the EVE notification text.
        /// </summary>
        /// <value>The text.</value>
        public string Text { get { return EVENotificationText.NotificationText; } }

        #endregion


        #region Helper Methods

        /// <summary>
        /// Gets the name of the ID.
        /// </summary>
        /// <param name="ID">The ID.</param>
        /// <returns></returns>
        private string GetIDToName(long ID)
        {
            if (ID == 0)
                return "Unknown";

            // Look into EVEMon's data file if it's an NPC corporation or agent
            foreach (var station in StaticGeography.AllStations)
            {
                if (station.CorporationID == ID && station.CorporationName != null)
                    return station.CorporationName;

                if (station.Agents.Any(x => x.ID == ID))
                    return station.Agents.First(x => x.ID == ID).Name;
            }

            // Lookup if it's a players null sec corporation
            // (while we have the data we can avoid unnecessary queries to the API)
            Station conqStation = ConquerableStation.AllStations.FirstOrDefault(x => x.CorporationID == ID);
            if (conqStation != null)
                return conqStation.CorporationName;

            // Didn't found any ? Query the API
            return EveIDtoName.GetIDToName(ID.ToString());
        }

        /// <summary>
        /// Gets the recipient.
        /// </summary>
        /// <returns></returns>
        private List<string> GetRecipient()
        {
            Recipient = new List<string>();
            Recipient.Add(m_ccpCharacter.Name);

            return Recipient;
        }

        #endregion


        #region Querying

        /// <summary>
        /// Gets the EVE notification.
        /// </summary>
        public void GetNotificationText()
        {
            // Exit if we are already trying to download the mail message body text
            if (m_queryPending)
                return;

            m_queryPending = true;

            EveClient.APIProviders.CurrentProvider.QueryMethodAsync<SerializableAPINotificationTexts>(
                                                                    APIMethods.NotificationTexts,
                                                                    m_ccpCharacter.Identity.Account.UserID,
                                                                    m_ccpCharacter.Identity.Account.APIKey,
                                                                    m_ccpCharacter.CharacterID,
                                                                    NotificationID,
                                                                    OnEVENotificationTextDownloaded);
        }

        /// <summary>
        /// Processes the queried EVE notification text.
        /// </summary>
        /// <param name="result">The result.</param>
        private void OnEVENotificationTextDownloaded(APIResult<SerializableAPINotificationTexts> result)
        {
            m_queryPending = false;

            // Notify an error occured
            if (m_ccpCharacter.ShouldNotifyError(result, APIMethods.NotificationTexts))
                EveClient.Notifications.NotifyEVENotificationTextsError(m_ccpCharacter, result);

            // Quits if there is an error
            if (result.HasError)
                return;

            // If there is an error response on missing IDs inform the user
            if (!String.IsNullOrEmpty(result.Result.MissingMessageIDs))
                result.Result.Texts.Add(
                                    new SerializableNotificationTextsListItem
                                    {
                                        NotificationID = long.Parse(result.Result.MissingMessageIDs),
                                        NotificationText = "The text for this notification was reported missing." 
                                    });

            // Quit if for any reason there is no text
            if (result.Result.Texts.Count == 0)
                return;            

            // Import the data
            EVENotificationText = new EveNotificationText(result.Result.Texts[0]);

            EveClient.OnCharacterEVENotificationTextDownloaded(m_ccpCharacter);
        }

        #endregion
    }
}
