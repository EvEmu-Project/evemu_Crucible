using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    public sealed class EveMailMessage : IEveMessage
    {
        private CCPCharacter m_ccpCharacter;
        private bool m_queryPending;


        #region Constructor

        /// <summary>
        /// Constructor from the API.
        /// </summary>
        /// <param name="src"></param>
        internal EveMailMessage(CCPCharacter ccpCharacter, SerializableMailMessagesListItem src)
        {
            m_ccpCharacter = ccpCharacter;
            State = (src.SenderID != ccpCharacter.CharacterID ?
                        EVEMailState.Inbox : EVEMailState.SentItem);
            MessageID = src.MessageID;
            Sender = src.ToListID.Any(x => x == src.SenderID.ToString()) ?
                        GetMailingListIDToName(src.SenderID.ToString()) : EveIDtoName.GetIDToName(src.SenderID.ToString());
            SentDate = src.SentDate;
            Title = src.Title;
            ToCorpOrAlliance = EveIDtoName.GetIDToName(src.ToCorpOrAllianceID);
            ToCharacters = GetIDsToNames(src.ToCharacterIDs);
            ToMailingLists = GetMailingListIDsToNames(src.ToListID);
            Recipient = GetRecipient();
        }

        #endregion


        #region Properties

        /// <summary>
        /// Gets or sets the EVE mail state.
        /// </summary>
        /// <value>The state.</value>
        public EVEMailState State { get; private set; }

        /// <summary>
        /// Gets or sets the EVE mail message ID.
        /// </summary>
        /// <value>The message ID.</value>
        public long MessageID { get; private set; }

        /// <summary>
        /// Gets or sets the EVE mail sender name.
        /// </summary>
        /// <value>The sender.</value>
        public string Sender { get; private set; }

        /// <summary>
        /// Gets or sets the sent date of the EVE mail.
        /// </summary>
        /// <value>The sent date.</value>
        public DateTime SentDate { get; private set; }

        /// <summary>
        /// Gets or sets the EVE mail title.
        /// </summary>
        /// <value>The title.</value>
        public string Title { get; private set; }

        /// <summary>
        /// Gets or sets the EVE mail recipient (corp or alliance).
        /// </summary>
        /// <value>To corp or alliance.</value>
        public string ToCorpOrAlliance { get; private set; }

        /// <summary>
        /// Gets or sets the EVE mail recipient(s) (characters).
        /// </summary>
        /// <value>To characters.</value>
        public List<string> ToCharacters { get; private set; }

        /// <summary>
        /// Gets or sets the EVE mail recipient (mailing lists).
        /// </summary>
        /// <value>To mailing lists.</value>
        public List<string> ToMailingLists { get; private set; }

        /// <summary>
        /// Gets or sets the recipients.
        /// </summary>
        /// <value>The recipient.</value>
        public List<string> Recipient { get; private set; }

        /// <summary>
        /// Gets or sets the EVE mail body.
        /// </summary>
        /// <value>The EVE mail body.</value>
        public EveMailBody EVEMailBody { get; private set; }

        /// <summary>
        /// Gets the EVE mail body text.
        /// </summary>
        /// <value>The text.</value>
        public string Text { get { return EVEMailBody.BodyText; } }

        #endregion


        #region Helper Methods

        /// <summary>
        /// Gets the names of the character IDs.
        /// </summary>
        /// <param name="src">A list of character IDs.</param>
        /// <returns>A list of character names</returns>
        private List<string> GetIDsToNames(List<string> src)
        {
            // If there are no IDs to query return a list with an empty entry
            if (src.Count == 0)
            {
                src.Add(String.Empty);
                return src;
            }

            List<string> listOfNames = new List<string>();
            List<string> listOfIDsToQuery = new List<string>();

            foreach (var id in src)
            {
                if (id == m_ccpCharacter.CharacterID.ToString())
                {
                    listOfNames.Add(m_ccpCharacter.Name);
                }
                else
                {
                    listOfIDsToQuery.Add(id);
                }
            }

            // We have IDs to query
            if (listOfIDsToQuery.Count > 0)
                listOfNames.AddRange(EveIDtoName.GetIDsToNames(listOfIDsToQuery));

            return listOfNames;
        }

        /// <summary>
        /// Gets the name of the mailing list ID.
        /// </summary>
        /// <param name="mailingListID">The mailing list ID.</param>
        /// <returns></returns>
        private string GetMailingListIDToName(string mailingListID)
        {
            // If there is no ID to query return an empty string
            if (String.IsNullOrEmpty(mailingListID))
                return String.Empty;

            // If it's a zero ID return "Unknown"
            if (mailingListID == "0")
                return "Unknown";

            List<string> list = new List<string>();
            list.Add(mailingListID);
            List<string> name = GetMailingListIDsToNames(list);
            return name[0];
        }

        /// <summary>
        /// Gets the mailing list IDs to names.
        /// </summary>
        /// <param name="mailingListIDs">The mailing list IDs.</param>
        /// <returns></returns>
        private List<string> GetMailingListIDsToNames(List<string> mailingListIDs)
        {
            // If there are no IDs to query return a list with an empty entry
            if (mailingListIDs.Count == 0)
            {
                mailingListIDs.Add(String.Empty);
                return mailingListIDs;
            }

            List<string> listOfNames = new List<string>();

            foreach (var listID in mailingListIDs)
            {
                var mailingList = m_ccpCharacter.EVEMailingLists.FirstOrDefault(x => x.ID.ToString() == listID);
                if (mailingList != null)
                {
                    listOfNames.Add(mailingList.Name);
                }
                else
                {
                    listOfNames.Add("Unknown");
                }
            }

            // In case the list returned from the API is empty, add an "Unknown" entry
            if (listOfNames.Count == 0)
                listOfNames.Add("Unknown");

            return listOfNames;
        }

        /// <summary>
        /// Gets the recipient.
        /// </summary>
        /// <returns></returns>
        private List<string> GetRecipient()
        {
            Recipient = new List<string>();

            if (!String.IsNullOrEmpty(ToCharacters[0]))
            {
                Recipient.AddRange(ToCharacters);
                return Recipient;
            }

            if (!String.IsNullOrEmpty(ToCorpOrAlliance))
            {
                Recipient.Add(ToCorpOrAlliance);
                return Recipient;
            }

            if (!String.IsNullOrEmpty(ToMailingLists[0]))
            {
                Recipient.AddRange(ToMailingLists);
                return Recipient;
            }

            return Recipient;
        }

        #endregion


        #region Querying

        /// <summary>
        /// Gets the EVE mail body.
        /// </summary>
        public void GetMailBody()
        {
            // Exit if we are already trying to download the mail message body text
            if (m_queryPending)
                return;

            m_queryPending = true;

            EveClient.APIProviders.CurrentProvider.QueryMethodAsync<SerializableAPIMailBodies>(
                                                                    APIMethods.MailBodies,
                                                                    m_ccpCharacter.Identity.Account.UserID,
                                                                    m_ccpCharacter.Identity.Account.APIKey,
                                                                    m_ccpCharacter.CharacterID,
                                                                    MessageID,
                                                                    OnEVEMailBodyDownloaded);
        }

        /// <summary>
        /// Processes the queried EVE mail message mail body.
        /// </summary>
        /// <param name="result">The result.</param>
        private void OnEVEMailBodyDownloaded(APIResult<SerializableAPIMailBodies> result)
        {
            m_queryPending = false;

            // Notify an error occured
            if (m_ccpCharacter.ShouldNotifyError(result, APIMethods.MailBodies))
                EveClient.Notifications.NotifyEVEMailBodiesError(m_ccpCharacter, result);

            // Quits if there is an error
            if (result.HasError)
                return;

            // If there is an error response on missing IDs inform the user
            if (!String.IsNullOrEmpty(result.Result.MissingMessageIDs))
                result.Result.Bodies.Add(
                                    new SerializableMailBodiesListItem
                                    {
                                        MessageID = long.Parse(result.Result.MissingMessageIDs),
                                        MessageText = "The text for this message was reported missing."
                                    });

            // Quit if for any reason there is no text
            if (result.Result.Bodies.Count == 0)
                return;            

            // Import the data
            EVEMailBody = new EveMailBody(result.Result.Bodies[0]);

            EveClient.OnCharacterEVEMailBodyDownloaded(m_ccpCharacter);
        }

        #endregion
    }
}
