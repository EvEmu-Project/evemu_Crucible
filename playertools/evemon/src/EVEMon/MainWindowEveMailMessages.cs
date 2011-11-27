using System;
using System.Data;
using System.Text;
using System.Linq;
using System.Drawing;
using System.Windows.Forms;
using System.ComponentModel;
using System.Collections.Generic;

using EVEMon.Common;
using EVEMon.Controls;
using EVEMon.Common.Data;
using EVEMon.Common.SettingsObjects;

namespace EVEMon
{
    public partial class MainWindowEveMailMessagesList : UserControl, IGroupingListView
    {
        #region Fields

        private List<EveMailMessagesColumnSettings> m_columns = new List<EveMailMessagesColumnSettings>();
        private readonly List<EveMailMessage> m_list = new List<EveMailMessage>();

        private EVEMailMessagesGrouping m_grouping;
        private EveMailMessagesColumn m_sortCriteria;
        private ReadingPanePositioning m_panePosition;

        private string m_textFilter = String.Empty;
        private bool m_sortAscending = false;
        private bool m_columnsChanged;
        private bool m_isUpdatingColumns;
        private bool m_init;

        #endregion


        #region Constructor

        /// <summary>
        /// Constructor.
        /// </summary>
        public MainWindowEveMailMessagesList()
        {
            InitializeComponent();

            eveMailReadingPane.HidePane();
            splitContainerMailMessages.Visible = false;
            lvMailMessages.AllowColumnReorder = true;
            lvMailMessages.Columns.Clear();

            noEVEMailMessagesLabel.Font = FontFactory.GetFont("Tahoma", 11.25F, FontStyle.Bold);

            DoubleBuffered = true;
            ListViewHelper.EnableDoubleBuffer(lvMailMessages);

            EveClient.TimerTick += EveClient_TimerTick;
            EveClient.CharacterEVEMailMessagesUpdated += EveClient_CharacterEVEMailMessagesUpdated;
            EveClient.CharacterEVEMailBodyDownloaded += EveClient_CharacterEVEMailBodyDownloaded;
            Disposed += OnDisposed;
        }

        #endregion


        #region Properties

        /// <summary>
        /// Gets the character associated with this monitor.
        /// </summary>
        public Character Character
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the text filter.
        /// </summary>
        public string TextFilter
        {
            get { return m_textFilter; }
            set
            {
                m_textFilter = value;
                if (m_init)
                    UpdateColumns();
            }
        }

        /// <summary>
        /// Gets or sets the grouping mode.
        /// </summary>
        public Enum Grouping
        {
            get { return m_grouping; }
            set
            {
                m_grouping = (EVEMailMessagesGrouping)value;
                if (m_init)
                    UpdateColumns();
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public ReadingPanePositioning PanePosition
        {
            get { return m_panePosition; }
            set
            {
                m_panePosition = value;
                UpdatePanePosition();
            }
        }

        /// <summary>
        /// Gets or sets the enumeration of EVE mail messages to display.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public IEnumerable<EveMailMessage> EVEMailMessages
        {
            get
            {
                foreach (var eveMailMessage in m_list)
                {
                    yield return eveMailMessage;
                }
            }
            set
            {
                m_list.Clear();
                if (value == null)
                    return;

                m_list.AddRange(value);
            }
        }

        /// <summary>
        /// Gets or sets the settings used for columns.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public IEnumerable<EveMailMessagesColumnSettings> Columns
        {
            get
            {
                // Add the visible columns; matching the display order
                var newColumns = new List<EveMailMessagesColumnSettings>();
                foreach (var header in lvMailMessages.Columns.Cast<ColumnHeader>().OrderBy(x => x.DisplayIndex))
                {
                    var columnSetting = m_columns.First(x => x.Column == (EveMailMessagesColumn)header.Tag);
                    if (columnSetting.Width != -1)
                        columnSetting.Width = header.Width;

                    newColumns.Add(columnSetting);
                }

                // Then add the other columns
                foreach (var column in m_columns.Where(x => !x.Visible))
                {
                    newColumns.Add(column);
                }

                return newColumns;
            }
            set
            {
                m_columns.Clear();
                if (value != null)
                    m_columns.AddRange(value.Select(x => x.Clone()));

                if (m_init)
                    UpdateColumns();
            }
        }

        #endregion


        # region Inherited Events

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.TimerTick -= EveClient_TimerTick;
            EveClient.CharacterEVEMailMessagesUpdated -= EveClient_CharacterEVEMailMessagesUpdated;
            EveClient.CharacterEVEMailBodyDownloaded -= EveClient_CharacterEVEMailBodyDownloaded;
            Disposed -= OnDisposed;
        }

        /// <summary>
        /// When the control becomes visible again, we update the content.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            if (DesignMode || this.IsDesignModeHosted() || Character == null)
                return;

            base.OnVisibleChanged(e);

            if (!Visible)
                return;

            // Prevents the properties to call UpdateColumns() till we set all properties
            m_init = false;

            var ccpCharacter = Character as CCPCharacter;
            EVEMailMessages = (ccpCharacter == null ? null : ccpCharacter.EVEMailMessages);
            Columns = Settings.UI.MainWindow.EVEMailMessages.Columns;
            Grouping = (Character == null ? EVEMailMessagesGrouping.State : Character.UISettings.EVEMailMessagesGroupBy);
            PanePosition = Settings.UI.MainWindow.EVEMailMessages.ReadingPanePosition;

            UpdateColumns();
            m_init = true;

            UpdateContent();
        }

        # endregion


        #region Update Methods

        /// <summary>
        /// <summary>
        /// Updates the columns.
        /// </summary>
        public void UpdateColumns()
        {
            lvMailMessages.BeginUpdate();
            m_isUpdatingColumns = true;

            try
            {
                lvMailMessages.Columns.Clear();

                foreach (var column in m_columns.Where(x => x.Visible))
                {
                    var header = lvMailMessages.Columns.Add(column.Column.GetHeader(), column.Column.GetHeader(), column.Width);
                    header.Tag = (object)column.Column;
                }

                // We update the content
                UpdateContent();

                // Force the auto-resize of the columns with -1 width
                var resizeStyle = (lvMailMessages.Items.Count == 0 ?
                    ColumnHeaderAutoResizeStyle.HeaderSize :
                    ColumnHeaderAutoResizeStyle.ColumnContent);

                int index = 0;
                foreach (var column in m_columns.Where(x => x.Visible))
                {
                    if (column.Width == -1)
                        lvMailMessages.AutoResizeColumn(index, resizeStyle);

                    index++;
                }
            }
            finally
            {
                lvMailMessages.EndUpdate();
                m_isUpdatingColumns = false;
            }
        }

        /// <summary>
        /// Updates the content of the listview.
        /// </summary>
        public void UpdateContent()
        {
            // Returns if not visible
            if (!Visible)
                return;

            // Store the selected item (if any) to restore it after the update
            int selectedItem = (lvMailMessages.SelectedItems.Count > 0 ?
                                lvMailMessages.SelectedItems[0].Tag.GetHashCode() : 0);

            lvMailMessages.BeginUpdate();
            splitContainerMailMessages.Visible = false;
            try
            {
                var text = m_textFilter.ToLowerInvariant();
                var eveMailMessages = m_list.Where(x => IsTextMatching(x, text));

                UpdateSort();

                switch (m_grouping)
                {
                    case EVEMailMessagesGrouping.State:
                        var groups0 = eveMailMessages.GroupBy(x => x.State).OrderBy(x => (int)x.Key);
                        UpdateContent(groups0);
                        break;
                    case EVEMailMessagesGrouping.StateDesc:
                        var groups1 = eveMailMessages.GroupBy(x => x.State).OrderByDescending(x => (int)x.Key);
                        UpdateContent(groups1);
                        break;
                    case EVEMailMessagesGrouping.SentDate:
                        var groups2 = eveMailMessages.GroupBy(x => x.SentDate.Date).OrderBy(x => x.Key);
                        UpdateContent(groups2);
                        break;
                    case EVEMailMessagesGrouping.SentDateDesc:
                        var groups3 = eveMailMessages.GroupBy(x => x.SentDate.Date).OrderByDescending(x => x.Key);
                        UpdateContent(groups3);
                        break;
                    case EVEMailMessagesGrouping.Sender:
                        var groups4 = eveMailMessages.GroupBy(x => x.Sender).OrderBy(x => x.Key);
                        UpdateContent(groups4);
                        break;
                    case EVEMailMessagesGrouping.SenderDesc:
                        var groups5 = eveMailMessages.GroupBy(x => x.Sender).OrderByDescending(x => x.Key);
                        UpdateContent(groups5);
                        break;
                    case EVEMailMessagesGrouping.Subject:
                        var groups6 = eveMailMessages.GroupBy(x => x.Title).OrderBy(x => x.Key);
                        UpdateContent(groups6);
                        break;
                    case EVEMailMessagesGrouping.SubjectDesc:
                        var groups7 = eveMailMessages.GroupBy(x => x.Title).OrderByDescending(x => x.Key);
                        UpdateContent(groups7);
                        break;
                    case EVEMailMessagesGrouping.Recipient:
                        var groups8 = eveMailMessages.GroupBy(x => x.ToCharacters[0]).OrderBy(x => x.Key);
                        UpdateContent(groups8);
                        break;
                    case EVEMailMessagesGrouping.RecipientDesc:
                        var groups9 = eveMailMessages.GroupBy(x => x.ToCharacters[0]).OrderByDescending(x => x.Key);
                        UpdateContent(groups9);
                        break;
                    case EVEMailMessagesGrouping.CorpOrAlliance:
                        var groups10 = eveMailMessages.GroupBy(x => x.ToCorpOrAlliance).OrderBy(x => x.Key);
                        UpdateContent(groups10);
                        break;
                    case EVEMailMessagesGrouping.CorpOrAllianceDesc:
                        var groups11 = eveMailMessages.GroupBy(x => x.ToCorpOrAlliance).OrderByDescending(x => x.Key);
                        UpdateContent(groups11);
                        break;
                    case EVEMailMessagesGrouping.MailingList:
                        var groups12 = eveMailMessages.GroupBy(x => x.ToMailingLists[0]).OrderBy(x => x.Key);
                        UpdateContent(groups12);
                        break;
                    case EVEMailMessagesGrouping.MailingListDesc:
                        var groups13 = eveMailMessages.GroupBy(x => x.ToMailingLists[0]).OrderByDescending(x => x.Key);
                        UpdateContent(groups13);
                        break;
                }

                // Restore the selected item (if any)
                if (selectedItem > 0)
                {
                    foreach (ListViewItem lvItem in lvMailMessages.Items)
                    {
                        if (lvItem.Tag.GetHashCode() == selectedItem)
                            lvItem.Selected = true;
                    }
                }

                // Display or hide the "no EVE mail messages" label
                if (m_init)
                {
                    noEVEMailMessagesLabel.Visible = eveMailMessages.IsEmpty();
                    splitContainerMailMessages.Visible = !eveMailMessages.IsEmpty();
                }
            }
            finally
            {
                lvMailMessages.EndUpdate();
            }
        }

        /// <summary>
        /// Updates the content of the listview.
        /// </summary>
        /// <typeparam name="TKey"></typeparam>
        /// <param name="groups"></param>
        private void UpdateContent<TKey>(IEnumerable<IGrouping<TKey, EveMailMessage>> groups)
        {
            lvMailMessages.Items.Clear();
            lvMailMessages.Groups.Clear();

            // Add the groups
            foreach (var group in groups)
            {
                string groupText = String.Empty;
                if (group.Key is EVEMailState)
                {
                    groupText = ((EVEMailState)(Object)group.Key).GetHeader();
                }
                else if (group.Key is DateTime)
                {
                    groupText = ((DateTime)(Object)group.Key).ToShortDateString();
                }
                else
                {
                    groupText = group.Key.ToString();
                }

                var listGroup = new ListViewGroup(groupText);
                lvMailMessages.Groups.Add(listGroup);

                // Add the items in every group
                foreach (var eveMailMessage in group)
                {
                    if (String.IsNullOrEmpty(eveMailMessage.MessageID.ToString()))
                        continue;

                    var item = new ListViewItem(eveMailMessage.Sender, listGroup);
                    item.UseItemStyleForSubItems = false;
                    item.Tag = eveMailMessage;

                    // Add enough subitems to match the number of columns
                    while (item.SubItems.Count < lvMailMessages.Columns.Count + 1)
                    {
                        item.SubItems.Add(String.Empty);
                    }

                    // Creates the subitems
                    for (int i = 0; i < lvMailMessages.Columns.Count; i++)
                    {
                        var header = lvMailMessages.Columns[i];
                        var column = (EveMailMessagesColumn)header.Tag;
                        SetColumn(eveMailMessage, item.SubItems[i], column);
                    }

                    lvMailMessages.Items.Add(item);
                }
            }
        }

        /// <summary>
        /// Updates the item sorter.
        /// </summary>
        private void UpdateSort()
        {
            lvMailMessages.ListViewItemSorter = new ListViewItemComparerByTag<EveMailMessage>(
                new EveMailMessagesComparer(m_sortCriteria, m_sortAscending));

            UpdateSortVisualFeedback();
        }

        /// <summary>
        /// Updates the sort feedback (the arrow on the header).
        /// </summary>
        private void UpdateSortVisualFeedback()
        {
            for (int i = 0; i < lvMailMessages.Columns.Count; i++)
            {
                var column = (EveMailMessagesColumn)lvMailMessages.Columns[i].Tag;
                if (m_sortCriteria == column)
                {
                    lvMailMessages.Columns[i].ImageIndex = (m_sortAscending ? 0 : 1);
                }
                else
                {
                    lvMailMessages.Columns[i].ImageIndex = 2;
                }
            }
        }

        /// <summary>
        /// Updates the listview sub-item.
        /// </summary>
        /// <param name="order"></param>
        /// <param name="item"></param>
        /// <param name="column"></param>
        private void SetColumn(EveMailMessage eveMailMessage, ListViewItem.ListViewSubItem item, EveMailMessagesColumn column)
        {
            switch (column)
            {
                case EveMailMessagesColumn.SenderName:
                    item.Text = eveMailMessage.Sender;
                    break;

                case EveMailMessagesColumn.Title:
                    item.Text = eveMailMessage.Title;
                    break;

                case EveMailMessagesColumn.SentDate:
                    item.Text = String.Format(CultureConstants.DefaultCulture,
                                                "{0:ddd} {0:G}", eveMailMessage.SentDate.ToLocalTime());
                    break;

                case EveMailMessagesColumn.ToCharacters:
                    item.Text = string.Join(", ", eveMailMessage.ToCharacters);
                    break;

                case EveMailMessagesColumn.ToCorpOrAlliance:
                    item.Text = eveMailMessage.ToCorpOrAlliance;
                    break;

                case EveMailMessagesColumn.ToMailingList:
                    item.Text = string.Join(", ", eveMailMessage.ToMailingLists);
                    break;

                default:
                    //return;
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        /// Updates the pane position.
        /// </summary>
        private void UpdatePanePosition()
        {
            switch (PanePosition)
            {
                default:
                case ReadingPanePositioning.Off:
                    splitContainerMailMessages.Panel2Collapsed = true;
                    break;
                case ReadingPanePositioning.Bottom:
                    splitContainerMailMessages.Orientation = Orientation.Horizontal;
                    splitContainerMailMessages.Panel2Collapsed = false;
                    break;
                case ReadingPanePositioning.Right:
                    splitContainerMailMessages.Orientation = Orientation.Vertical;
                    splitContainerMailMessages.Panel2Collapsed = false;
                    break;
            }
        }

        #endregion


        #region Helper Methods

        /// <summary>
        /// Checks the given text matches the item.
        /// </summary>
        /// <param name="x">The x.</param>
        /// <param name="text">The text.</param>
        /// <returns>
        /// 	<c>true</c> if [is text matching] [the specified x]; otherwise, <c>false</c>.
        /// </returns>
        private bool IsTextMatching(EveMailMessage x, string text)
        {
            if (String.IsNullOrEmpty(text)
                || x.Sender.ToLowerInvariant().Contains(text)
                || x.Title.ToLowerInvariant().Contains(text)
                || x.ToCorpOrAlliance.ToLowerInvariant().Contains(text)
                || x.ToCharacters.Any(y => y.ToLowerInvariant().Contains(text))
                || (x.EVEMailBody != null && x.EVEMailBody.BodyText.ToLowerInvariant().Contains(text)))
                
                return true;

            return false;
        }

        /// <summary>
        /// Called when selection changed.
        /// </summary>
        private void OnSelectionChanged()
        {
            if (lvMailMessages.SelectedItems.Count == 0)
            {
                eveMailReadingPane.HidePane();
                return;
            }

            EveMailMessage selectedObject = lvMailMessages.SelectedItems[0].Tag as EveMailMessage;

            // If we haven't done it yet, download the mail body
            if (selectedObject.EVEMailBody == null)
                selectedObject.GetMailBody();

            // In case there was an error, hide the pane and quit
            if (selectedObject.EVEMailBody == null)
            {
                eveMailReadingPane.HidePane();
                return;
            }

            eveMailReadingPane.SelectedObject = selectedObject;
        }

        #endregion


        #region Local Event Handlers

        /// <summary>
        /// On resize, updates the controls visibility.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MainWindowEVEMailMessagesList_Resize(object sender, EventArgs e)
        {
            if (!m_init)
                return;

            UpdateContent();
        }

        /// <summary>
        /// When the selection update timer ticks, we process the changes caused by a selection change.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void timer_Tick(object sender, EventArgs e)
        {
            timer.Enabled = false;
            OnSelectionChanged();
        }

        /// <summary>
        /// When the user selects another item, we do not immediately process the change but rather delay it through a timer.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.ListViewItemSelectionChangedEventArgs"/> instance containing the event data.</param>
        private void lvMailMessages_ItemSelectionChanged(object sender, ListViewItemSelectionChangedEventArgs e)
        {
            if (timer.Enabled)
                return;

            timer.Interval = 100;
            timer.Enabled = true;
            timer.Start();
        }

        /// <summary>
        /// Opens a window form to display the EVE mail body.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void lvMailMessages_DoubleClick(object sender, EventArgs e)
        {
            var item = (ListViewItem)lvMailMessages.SelectedItems[0];
            var message = (EveMailMessage)item.Tag;

            // Quit if we haven't downloaded the mail message body yet
            if (message.EVEMailBody == null)
                return;

            // Show or bring to front if a window with the same EVE mail message already exists
            WindowsFactory<EveMessageWindow>.ShowByTag(message);
        }

        /// <summary>
        /// On column reorder we update the settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void lvMailMessages_ColumnReordered(object sender, ColumnReorderedEventArgs e)
        {
            m_columnsChanged = true;
        }

        /// <summary>
        /// When the user manually resizes a column, we make sure to update the column preferences.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void lvMailMessages_ColumnWidthChanged(object sender, ColumnWidthChangedEventArgs e)
        {
            if (m_isUpdatingColumns || m_columns.Count <= e.ColumnIndex)
                return;

            m_columns[e.ColumnIndex].Width = lvMailMessages.Columns[e.ColumnIndex].Width;
            m_columnsChanged = true;
        }

        /// <summary>
        /// When the user clicks a column header, we update the sorting.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lvMailMessages_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            var column = (EveMailMessagesColumn)lvMailMessages.Columns[e.Column].Tag;
            if (m_sortCriteria == column)
            {
                m_sortAscending = !m_sortAscending;
            }
            else
            {
                m_sortCriteria = column;
                m_sortAscending = true;
            }

            UpdateContent();
        }

        # endregion


        #region Global Events

        /// <summary>
        /// On timer tick, we update the column settings if any changes have been made to them.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_TimerTick(object sender, EventArgs e)
        {
            if (m_columnsChanged)
            {
                Settings.UI.MainWindow.EVEMailMessages.Columns = Columns.Select(x => x.Clone()).ToArray();

                // Recreate the columns
                Columns = Settings.UI.MainWindow.EVEMailMessages.Columns;
            }

            m_columnsChanged = false;
        }
        
        /// <summary>
        /// When the mail messages change update the list.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterEVEMailMessagesUpdated(object sender, CharacterChangedEventArgs e)
        {
            var ccpCharacter = Character as CCPCharacter;
            if (e.Character != ccpCharacter)
                return;

            EVEMailMessages = ccpCharacter.EVEMailMessages;
            UpdateColumns();
        }

        /// <summary>
        /// When the mail message body gets downloaded update the reading pane.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterEVEMailBodyDownloaded(object sender, CharacterChangedEventArgs e)
        {
            var ccpCharacter = Character as CCPCharacter;
            if (e.Character != ccpCharacter)
                return;

            OnSelectionChanged();
        }
        # endregion
    }
}
