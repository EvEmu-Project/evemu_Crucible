using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common.Data;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a character from CCP, with additional capacities for training and such
    /// </summary>
    public sealed class CCPCharacter : Character
    {
        private readonly CharacterQueryMonitor<SerializableAPISkillQueue> m_skillQueueMonitor;
        private readonly CharacterQueryMonitor<SerializableAPICharacterSheet> m_charSheetMonitor;
        private readonly CharacterQueryMonitor<SerializableAPIResearch> m_charResearchPointsMonitor;
        private readonly CharacterQueryMonitor<SerializableAPIMarketOrders> m_charMarketOrdersMonitor;
        private readonly CharacterQueryMonitor<SerializableAPIMarketOrders> m_corpMarketOrdersMonitor;
        private readonly CharacterQueryMonitor<SerializableAPIIndustryJobs> m_charIndustryJobsMonitor;
        private readonly CharacterQueryMonitor<SerializableAPIIndustryJobs> m_corpIndustryJobsMonitor;
        private readonly CharacterQueryMonitor<SerializableAPIMailMessages> m_charEVEMailMessagesMonitor;
        private readonly CharacterQueryMonitor<SerializableAPINotifications> m_charEVENotificationsMonitor;
        private readonly MarketOrderCollection m_marketOrders;
        private readonly IndustryJobCollection m_industryJobs;
        private readonly ResearchPointCollection m_researchPoints;
        private readonly EveMailMessagesCollection m_eveMailMessages;
        private readonly EveMailingListsCollection m_eveMailingLists;
        private readonly EveNotificationsCollection m_eveNotifications;
        private readonly SkillQueue m_queue;
        private readonly QueryMonitorCollection m_monitors;

        private List<SerializableOrderListItem> m_orders = new List<SerializableOrderListItem>();
        private List<SerializableJobListItem> m_jobs = new List<SerializableJobListItem>();
        private APIMethods m_errorNotifiedMethod;
        private DateTime m_mailingListsNextUpdate = DateTime.MinValue;
        private SerializableAPIMailMessages m_cachedAPIMailMessagesResult;

        private bool m_charOrdersUpdated;
        private bool m_corpOrdersUpdated;
        private bool m_charOrdersAdded;
        private bool m_corpOrdersAdded;

        private bool m_charJobsUpdated;
        private bool m_corpJobsUpdated;
        private bool m_charJobsAdded;
        private bool m_corpJobsAdded;

        #region Constructors

        /// <summary>
        /// Base constructor.
        /// </summary>
        /// <param name="identity"></param>
        /// <param name="guid"></param>
        private CCPCharacter(CharacterIdentity identity, Guid guid)
            : base(identity, guid)
        {
            m_queue = new SkillQueue(this);
            m_marketOrders = new MarketOrderCollection(this);
            m_industryJobs = new IndustryJobCollection(this);
            m_researchPoints = new ResearchPointCollection(this);
            m_eveMailMessages = new EveMailMessagesCollection(this);
            m_eveMailingLists = new EveMailingListsCollection(this);
            m_eveNotifications = new EveNotificationsCollection(this);
            m_monitors = new QueryMonitorCollection();

            // Initializes the query monitors 
            m_charSheetMonitor = new CharacterQueryMonitor<SerializableAPICharacterSheet>(this, APIMethods.CharacterSheet);
            m_charSheetMonitor.Updated += OnCharacterSheetUpdated;
            m_monitors.Add(m_charSheetMonitor);

            m_skillQueueMonitor = new CharacterQueryMonitor<SerializableAPISkillQueue>(this, APIMethods.SkillQueue);
            m_skillQueueMonitor.Updated += OnSkillQueueUpdated;
            m_monitors.Add(m_skillQueueMonitor);

            m_charMarketOrdersMonitor = new CharacterQueryMonitor<SerializableAPIMarketOrders>(this, APIMethods.MarketOrders);
            m_charMarketOrdersMonitor.Updated += OnCharacterMarketOrdersUpdated;
            m_monitors.Add(m_charMarketOrdersMonitor);

            m_corpMarketOrdersMonitor = new CharacterQueryMonitor<SerializableAPIMarketOrders>(this, APIMethods.CorporationMarketOrders);
            m_corpMarketOrdersMonitor.Updated += OnCorporationMarketOrdersUpdated;
            m_monitors.Add(m_corpMarketOrdersMonitor);

            m_charIndustryJobsMonitor = new CharacterQueryMonitor<SerializableAPIIndustryJobs>(this, APIMethods.IndustryJobs);
            m_charIndustryJobsMonitor.Updated += OnCharacterJobsUpdated;
            m_monitors.Add(m_charIndustryJobsMonitor);

            m_corpIndustryJobsMonitor = new CharacterQueryMonitor<SerializableAPIIndustryJobs>(this, APIMethods.CorporationIndustryJobs);
            m_corpIndustryJobsMonitor.Updated += OnCorporationJobsUpdated;
            m_monitors.Add(m_corpIndustryJobsMonitor);

            m_charResearchPointsMonitor = new CharacterQueryMonitor<SerializableAPIResearch>(this, APIMethods.ResearchPoints);
            m_charResearchPointsMonitor.Updated += OnCharacterResearchPointsUpdated;
            m_monitors.Add(m_charResearchPointsMonitor);

            m_charEVEMailMessagesMonitor = new CharacterQueryMonitor<SerializableAPIMailMessages>(this, APIMethods.MailMessages);
            m_charEVEMailMessagesMonitor.Updated += OnCharacterEVEMailMessagesUpdated;
            m_monitors.Add(m_charEVEMailMessagesMonitor);

            m_charEVENotificationsMonitor = new CharacterQueryMonitor<SerializableAPINotifications>(this, APIMethods.Notifications);
            m_charEVENotificationsMonitor.Updated += OnCharacterEVENotificationsUpdated;
            m_monitors.Add(m_charEVENotificationsMonitor);

            // We enable only the monitors that require a limited api key,
            // full api key required monitors will be enabled individually
            // through each character's enabled full api key feature
            foreach (var monitor in m_monitors)
            {
                monitor.Enabled = !monitor.IsFullKeyNeeded;
            }
        }

        /// <summary>
        /// Deserialization constructor.
        /// </summary>
        /// <param name="identity">The identity for this character</param>
        /// <param name="serial">A deserialization object for characters</param>
        internal CCPCharacter(CharacterIdentity identity, SerializableCCPCharacter serial)
            : this(identity, serial.Guid)
        {
            Import(serial);
        }

        /// <summary>
        /// Constructor for a new CCP character on a new identity.
        /// </summary>
        /// <param name="identity"></param>
        internal CCPCharacter(CharacterIdentity identity)
            : this(identity, Guid.NewGuid())
        {
            m_charSheetMonitor.ForceUpdate(true);
            m_skillQueueMonitor.ForceUpdate(true);
        }
        
        #endregion


        #region Public Properties

        /// <summary>
        /// Gets an adorned name, with (file), (url) or (cached) labels.
        /// </summary>
        public override string AdornedName
        {
            get
            {
                if (m_charSheetMonitor.LastResult != null && m_charSheetMonitor.LastResult.HasError)
                    return String.Format("{0} (cached)", m_name);

                return m_name;
            }
        }

        /// <summary>
        /// Gets the skill queue for this character.
        /// </summary>
        public SkillQueue SkillQueue
        {
            get { return m_queue; }
        }

        /// <summary>
        /// Gets the collection of market orders.
        /// </summary>
        public MarketOrderCollection MarketOrders
        {
            get { return m_marketOrders; }
        }

        /// <summary>
        /// Gets the collection of industry jobs.
        /// </summary>
        public IndustryJobCollection IndustryJobs
        {
            get { return m_industryJobs; }
        }

        /// <summary>
        /// Gets the collection of research points.
        /// </summary>
        public ResearchPointCollection ResearchPoints
        {
            get { return m_researchPoints; }
        }

        /// <summary>
        /// Gets the collection of EVE mail messages.
        /// </summary>
        public EveMailMessagesCollection EVEMailMessages
        {
            get { return m_eveMailMessages; }
        }

        /// <summary>
        /// Gets the collection of EVE mail messages.
        /// </summary>
        public EveMailingListsCollection EVEMailingLists
        {
            get { return m_eveMailingLists; }
        }

        /// <summary>
        /// Gets the collection of EVE notifications.
        /// </summary>
        public EveNotificationsCollection EVENotifications
        {
            get { return m_eveNotifications; }
        }

        /// <summary>
        /// Gets true when the character is currently actively training, false otherwise.
        /// </summary>
        public override bool IsTraining
        {
            get { return m_queue.IsTraining; }
        }

        /// <summary>
        /// Gets the skill currently in training, even when it is paused.
        /// </summary>
        public override QueuedSkill CurrentlyTrainingSkill
        {
            get { return m_queue.CurrentlyTraining; }
        }

        /// <summary>
        /// Gets true when the character is in a NPC corporation, false otherwise.
        /// </summary>
        public bool IsInNPCCorporation
        {
            get { return StaticGeography.AllStations.Any(x => x.CorporationID == CorporationID); }
        }

        /// <summary>
        /// Gets true when character has insufficient balance to complete its buy orders.
        /// </summary>
        public bool HasSufficientBalance
        {
            get
            {
                var activeBuyOrdersIssuedForCharacter = m_marketOrders
                    .Where(x => (x.State == OrderState.Active || x.State == OrderState.Modified)
                    && x is BuyOrder && x.IssuedFor == IssuedFor.Character);

                decimal activeTotal = activeBuyOrdersIssuedForCharacter.Sum(x => x.TotalPrice);
                decimal activeEscrow = activeBuyOrdersIssuedForCharacter.Sum(x => ((BuyOrder)x).Escrow);
                decimal additionalToCover = activeTotal - activeEscrow;

                return m_balance >= additionalToCover;
            }
        }

        /// <summary>
        /// Gets the query monitors enumeration.
        /// </summary>
        public QueryMonitorCollection QueryMonitors
        {
            get { return m_monitors; }
        }
        
        #endregion


        #region Importing & Exporting

        /// <summary>
        /// Create a serializable character sheet for this character
        /// </summary>
        /// <returns></returns>
        public override SerializableSettingsCharacter Export()
        {
            var serial = new SerializableCCPCharacter();
            Export(serial);

            // Skill queue
            serial.SkillQueue = m_queue.Export();

            // Market orders
            serial.MarketOrders = m_marketOrders.Export();

            // Industry jobs
            serial.IndustryJobs = m_industryJobs.Export();

            // Research points
            serial.ResearchPoints = m_researchPoints.Export();

            // Eve mail messages IDs
            serial.EveMailMessagesIDs = m_eveMailMessages.Export();

            // Eve notifications IDs
            serial.EveNotificationsIDs = m_eveNotifications.Export();

            // Last API updates
            foreach (var monitor in m_monitors)
            {
                var update = new SerializableAPIUpdate
                {
                    Method = monitor.Method,
                    Time = monitor.LastUpdate
                };

                serial.LastUpdates.Add(update);
            }

            return serial;
        }

        /// <summary>
        /// Imports data from a serialization object
        /// </summary>
        /// <param name="serial"></param>
        public void Import(SerializableCCPCharacter serial)
        {
            Import((SerializableSettingsCharacter)serial);

            // Skill queue
            m_queue.Import(serial.SkillQueue);
            m_queue.UpdateOnTimerTick();

            // Market orders
            m_marketOrders.Import(serial.MarketOrders);

            // Industry jobs
            m_industryJobs.Import(serial.IndustryJobs);

            // Research points
            m_researchPoints.Import(serial.ResearchPoints);

            // EVE mail messages IDs
            m_eveMailMessages.Import(serial.EveMailMessagesIDs);

            // EVE notifications IDs
            m_eveNotifications.Import(serial.EveNotificationsIDs);

            // Last API updates
            foreach (var lastUpdate in serial.LastUpdates)
            {
                var monitor = m_monitors[lastUpdate.Method] as IQueryMonitorEx;
                if (monitor != null)
                    monitor.Reset(lastUpdate.Time);
            }

            // Fire the global event
            EveClient.OnCharacterChanged(this);
        }
        
        #endregion
        
        
        #region Querying

        /// <summary>
        /// Updates the character on a timer tick.
        /// </summary>
        internal override void UpdateOnOneSecondTick()
        {
            if (!Monitored)
                return;

            m_monitors.UpdateOnOneSecondTick();
            m_queue.UpdateOnTimerTick();

            // Exit if API key is a limited one
            Account account = m_identity.Account;
            if (account == null || account.KeyLevel != CredentialsLevel.Full)
                return;

            // Exit if industry jobs monitoring is disabled
            if (!m_charIndustryJobsMonitor.Enabled)
                return;

            m_industryJobs.UpdateOnTimerTick();
        }

        /// <summary>
        /// Queries the character's mailing list.
        /// </summary>
        private void QueryCharacterMailingList()
        {
            EveClient.APIProviders.CurrentProvider.QueryMethodAsync<SerializableAPIMailingLists>(
                                                                        APIMethods.MailingLists,
                                                                        Identity.Account.UserID,
                                                                        Identity.Account.APIKey,
                                                                        CharacterID,
                                                                        OnCharacterMailingListUpdated);
        }

        /// <summary>
        /// Processed the queried skill queue information.
        /// </summary>
        /// <param name="result"></param>
        private void OnCharacterSheetUpdated(APIResult<SerializableAPICharacterSheet> result)
        {
            // Notify an error occurred
            if (ShouldNotifyError(result, APIMethods.CharacterSheet))
                EveClient.Notifications.NotifyCharacterSheetError(this, result);

            // Quits if there is an error
            if (result.HasError)
                return;

            // Imports the data
            Import(result);

            // Check the character has a sufficient clone or send a notification
            if (Monitored && (m_cloneSkillPoints < SkillPoints))
            {
                EveClient.Notifications.NotifyInsufficientClone(this);
                return;
            }

            EveClient.Notifications.InvalidateInsufficientClone(this);
        }

        /// <summary>
        /// Processes the queried skill queue information.
        /// </summary>
        /// <param name="result"></param>
        private void OnSkillQueueUpdated(APIResult<SerializableAPISkillQueue> result)
        {
            // Notify an error occurred
            if (ShouldNotifyError(result, APIMethods.SkillQueue))
                EveClient.Notifications.NotifySkillQueueError(this, result);

            // Quits if there is an error
            if (result.HasError)
                return;

            // Import the data
            m_queue.Import(result.Result.Queue);

            // Check the account has a character in training
            m_identity.Account.CharacterInTraining();

            // Check the character has room in skill queue
            if (IsTraining && (SkillQueue.EndTime < DateTime.UtcNow.AddHours(24)))
            {
                EveClient.Notifications.NotifySkillQueueRoomAvailable(this);
                return;
            }

            EveClient.Notifications.InvalidateSkillQueueRoomAvailability(this);
        }

        /// <summary>
        /// Processes the queried character's personal market orders.
        /// </summary>
        /// <param name="result"></param>
        /// <remarks>This method is sensitive to which market orders gets queried first</remarks>
        private void OnCharacterMarketOrdersUpdated(APIResult<SerializableAPIMarketOrders> result)
        {
            m_charOrdersUpdated = true;

            // Notify an error occurred
            if (ShouldNotifyError(result, APIMethods.MarketOrders))
                EveClient.Notifications.NotifyCharacterMarketOrdersError(this, result);

            // Add orders to list
            m_charOrdersAdded = AddOrders(result, m_corpOrdersAdded, IssuedFor.Character);

            // If character is in NPC corporation we switch the corp orders updated flag
            // to assure the character issued orders gets imported
            m_corpOrdersUpdated |= !m_corpMarketOrdersMonitor.Enabled;

            // Import the data if all queried and there are orders to import 
            if (m_corpOrdersUpdated)
                Import(m_orders);
        }

        /// <summary>
        /// Processes the queried character's corporation market orders.
        /// </summary>
        /// <param name="result"></param>
        /// <remarks>This method is sensitive to which market orders gets queried first</remarks>
        private void OnCorporationMarketOrdersUpdated(APIResult<SerializableAPIMarketOrders> result)
        {
            m_corpOrdersUpdated = true;

            // Character is not in NPC corporation
            if (!IsInNPCCorporation)
            {
                // We don't want to be notified about corp roles error
                if (result.CCPError != null && !result.CCPError.IsOrdersRelatedCorpRolesError)
                {
                    // Notify an error occurred
                    if (ShouldNotifyError(result, APIMethods.CorporationMarketOrders))
                        EveClient.Notifications.NotifyCorporationMarketOrdersError(this, result);
                }

                // Add orders to list
                m_corpOrdersAdded = AddOrders(result, m_charOrdersAdded, IssuedFor.Corporation);
            }

            // Import the data if all queried and there are orders to import
            if (m_charOrdersUpdated)
                Import(m_orders);
        }

        /// <summary>
        /// Processes the queried character's personal industry jobs.
        /// </summary>
        /// <param name="result"></param>
        /// <remarks>This method is sensitive to which "issued for" jobs gets queried first</remarks>
        private void OnCharacterJobsUpdated(APIResult<SerializableAPIIndustryJobs> result)
        {
            m_charJobsUpdated = true;

            // Notify an error occurred
            if (ShouldNotifyError(result, APIMethods.IndustryJobs))
                EveClient.Notifications.NotifyCharacterIndustryJobsError(this, result);

            // Add jobs to list
            m_charJobsAdded = AddJobs(result, m_corpJobsAdded, IssuedFor.Character);

            // If character is in NPC corporation we switch the corp jobs updated flag
            // to assure the character issued jobs gets imported
            m_corpJobsUpdated |= !m_corpIndustryJobsMonitor.Enabled;

            // Import the data if all queried and there are jobs to import 
            if (m_corpJobsUpdated && m_jobs.Count != 0)
                Import(m_jobs);
        }

        /// <summary>
        /// Processes the queried character's corporation industry jobs.
        /// </summary>
        /// <param name="result"></param>
        /// <remarks>This method is sensitive to which industry jobs gets queried first</remarks>
        private void OnCorporationJobsUpdated(APIResult<SerializableAPIIndustryJobs> result)
        {
            m_corpJobsUpdated = true;

            // Character is not in NPC corporation
            if (!IsInNPCCorporation)
            {
                // We don't want to be notified about corp roles error
                if (result.CCPError != null && !result.CCPError.IsJobsRelatedCorpRolesError)
                {
                    // Notify an error occurred
                    if (ShouldNotifyError(result, APIMethods.CorporationMarketOrders))
                        EveClient.Notifications.NotifyCorporationIndustryJobsError(this, result);
                }

                // Add jobs to list
                m_corpJobsAdded = AddJobs(result, m_charJobsAdded, IssuedFor.Corporation);
            }

            // Import the data if all queried and there are jobs to import
            if (m_charJobsUpdated && m_jobs.Count != 0)
                Import(m_jobs);
        }

        /// <summary>
        /// Processes the queried character's research points.
        /// </summary>
        /// <param name="result"></param>
        private void OnCharacterResearchPointsUpdated(APIResult<SerializableAPIResearch> result)
        {
            // Notify an error occured
            if (ShouldNotifyError(result, APIMethods.ResearchPoints))
                EveClient.Notifications.NotifyResearchPointsError(this, result);

            // Quits if there is an error
            if (result.HasError)
                return;

            // Import the data
            m_researchPoints.Import(result.Result.ResearchPoints);

            // Fires the event regarding research points update
            EveClient.OnCharacterResearchPointsChanged(this);
        }

        /// <summary>
        /// Processes the queried character's EVE mail messages.
        /// </summary>
        /// <param name="result"></param>
        private void OnCharacterEVEMailMessagesUpdated(APIResult<SerializableAPIMailMessages> result)
        {
            // Notify an error occured
            if (ShouldNotifyError(result, APIMethods.MailMessages))
                EveClient.Notifications.NotifyEVEMailMessagesError(this, result);

            // Quits if there is an error
            if (result.HasError)
                return;

            m_cachedAPIMailMessagesResult = result.Result;

            // Each time we import a new batch of EVE mail messages,
            // query the mailing lists (if it's time to) so that we are always up to date
            // and import the mail messages after we received the mailing lists
            if (DateTime.UtcNow > m_mailingListsNextUpdate)
            {
                QueryCharacterMailingList();
                return;
            }

            // Import the EVE mail messages
            ImportEVEMailMessages();
        }

        /// <summary>
        /// Processes the queried character's EVE mailing lists.
        /// </summary>
        /// <param name="result">The result.</param>
        private void OnCharacterMailingListUpdated(APIResult<SerializableAPIMailingLists> result)
        {
            m_mailingListsNextUpdate = result.CachedUntil;

            // Notify an error occured
            if (ShouldNotifyError(result, APIMethods.MailingLists))
                EveClient.Notifications.NotifyMailingListsError(this, result);

            // If there is no error deserialize the result
            if (!result.HasError)
                EVEMailingLists.Import(result.Result.MailingLists);

            // Whether we have the mailing list info or not
            // import the EVE mail messages
            ImportEVEMailMessages();
        }

        /// <summary>
        /// Processes the queried character's EVE notifications.
        /// </summary>
        /// <param name="result">The result.</param>
        private void OnCharacterEVENotificationsUpdated(APIResult<SerializableAPINotifications> result)
        {
            // Notify an error occured
            if (ShouldNotifyError(result, APIMethods.Notifications))
                EveClient.Notifications.NotifyEVENotificationsError(this, result);

            // Quits if there is an error
            if (result.HasError)
                return;

            // Import the data
            m_eveNotifications.Import(result.Result.Notifications);

            // Notify on new messages
            if (m_eveNotifications.NewNotifications != 0)
                EveClient.Notifications.NotifyNewEVENotifications(this, m_eveNotifications.NewNotifications);

            // Fires the event regarding EVE mail messages update
            EveClient.OnCharacterEVENotificationsUpdated(this);
        }
        #endregion


        # region Helper Methods

        /// <summary>
        /// Forces an update on the selected query monitor.
        /// </summary>
        /// <param name="queryMonitor">The query monitor.</param>
        public void ForceUpdate(IQueryMonitor queryMonitor)
        {
            var monitor = m_monitors[queryMonitor.Method] as IQueryMonitorEx;
            monitor.ForceUpdate(false);
        }

        /// <summary>
        /// Checks whether we should notify an error.
        /// </summary>
        /// <param name="result"></param>
        /// <param name="method"></param>
        /// <returns></returns>
        internal bool ShouldNotifyError(IAPIResult result, APIMethods method)
        {
            // Notify an error occurred
            if (result.HasError)
            {
                // Checks if EVE Backend Database is temporarily disabled
                if (result.EVEBackendDatabaseDisabled)
                    return false;

                if (m_errorNotifiedMethod != APIMethods.None)
                    return false;

                m_errorNotifiedMethod = method;
                return true;
            }

            // Removes the previous error notification
            if (m_errorNotifiedMethod == method)
            {
                EveClient.Notifications.InvalidateCharacterAPIError(this);
                m_errorNotifiedMethod = APIMethods.None;
            }
            return false;
        }

        /// <summary>
        /// Add the queried orders to a list.
        /// </summary>
        /// <param name="result"></param>
        /// <param name="ordersAdded"></param>
        /// <param name="issuedFor"></param>
        /// <returns>True if orders get added, false otherwise</returns>
        private bool AddOrders(APIResult<SerializableAPIMarketOrders> result, bool ordersAdded, IssuedFor issuedFor)
        {           
            // Add orders if there isn't an error
            if (result.HasError)
                return false;

            // Check to see if other market
            // orders have been added before
            if (!ordersAdded)
                m_orders.Clear();

            // Add orders in list
            result.Result.Orders.ForEach(x => x.IssuedFor = issuedFor);
            m_orders.AddRange(result.Result.Orders);

            return true;
        }

        /// <summary>
        /// Import the orders from both market orders querying.
        /// </summary>
        /// <param name="orders"></param>
        private void Import(List<SerializableOrderListItem> orders)
        {
            // Exclude orders that wheren't issued by this character
            var characterOrders = orders.Where(x => x.OwnerID == m_characterID);

            var endedOrders = new List<MarketOrder>();
            m_marketOrders.Import(characterOrders, endedOrders);

            NotifyOnEndedOrders(endedOrders);

            // Reset flags
            m_charOrdersUpdated = false;
            m_corpOrdersUpdated = false;
            m_charOrdersAdded = false;
            m_corpOrdersAdded = false;

            // Check the character has sufficient balance
            // for its buying orders or send a notification
            if (!HasSufficientBalance)
            {
                EveClient.Notifications.NotifyInsufficientBalance(this);
                return;
            }

            EveClient.Notifications.InvalidateInsufficientBalance(this);
        }

        /// <summary>
        /// Notify the user which orders has ended.
        /// </summary>
        /// <param name="endedOrders"></param>
        private void NotifyOnEndedOrders(List<MarketOrder> endedOrders)
        {
            // Sends a notification
            if (endedOrders.Count != 0)
                EveClient.Notifications.NotifyMarkerOrdersEnding(this, endedOrders);

            // Fires the event regarding market orders update
            EveClient.OnCharacterMarketOrdersChanged(this);
        }

        /// <summary>
        /// Add the queried jobs to a list.
        /// </summary>
        /// <param name="result"></param>
        /// <param name="jobsAdded"></param>
        /// <param name="issuedFor"></param>
        /// <returns>True if jobs get added, false otherwise</returns>
        private bool AddJobs(APIResult<SerializableAPIIndustryJobs> result, bool jobsAdded, IssuedFor issuedFor)
        {
            // Add orders if there isn't an error
            if (result.HasError)
                return false;

            // Check to see if other market
            // orders have been added before
            if (!jobsAdded)
                m_jobs.Clear();

            // Add jobs in list
            result.Result.Jobs.ForEach(x => x.IssuedFor = issuedFor);
            m_jobs.AddRange(result.Result.Jobs);

            return true;
        }

        /// <summary>
        /// Import the jobs from both industry jobs querying.
        /// </summary>
        /// <param name="jobs"></param>
        private void Import(List<SerializableJobListItem> jobs)
        {
            // Exclude jobs that wheren't issued by this character
            var characterJobs = jobs.Where(x => x.InstallerID == m_characterID);

            m_industryJobs.Import(characterJobs);
            
            // Fires the event regarding industry jobs update
            EveClient.OnCharacterIndustryJobsChanged(this);

            // Reset flags
            m_charJobsUpdated = false;
            m_corpJobsUpdated = false;
            m_charJobsAdded = false;
            m_corpJobsAdded = false;
        }

        /// <summary>
        /// Imports the EVE mail messages.
        /// </summary>
        private void ImportEVEMailMessages()
        {
            // Import the data
            m_eveMailMessages.Import(m_cachedAPIMailMessagesResult.Messages);

            // Notify on new messages
            if (m_eveMailMessages.NewMessages != 0)
                EveClient.Notifications.NotifyNewEVEMailMessages(this, m_eveMailMessages.NewMessages);

            // Fires the event regarding EVE mail messages update
            EveClient.OnCharacterEVEMailMessagesUpdated(this);
        }

        #endregion
    }
}
