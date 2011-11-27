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
    public partial class MainWindowEveNotificationsList : UserControl, IGroupingListView
    {
        #region Fields

        private List<EveNotificationsColumnSettings> m_columns = new List<EveNotificationsColumnSettings>();
        private readonly List<EveNotification> m_list = new List<EveNotification>();

        private EVENotificationsGrouping m_grouping;
        private EveNotificationsColumn m_sortCriteria;
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
        public MainWindowEveNotificationsList()
        {
            InitializeComponent();

            eveNotificationReadingPane.HidePane();
            splitContainerNotifications.Visible = false;
            lvNotifications.AllowColumnReorder = true;
            lvNotifications.Columns.Clear();

            noEVENotificationsLabel.Font = FontFactory.GetFont("Tahoma", 11.25F, FontStyle.Bold);

            DoubleBuffered = true;
            ListViewHelper.EnableDoubleBuffer(lvNotifications);

            EveClient.TimerTick += EveClient_TimerTick;
            EveClient.CharacterEVENotificationsUpdated += EveClient_CharacterEVENotificationsUpdated;
            EveClient.CharacterEVENotificationTextDownloaded += EveClient_CharacterEVENotificationTextDownloaded;
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
                m_grouping = (EVENotificationsGrouping)value;
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
        public IEnumerable<EveNotification> EVENotifications
        {
            get
            {
                foreach (var eveNotification in m_list)
                {
                    yield return eveNotification;
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
        public IEnumerable<EveNotificationsColumnSettings> Columns
        {
            get
            {
                // Add the visible columns; matching the display order
                var newColumns = new List<EveNotificationsColumnSettings>();
                foreach (var header in lvNotifications.Columns.Cast<ColumnHeader>().OrderBy(x => x.DisplayIndex))
                {
                    var columnSetting = m_columns.First(x => x.Column == (EveNotificationsColumn)header.Tag);
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
            EveClient.CharacterEVENotificationsUpdated -= EveClient_CharacterEVENotificationsUpdated;
            EveClient.CharacterEVENotificationTextDownloaded -= EveClient_CharacterEVENotificationTextDownloaded;
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
            EVENotifications = (ccpCharacter == null ? null : ccpCharacter.EVENotifications);
            Columns = Settings.UI.MainWindow.EVENotifications.Columns;
            Grouping = (Character == null ? EVENotificationsGrouping.Type : Character.UISettings.EVENotificationsGroupBy);
            PanePosition = Settings.UI.MainWindow.EVENotifications.ReadingPanePosition;

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
            lvNotifications.BeginUpdate();
            m_isUpdatingColumns = true;

            try
            {
                lvNotifications.Columns.Clear();

                foreach (var column in m_columns.Where(x => x.Visible))
                {
                    var header = lvNotifications.Columns.Add(column.Column.GetHeader(), column.Column.GetHeader(), column.Width);
                    header.Tag = (object)column.Column;
                }

                // We update the content
                UpdateContent();

                // Force the auto-resize of the columns with -1 width
                var resizeStyle = (lvNotifications.Items.Count == 0 ?
                    ColumnHeaderAutoResizeStyle.HeaderSize :
                    ColumnHeaderAutoResizeStyle.ColumnContent);

                int index = 0;
                foreach (var column in m_columns.Where(x => x.Visible))
                {
                    if (column.Width == -1)
                        lvNotifications.AutoResizeColumn(index, resizeStyle);

                    index++;
                }
            }
            finally
            {
                lvNotifications.EndUpdate();
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
            int selectedItem = (lvNotifications.SelectedItems.Count > 0 ?
                                lvNotifications.SelectedItems[0].Tag.GetHashCode() : 0);

            lvNotifications.BeginUpdate();
            splitContainerNotifications.Visible = false;
            try
            {
                var text = m_textFilter.ToLowerInvariant();
                var eveNotifications = m_list.Where(x => IsTextMatching(x, text));

                UpdateSort();

                switch (m_grouping)
                {
                    case EVENotificationsGrouping.Type:
                        var groups0 = eveNotifications.GroupBy(x => x.Type).OrderBy(x => x.Key);
                        UpdateContent(groups0);
                        break;
                    case EVENotificationsGrouping.TypeDesc:
                        var groups1 = eveNotifications.GroupBy(x => x.Type).OrderByDescending(x => x.Key);
                        UpdateContent(groups1);
                        break;
                    case EVENotificationsGrouping.SentDate:
                        var groups2 = eveNotifications.GroupBy(x => x.SentDate.Date).OrderBy(x => x.Key);
                        UpdateContent(groups2);
                        break;
                    case EVENotificationsGrouping.SentDateDesc:
                        var groups3 = eveNotifications.GroupBy(x => x.SentDate.Date).OrderByDescending(x => x.Key);
                        UpdateContent(groups3);
                        break;
                    case EVENotificationsGrouping.Sender:
                        var groups4 = eveNotifications.GroupBy(x => x.Sender).OrderBy(x => x.Key);
                        UpdateContent(groups4);
                        break;
                    case EVENotificationsGrouping.SenderDesc:
                        var groups5 = eveNotifications.GroupBy(x => x.Sender).OrderByDescending(x => x.Key);
                        UpdateContent(groups5);
                        break;
                }

                // Restore the selected item (if any)
                if (selectedItem > 0)
                {
                    foreach (ListViewItem lvItem in lvNotifications.Items)
                    {
                        if (lvItem.Tag.GetHashCode() == selectedItem)
                            lvItem.Selected = true;
                    }
                }

                // Display or hide the "no EVE mail messages" label
                if (m_init)
                {
                    noEVENotificationsLabel.Visible = eveNotifications.IsEmpty();
                    splitContainerNotifications.Visible = !eveNotifications.IsEmpty();
                }
            }
            finally
            {
                lvNotifications.EndUpdate();
            }
        }

        /// <summary>
        /// Updates the content of the listview.
        /// </summary>
        /// <typeparam name="TKey"></typeparam>
        /// <param name="groups"></param>
        private void UpdateContent<TKey>(IEnumerable<IGrouping<TKey, EveNotification>> groups)
        {
            lvNotifications.Items.Clear();
            lvNotifications.Groups.Clear();

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
                lvNotifications.Groups.Add(listGroup);

                // Add the items in every group
                foreach (var eveNotification in group)
                {
                    if (String.IsNullOrEmpty(eveNotification.NotificationID.ToString()))
                        continue;

                    var item = new ListViewItem(eveNotification.Sender, listGroup);
                    item.UseItemStyleForSubItems = false;
                    item.Tag = eveNotification;

                    // Add enough subitems to match the number of columns
                    while (item.SubItems.Count < lvNotifications.Columns.Count + 1)
                    {
                        item.SubItems.Add(String.Empty);
                    }

                    // Creates the subitems
                    for (int i = 0; i < lvNotifications.Columns.Count; i++)
                    {
                        var header = lvNotifications.Columns[i];
                        var column = (EveNotificationsColumn)header.Tag;
                        SetColumn(eveNotification, item.SubItems[i], column);
                    }

                    lvNotifications.Items.Add(item);
                }
            }
        }

        /// <summary>
        /// Updates the item sorter.
        /// </summary>
        private void UpdateSort()
        {
            lvNotifications.ListViewItemSorter = new ListViewItemComparerByTag<EveNotification>(
                new EveNotificationsComparer(m_sortCriteria, m_sortAscending));

            UpdateSortVisualFeedback();
        }

        /// <summary>
        /// Updates the sort feedback (the arrow on the header).
        /// </summary>
        private void UpdateSortVisualFeedback()
        {
            for (int i = 0; i < lvNotifications.Columns.Count; i++)
            {
                var column = (EveNotificationsColumn)lvNotifications.Columns[i].Tag;
                if (m_sortCriteria == column)
                {
                    lvNotifications.Columns[i].ImageIndex = (m_sortAscending ? 0 : 1);
                }
                else
                {
                    lvNotifications.Columns[i].ImageIndex = 2;
                }
            }
        }

        /// <summary>
        /// Updates the listview sub-item.
        /// </summary>
        /// <param name="order"></param>
        /// <param name="item"></param>
        /// <param name="column"></param>
        private void SetColumn(EveNotification eveNotification, ListViewItem.ListViewSubItem item, EveNotificationsColumn column)
        {
            switch (column)
            {
                case EveNotificationsColumn.SenderName:
                    item.Text = eveNotification.Sender;
                    break;

                case EveNotificationsColumn.Type:
                    item.Text = eveNotification.Type;
                    break;

                case EveNotificationsColumn.SentDate:
                    item.Text = String.Format(CultureConstants.DefaultCulture,
                                                "{0:ddd} {0:G}", eveNotification.SentDate.ToLocalTime());
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
                    splitContainerNotifications.Panel2Collapsed = true;
                    break;
                case ReadingPanePositioning.Bottom:
                    splitContainerNotifications.Orientation = Orientation.Horizontal;
                    splitContainerNotifications.Panel2Collapsed = false;
                    break;
                case ReadingPanePositioning.Right:
                    splitContainerNotifications.Orientation = Orientation.Vertical;
                    splitContainerNotifications.Panel2Collapsed = false;
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
        private bool IsTextMatching(EveNotification x, string text)
        {
            if (String.IsNullOrEmpty(text)
                || x.Sender.ToLowerInvariant().Contains(text)
                || x.Type.ToLowerInvariant().Contains(text)
                || (x.EVENotificationText != null && x.EVENotificationText.NotificationText.ToLowerInvariant().Contains(text)))

                return true;

            return false;
        }

        /// <summary>
        /// Called when selection changed.
        /// </summary>
        private void OnSelectionChanged()
        {
            if (lvNotifications.SelectedItems.Count == 0)
            {
                eveNotificationReadingPane.HidePane();
                return;
            }

            EveNotification selectedObject = lvNotifications.SelectedItems[0].Tag as EveNotification;

            // If we haven't done it yet, download the mail body
            if (selectedObject.EVENotificationText == null)
                selectedObject.GetNotificationText();

            // In case there was an error, hide the pane and quit
            if (selectedObject.EVENotificationText == null)
            {
                eveNotificationReadingPane.HidePane();
                return;
            }

            eveNotificationReadingPane.SelectedObject = selectedObject;
        }

        #endregion


        #region Local Event Handlers

        /// <summary>
        /// On resize, updates the controls visibility.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MainWindowEVENotificationsList_Resize(object sender, EventArgs e)
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
        private void lvNotifications_ItemSelectionChanged(object sender, ListViewItemSelectionChangedEventArgs e)
        {
            if (timer.Enabled)
                return;

            timer.Interval = 100;
            timer.Enabled = true;
            timer.Start();
        }

        /// <summary>
        /// Opens a window form to display the EVE notification.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void lvNotifications_DoubleClick(object sender, EventArgs e)
        {
            var item = (ListViewItem)lvNotifications.SelectedItems[0];
            var notification = (EveNotification)item.Tag;

            // Quit if we haven't downloaded the notification text yet
            if (notification.EVENotificationText == null)
                return;

            // Show or bring to front if a window with the same EVE notification already exists
            WindowsFactory<EveMessageWindow>.ShowByTag(notification);
        }

        /// <summary>
        /// On column reorder we update the settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void lvNotifications_ColumnReordered(object sender, ColumnReorderedEventArgs e)
        {
            m_columnsChanged = true;
        }

        /// <summary>
        /// When the user manually resizes a column, we make sure to update the column preferences.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void lvNotifications_ColumnWidthChanged(object sender, ColumnWidthChangedEventArgs e)
        {
            if (m_isUpdatingColumns || m_columns.Count <= e.ColumnIndex)
                return;

            m_columns[e.ColumnIndex].Width = lvNotifications.Columns[e.ColumnIndex].Width;
            m_columnsChanged = true;
        }

        /// <summary>
        /// When the user clicks a column header, we update the sorting.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lvNotifications_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            var column = (EveNotificationsColumn)lvNotifications.Columns[e.Column].Tag;
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
                Settings.UI.MainWindow.EVENotifications.Columns = Columns.Select(x => x.Clone()).ToArray();

                // Recreate the columns
                Columns = Settings.UI.MainWindow.EVENotifications.Columns;
            }

            m_columnsChanged = false;
        }

        /// <summary>
        /// When the notifications change update the list.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterEVENotificationsUpdated(object sender, CharacterChangedEventArgs e)
        {
            var ccpCharacter = Character as CCPCharacter;
            if (e.Character != ccpCharacter)
                return;

            EVENotifications = ccpCharacter.EVENotifications;
            UpdateColumns();
        }

        /// <summary>
        /// When the notification text gets downloaded update the reading pane.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterEVENotificationTextDownloaded(object sender, CharacterChangedEventArgs e)
        {
            var ccpCharacter = Character as CCPCharacter;
            if (e.Character != ccpCharacter)
                return;

            OnSelectionChanged();
        }
        # endregion
    }
}
