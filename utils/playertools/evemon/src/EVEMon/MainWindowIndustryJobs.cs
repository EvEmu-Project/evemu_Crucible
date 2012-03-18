using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Data;
using EVEMon.Common.SettingsObjects;
using EVEMon.Controls;

namespace EVEMon
{
    public partial class MainWindowIndustryJobsList : UserControl, IGroupingListView
    {
        private List<IndustryJobColumnSettings> m_columns = new List<IndustryJobColumnSettings>();
        private readonly List<IndustryJob> m_list = new List<IndustryJob>();

        private IndustryJobGrouping m_grouping;
        private IndustryJobColumn m_sortCriteria;
        private IssuedFor m_showIssuedFor;
        private Character m_character;

        private string m_textFilter = String.Empty;
        private bool m_sortAscending = true;

        private bool m_hideInactive;
        private bool m_isUpdatingColumns;
        private bool m_columnsChanged;
        private bool m_init;

        private int m_displayIndexTTC;

        // Panel info variables
        private int m_skillBasedManufacturingJobs, m_skillBasedResearchingJobs;
        private int m_remoteManufacturingRange, m_remoteResearchingRange;
        private int m_activeManufJobsIssuedForCharacterCount, m_activeManufJobsIssuedForCorporationCount;
        private int m_activeResearchJobsIssuedForCharacterCount, m_activeResearchJobsIssuedForCorporationCount;


        # region Constructor
        
        public MainWindowIndustryJobsList()
        {
            InitializeComponent();
            InitializeExpandablePanelControls();

            lvJobs.Visible = false;
            lvJobs.AllowColumnReorder = true;
            lvJobs.Columns.Clear();

            noJobsLabel.Font = FontFactory.GetFont("Tahoma", 11.25F, FontStyle.Bold);
            industryExpPanelControl.Font = FontFactory.GetFont("Tahoma", 8.25f);
            industryExpPanelControl.Visible = false;

            DoubleBuffered = true;
            ListViewHelper.EnableDoubleBuffer(lvJobs);

            lvJobs.ColumnClick += lvJobs_ColumnClick;
            lvJobs.KeyDown += lvJobs_KeyDown;
            lvJobs.ColumnWidthChanged += lvJobs_ColumnWidthChanged;
            lvJobs.ColumnReordered += lvJobs_ColumnReordered;

            Resize += MainWindowIndustryJobsList_Resize;

            EveClient.TimerTick += EveClient_TimerTick;
            EveClient.CharacterIndustryJobsChanged += EveClient_CharacterIndustryJobsChanged;
            EveClient.CharacterIndustryJobsCompleted += EveClient_IndustryJobsCompleted;
            Disposed += OnDisposed;
        }
                
        /// <summary>
        /// Gets the character associated with this monitor.
        /// </summary>
        public Character Character
        {
            get { return m_character; }
            set { m_character = value; }
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
                m_grouping = (IndustryJobGrouping)value;
                if (m_init)
                    UpdateColumns();
            }
        }

        /// <summary>
        /// Gets or sets which "Issued for" jobs to display.
        /// </summary>
        public IssuedFor ShowIssuedFor
        {
            get { return m_showIssuedFor; }
            set
            {
                m_showIssuedFor = value;
                if (m_init)
                    UpdateColumns();
            }
        }

        /// <summary>
        /// Gets true when character has active jobs issued for corporation.
        /// </summary>
        public bool HasActiveCorporationIssuedJobs
        {
            get
            {
                return m_list.Any(x =>
                    x.State == JobState.Active && x.IssuedFor == IssuedFor.Corporation);
            }
        }

        /// <summary>
        /// Gets or sets the enumeration of jobs to display.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public IEnumerable<IndustryJob> Jobs
        {
            get
            {
                foreach (var job in m_list)
                {
                    yield return job;
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
        public IEnumerable<IndustryJobColumnSettings> Columns
        {
            get 
            { 
                // Add the visible columns; matching the display order
                var newColumns = new List<IndustryJobColumnSettings>();
                foreach (var header in lvJobs.Columns.Cast<ColumnHeader>().OrderBy(x => x.DisplayIndex))
                {
                    var columnSetting = m_columns.First(x => x.Column == (IndustryJobColumn)header.Tag);
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
                if ( value != null)
                    m_columns.AddRange(value.Select(x => x.Clone()));
                
                // Whenever the columns changes, we need to
                // reset the dipslay index of the TTC column
                m_displayIndexTTC = -1;

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
            EveClient.CharacterIndustryJobsChanged -= EveClient_CharacterIndustryJobsChanged;
            EveClient.CharacterIndustryJobsCompleted -= EveClient_IndustryJobsCompleted;
            Disposed -= OnDisposed;
        }

        /// <summary>
        /// When the control becomes visible again, we update the content.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            if (DesignMode || this.IsDesignModeHosted() || m_character == null)
                return;

            base.OnVisibleChanged(e);

            if (!Visible)
                return;

            // Prevents the properties to call UpdateColumns() till we set all properties
            m_init = false;

            var ccpCharacter = m_character as CCPCharacter;
            Jobs = (ccpCharacter == null ? null : ccpCharacter.IndustryJobs);
            Columns = Settings.UI.MainWindow.IndustryJobs.Columns;
            Grouping = (m_character == null ? IndustryJobGrouping.State : m_character.UISettings.JobsGroupBy);

            UpdateExpPanelContent();
            UpdateColumns();

            m_init = true;

            UpdateContent();
        }

        # endregion


        #region Updates Main Industry Window On Global Events

        /// <summary>
        /// <summary>
        /// Updates the columns.
        /// </summary>
        public void UpdateColumns()
        {
            lvJobs.BeginUpdate();
            m_isUpdatingColumns = true;

            try
            {
                lvJobs.Columns.Clear();

                foreach (var column in m_columns.Where(x => x.Visible))
                {
                    var header = lvJobs.Columns.Add(column.Column.GetHeader(), column.Column.GetHeader(), column.Width);
                    header.Tag = (object)column.Column;
                }

                // We update the content
                UpdateContent();

                // Force the auto-resize of the columns with -1 width
                var resizeStyle = (lvJobs.Items.Count == 0 ?
                    ColumnHeaderAutoResizeStyle.HeaderSize :
                    ColumnHeaderAutoResizeStyle.ColumnContent);

                int index = 0;
                foreach (var column in m_columns.Where(x => x.Visible))
                {
                    if (column.Width == -1)
                        lvJobs.AutoResizeColumn(index, resizeStyle);

                    index++;
                }
            }
            finally
            {
                lvJobs.EndUpdate();
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
            int selectedItem = (lvJobs.SelectedItems.Count > 0 ?
                                lvJobs.SelectedItems[0].Tag.GetHashCode() : 0);

            m_hideInactive = Settings.UI.MainWindow.IndustryJobs.HideInactiveJobs;

            lvJobs.BeginUpdate();
            try
            {
                var text = m_textFilter.ToLowerInvariant();
                var jobs = m_list.Where(x => !x.Ignored && IsTextMatching(x, text));

                if (m_character != null && m_hideInactive)
                    jobs = jobs.Where(x => x.IsActive);

                if (m_showIssuedFor != IssuedFor.All)
                    jobs = jobs.Where(x => x.IssuedFor == m_showIssuedFor);

                UpdateSort();

                switch (m_grouping)
                {
                    case IndustryJobGrouping.State:
                        var groups0 = jobs.GroupBy(x => x.State).OrderBy(x => (int)x.Key);
                        UpdateContent(groups0);
                        break;
                    case IndustryJobGrouping.StateDesc:
                        var groups1 = jobs.GroupBy(x => x.State).OrderByDescending(x => (int)x.Key);
                        UpdateContent(groups1);
                        break;
                    case IndustryJobGrouping.EndDate:
                        var groups2 = jobs.GroupBy(x => x.EndProductionTime.Date).OrderBy(x => x.Key);
                        UpdateContent(groups2);
                        break;
                    case IndustryJobGrouping.EndDateDesc:
                        var groups3 = jobs.GroupBy(x => x.EndProductionTime.Date).OrderByDescending(x => x.Key);
                        UpdateContent(groups3);
                        break;
                    case IndustryJobGrouping.InstalledItemType:
                        var groups4 = jobs.GroupBy(x => x.InstalledItem.MarketGroup.FullCategoryName).OrderBy(x => x.Key);
                        UpdateContent(groups4);
                        break;
                    case IndustryJobGrouping.InstalledItemTypeDesc:
                        var groups5 = jobs.GroupBy(x => x.InstalledItem.MarketGroup.FullCategoryName).OrderByDescending(x => x.Key);
                        UpdateContent(groups5);
                        break;
                    case IndustryJobGrouping.OutputItemType:
                        var groups6 = jobs.GroupBy(x => x.OutputItem.MarketGroup.FullCategoryName).OrderBy(x => x.Key);
                        UpdateContent(groups6);
                        break;
                    case IndustryJobGrouping.OutputItemTypeDesc:
                        var groups7 = jobs.GroupBy(x => x.OutputItem.MarketGroup.FullCategoryName).OrderByDescending(x => x.Key);
                        UpdateContent(groups7);
                        break;
                    case IndustryJobGrouping.Activity:
                        var groups8 = jobs.GroupBy(x => x.Activity.GetDescription()).OrderBy(x => x.Key);
                        UpdateContent(groups8);
                        break;
                    case IndustryJobGrouping.ActivityDesc:
                        var groups9 = jobs.GroupBy(x => x.Activity.GetDescription()).OrderByDescending(x => x.Key);
                        UpdateContent(groups9);
                        break;
                    case IndustryJobGrouping.Location:
                        var groups10 = jobs.GroupBy(x => x.Installation).OrderBy(x => x.Key);
                        UpdateContent(groups10);
                        break;
                    case IndustryJobGrouping.LocationDesc:
                        var groups11 = jobs.GroupBy(x => x.Installation).OrderByDescending(x => x.Key);
                        UpdateContent(groups11);
                        break;
                }

                // Restore the selected item (if any)
                if (selectedItem > 0)
                {
                    foreach (ListViewItem lvItem in lvJobs.Items)
                    {
                        if (lvItem.Tag.GetHashCode() == selectedItem)
                            lvItem.Selected = true;
                    }
                }

                // Display or hide the "no jobs" label
                if (m_init)
                {
                    noJobsLabel.Visible = jobs.IsEmpty();
                    lvJobs.Visible = !jobs.IsEmpty();
                    industryExpPanelControl.Visible = true;
                    industryExpPanelControl.Header.Visible = true;
                }
            }
            finally
            {
                lvJobs.EndUpdate();
            }
        }

        /// <summary>
        /// Updates the content of the listview.
        /// </summary>
        /// <typeparam name="TKey"></typeparam>
        /// <param name="groups"></param>
        private void UpdateContent<TKey>(IEnumerable<IGrouping<TKey, IndustryJob>> groups)
        {
            lvJobs.Items.Clear();
            lvJobs.Groups.Clear();

            // Add the groups
            foreach (var group in groups)
            {
                string groupText = String.Empty;
                if (group.Key is JobState)
                {
                    groupText = ((JobState)(Object)group.Key).GetHeader();
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
                lvJobs.Groups.Add(listGroup);

                // Add the items in every group
                foreach (var job in group)
                {
                    if (job.InstalledItem == null)
                        continue;

                    var item = new ListViewItem(job.InstalledItem.Name, listGroup);
                    item.UseItemStyleForSubItems = false;
                    item.Tag = job;

                    // Display text as dimmed if the job is no longer available
                    if (!job.IsActive)
                        item.ForeColor = SystemColors.GrayText;

                    // Add enough subitems to match the number of columns
                    while (item.SubItems.Count < lvJobs.Columns.Count + 1)
                    {
                        item.SubItems.Add(String.Empty);
                    }

                    // Creates the subitems
                    for (int i = 0; i < lvJobs.Columns.Count; i++)
                    {
                        var header = lvJobs.Columns[i];
                        var column = (IndustryJobColumn)header.Tag;
                        SetColumn(job, item.SubItems[i], column);
                    }

                    lvJobs.Items.Add(item);
                }
            }

        }

        /// <summary>
        /// Updates the item sorter.
        /// </summary>
        private void UpdateSort()
        {
            lvJobs.ListViewItemSorter = new ListViewItemComparerByTag<IndustryJob>(
                new IndustryJobComparer(m_sortCriteria, m_sortAscending));

            UpdateSortVisualFeedback();
        }

        /// <summary>
        /// Updates the sort feedback (the arrow on the header).
        /// </summary>
        private void UpdateSortVisualFeedback()
        {
            for (int i = 0; i < lvJobs.Columns.Count; i++)
            {
                var column = (IndustryJobColumn)lvJobs.Columns[i].Tag;
                if (m_sortCriteria == column)
                {
                    lvJobs.Columns[i].ImageIndex = (m_sortAscending ? 0 : 1);
                }
                else
                {
                    lvJobs.Columns[i].ImageIndex = 2;
                }
            }
        }

        /// <summary>
        /// Updates the listview sub-item.
        /// </summary>
        /// <param name="order"></param>
        /// <param name="item"></param>
        /// <param name="column"></param>
        private void SetColumn(IndustryJob job, ListViewItem.ListViewSubItem item, IndustryJobColumn column)
        {
            switch (column)
            {
                case IndustryJobColumn.State:
                    item.Text = (job.State == JobState.Active ?
                        job.ActiveJobState.GetDescription() : job.State.ToString());
                    item.ForeColor = GetStateColor(job);
                    break;

                case IndustryJobColumn.TTC:
                    item.Text = job.TTC;
                    if (job.State == JobState.Paused)
                        item.ForeColor = Color.Red;
                    break;

                case IndustryJobColumn.InstalledItem:
                    item.Text = job.InstalledItem.ToString();
                    break;

                case IndustryJobColumn.InstalledItemType:
                    item.Text = job.InstalledItem.MarketGroup.Name;
                    break;

                case IndustryJobColumn.OutputItem:
                    item.Text = String.Format("{0} Unit{1} of {2}", GetUnitCount(job), (GetUnitCount(job) > 1 ? "s" : String.Empty), job.OutputItem.ToString());
                    break;

                case IndustryJobColumn.OutputItemType:
                    item.Text = job.OutputItem.MarketGroup.Name;
                    break;

                case IndustryJobColumn.Activity:
                    item.Text = job.Activity.GetDescription();
                    break;

                case IndustryJobColumn.InstallTime:
                    item.Text = job.InstalledTime.ToLocalTime().ToString();
                    break;

                case IndustryJobColumn.EndTime:
                    item.Text = job.EndProductionTime.ToLocalTime().ToString();
                    break;

                case IndustryJobColumn.OriginalOrCopy:
                    item.Text = job.BlueprintType.ToString();
                    break;

                case IndustryJobColumn.InstalledME:
                    item.Text = (job.Activity == BlueprintActivity.ResearchingMaterialProductivity ?
                        job.InstalledME.ToString() : String.Empty);
                    break;

                case IndustryJobColumn.EndME:
                    item.Text = (job.Activity == BlueprintActivity.ResearchingMaterialProductivity ?
                        (job.InstalledME + job.Runs).ToString() : String.Empty);
                    break;

                case IndustryJobColumn.InstalledPE:
                    item.Text = (job.Activity == BlueprintActivity.ResearchingTimeProductivity ?
                        job.InstalledPE.ToString() : String.Empty);
                    break;

                case IndustryJobColumn.EndPE:
                    item.Text = (job.Activity == BlueprintActivity.ResearchingTimeProductivity ?
                        (job.InstalledPE + job.Runs).ToString() : String.Empty);
                    break;

                case IndustryJobColumn.Location:
                    item.Text = job.FullLocation;
                    break;

                case IndustryJobColumn.Region:
                    item.Text = job.SolarSystem.Constellation.Region.Name;
                    break;

                case IndustryJobColumn.SolarSystem:
                    item.Text = job.SolarSystem.Name;
                    break;

                case IndustryJobColumn.Installation:
                    item.Text = job.Installation;
                    break;

                case IndustryJobColumn.IssuedFor:
                    item.Text = job.IssuedFor.ToString();
                    break;

                case IndustryJobColumn.LastStateChange:
                    item.Text = job.LastStateChange.ToLocalTime().ToString();
                    break;

                default:
                    //return;
                    throw new NotImplementedException();
            }
        }

        #endregion


        # region Helper Methods

        /// <summary>
        /// Gets the unit count.
        /// </summary>
        /// <param name="job">The job.</param>
        /// <returns></returns>
        private int GetUnitCount(IndustryJob job)
        {
            if (job.Activity != BlueprintActivity.Manufacturing)
                return 1;

            if (job.InstalledItem.ID == DBConstants.WarpDisruptProbeBlueprintID)
                return job.Runs * 2;

            switch (job.InstalledItem.MarketGroup.ID)
            {
                case DBConstants.BoostersChargesBlueprintsGroupID:
                    return job.Runs * 10;
                case DBConstants.BombsBlueprintsGroupID:
                    return job.Runs * 20;
            }

            if (job.InstalledItem.MarketGroup.ParentGroup != null)
            {
                switch (job.InstalledItem.MarketGroup.ParentGroup.ID)
                {
                    case DBConstants.ProjectileAmmunitionBlueprintsGroupID:
                    case DBConstants.HybridAmmunitionBlueprintsGroupID:
                    case DBConstants.MissilesAmmunitionBlueprintsGroupID:
                        return job.Runs * 100;
                }
            }

            return job.Runs;
        }

        /// <summary>
        /// Gets the color of the state.
        /// </summary>
        /// <param name="job">The job.</param>
        /// <returns></returns>
        private Color GetStateColor(IndustryJob job)
        {
            switch (job.State)
            {
                case JobState.Canceled:
                    return Color.DarkGray;
                case JobState.Failed:
                    return Color.DarkRed;
                case JobState.Paused:
                    return Color.RoyalBlue;
                case JobState.Active:
                    return GetActiveJobStateColor(job.ActiveJobState);
                default:
                    return SystemColors.GrayText;
            }
        }

        /// <summary>
        /// Gets the color of the active job state.
        /// </summary>
        /// <param name="activeJobState">State of the active job.</param>
        /// <returns></returns>
        private Color GetActiveJobStateColor(ActiveJobState activeJobState)
        {
            switch (activeJobState)
            {
                case ActiveJobState.Pending:
                    return Color.Red;
                case ActiveJobState.InProgress:
                    return Color.Orange;
                case ActiveJobState.Ready:
                    return Color.Green;
                default:
                    return SystemColors.GrayText;
            }
        }

        /// <summary>
        /// The description of the range.
        /// </summary>
        public string GetRange(int range)
        {
            switch (range)
            {
                case 0:
                    return "stations";
                case 1:
                    return "solar systems";
                case 2:
                    return "5 jumps";
                case 3:
                    return "10 jumps";
                case 4:
                    return "20 jumps";
                case 5:
                    return "regions";
                default:
                    return String.Empty;
            }
        }

        /// <summary>
        /// Checks the given text matches the item.
        /// </summary>
        /// <param name="x">The x.</param>
        /// <param name="text">The text.</param>
        /// <returns>
        /// 	<c>true</c> if [is text matching] [the specified x]; otherwise, <c>false</c>.
        /// </returns>
        private bool IsTextMatching(IndustryJob x, string text)
        {
            if (String.IsNullOrEmpty(text)
                || x.InstalledItem.Name.ToLowerInvariant().Contains(text)
                || x.OutputItem.Name.ToLowerInvariant().Contains(text)
                || x.Installation.ToLowerInvariant().Contains(text)
                || x.SolarSystem.Name.ToLowerInvariant().Contains(text)
                || x.SolarSystem.Constellation.Name.ToLowerInvariant().Contains(text)
                || x.SolarSystem.Constellation.Region.Name.ToLowerInvariant().Contains(text))
                return true;
            
            return false;
        }

        #endregion


        #region Event Handlers

        /// <summary>
        /// On resize, updates the controls visibility.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void MainWindowIndustryJobsList_Resize(object sender, EventArgs e)
        {
            if (!m_init)
                return;

            UpdateContent();
        }

        /// <summary>
        /// On column reorder we update the settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void lvJobs_ColumnReordered(object sender, ColumnReorderedEventArgs e)
        {
            m_columnsChanged = true;
        }

        /// <summary>
        /// When the user manually resizes a column, we make sure to update the column preferences.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void lvJobs_ColumnWidthChanged(object sender, ColumnWidthChangedEventArgs e)
        {
            if (m_isUpdatingColumns || m_columns.Count <= e.ColumnIndex)
                return;

            // Don't update the columns if the TTC column width changes
            if (e.ColumnIndex == m_displayIndexTTC)
                return;

            m_columns[e.ColumnIndex].Width = lvJobs.Columns[e.ColumnIndex].Width;
            m_columnsChanged = true;
        }
        /// <summary>
        /// When the user clicks a column header, we update the sorting.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lvJobs_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            var column = (IndustryJobColumn)lvJobs.Columns[e.Column].Tag;
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

        /// <summary>
        /// Handles key press
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lvJobs_KeyDown(object sender, KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.A:
                    if (e.Control)
                        lvJobs.SelectAll();
                    break;
                case Keys.Delete:
                    if (lvJobs.SelectedItems.Count == 0)
                        return;

                    // Mark as ignored
                    foreach (ListViewItem item in lvJobs.SelectedItems)
                    {
                        var job = (IndustryJob)item.Tag;
                        job.Ignored = true;
                    }

                    // Updates
                    UpdateContent();
                    break;
                default:
                    break;
            }
        }

        /// <summary>
        /// Occurs on a mouse click in the expandable panel.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void jobExpPanelControl_MouseClick(object sender, MouseEventArgs e)
        {
            // We do this to avoid drawing border traces on the background
            // when there are no industry jobs present
            if (industryExpPanelControl.IsExpanded && noJobsLabel.Visible)
                noJobsLabel.BringToFront();

            // Reduce the label's mouse coordinate to panel's
            int positionX = (sender is Label ? ((Label)sender).Location.X + e.X : e.X);
            int positionY = (sender is Label ? ((Label)sender).Location.Y + e.Y : e.Y);
            var arg = new MouseEventArgs(e.Button, 1, positionX, positionY, 3);

            industryExpPanelControl.expandablePanelControl_MouseClick(sender, arg);
            noJobsLabel.SendToBack();
        }
        # endregion


        #region Global events
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_IndustryJobsCompleted(object sender, IndustryJobsEventArgs e)
        {
            UpdateContent();
        }

        /// <summary>
        /// When the industry jobs are changed,
        /// update the list and the expandable panel info.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterIndustryJobsChanged(object sender, CharacterChangedEventArgs e)
        {
            var ccpCharacter = m_character as CCPCharacter;
            if (e.Character != ccpCharacter)
                return;
            
            Jobs = ccpCharacter.IndustryJobs;
            UpdateColumns();
            UpdateExpPanelContent();
        }

        /// <summary>
        /// On timer tick, we update the time to completion, the active job state
        /// and the columns settings if any changes have beem made to them.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_TimerTick(object sender, EventArgs e)
        {
            var colIndexTTC = m_columns.IndexOf(m_columns.FirstOrDefault(x => x.Column == IndustryJobColumn.TTC));

            for (int i = 0; i < lvJobs.Items.Count; i++)
            {
                IndustryJob job = (IndustryJob)lvJobs.Items[i].Tag;
                if (!job.IsActive || job.ActiveJobState == ActiveJobState.Ready)
                    continue;

                // Update the time to completion
                if (colIndexTTC != -1 && m_columns[colIndexTTC].Visible)
                {
                    if (m_displayIndexTTC == -1)
                        m_displayIndexTTC = lvJobs.Columns[IndustryJobColumn.TTC.GetHeader()].DisplayIndex;

                    lvJobs.Items[i].SubItems[m_displayIndexTTC].Text = job.TTC;

                    // Using AutoResizeColumn when TTC is the first column
                    // results to a nasty visual bug due to ListViewItem.ImageIndex placeholder
                    if (m_displayIndexTTC == 0)
                    {
                        int columnWidth = 0;
                        foreach (ListViewItem item in lvJobs.Items)
                        {
                            int textWidth = TextRenderer.MeasureText(item.SubItems[m_displayIndexTTC].Text, Font).Width;
                            if (textWidth > columnWidth)
                                columnWidth = textWidth;
                        }
                        lvJobs.Columns[m_displayIndexTTC].Width = columnWidth + 22;
                    }
                    else
                    {
                        lvJobs.AutoResizeColumn(m_displayIndexTTC, ColumnHeaderAutoResizeStyle.ColumnContent);
                    }
                }

                // Job was pending and its time to start
                if (job.ActiveJobState == ActiveJobState.Pending && job.BeginProductionTime < DateTime.UtcNow)
                {
                    job.ActiveJobState = ActiveJobState.InProgress;
                    UpdateContent();
                }

                // Job is ready
                if (job.TTC == String.Empty)
                    job.ActiveJobState = ActiveJobState.Ready;
            }

            if (m_columnsChanged)
            {
                Settings.UI.MainWindow.IndustryJobs.Columns = Columns.Select(x => x.Clone()).ToArray();

                // Recreate the columns
                Columns = Settings.UI.MainWindow.IndustryJobs.Columns;
                m_columnsChanged = false;
            }
        }

        # endregion


        #region Updates Expandable Panel On Global Events

        /// <summary>
        /// Updates the content of the expandable panel.
        /// </summary>
        private void UpdateExpPanelContent()
        {
            if (m_character == null)
            {
                industryExpPanelControl.Visible = false;
                return;
            }

            if (m_init)
                industryExpPanelControl.Visible = true;

            // Calculate the related info for the panel
            CalculatePanelInfo();

            // Update the info in the panel
            UpdatePanelInfo();

            // Header text
            int baseJobs = 1;
            int maxManufacturingJobs = baseJobs + m_skillBasedManufacturingJobs;
            int maxResearchingJobs = baseJobs + m_skillBasedResearchingJobs;
            int activeManufacturingJobs = m_activeManufJobsIssuedForCharacterCount + m_activeManufJobsIssuedForCorporationCount;
            int activeResearchingJobs = m_activeResearchJobsIssuedForCharacterCount + m_activeResearchJobsIssuedForCorporationCount;
            int remainingManufacturingJobs = maxManufacturingJobs - activeManufacturingJobs;
            int remainingResearchingJobs = maxResearchingJobs - activeResearchingJobs;

            string manufJobsRemainingText = String.Format(CultureConstants.DefaultCulture,
                "Manufacturing Jobs Remaining: {0} out of {1} max", remainingManufacturingJobs, maxManufacturingJobs);
            string researchJobsRemainingText = String.Format(CultureConstants.DefaultCulture,
                "Researching Jobs Remaining: {0} out of {1} max", remainingResearchingJobs, maxResearchingJobs);
            industryExpPanelControl.HeaderText = String.Format(CultureConstants.DefaultCulture, "{0}{2,5}{1}", manufJobsRemainingText, researchJobsRemainingText, String.Empty);

            // Update label position
            UpdatePanelControlPosition();

            // Force to redraw
            industryExpPanelControl.Invalidate();
            industryExpPanelControl.Update();
        }

        /// <summary>
        /// Updates the labels text in the panel.
        /// </summary>
        private void UpdatePanelInfo()
        {
            // Update text
            int activeManufacturingJobsCount = m_activeManufJobsIssuedForCharacterCount + m_activeManufJobsIssuedForCorporationCount;
            int activeResearchingJobsCount = m_activeResearchJobsIssuedForCharacterCount + m_activeResearchJobsIssuedForCorporationCount;

            string remoteManufacturingRange = GetRange(m_remoteManufacturingRange);
            string remoteResearchingRange = GetRange(m_remoteResearchingRange);

            // Basic label text
            lblActiveManufacturingJobs.Text = String.Format(
                CultureConstants.DefaultCulture, "Active Manufacturing Jobs: {0}", activeManufacturingJobsCount);
            lblActiveResearchingJobs.Text = String.Format(
                CultureConstants.DefaultCulture, "Active Researching Jobs: {0}", activeResearchingJobsCount);
            lblRemoteManufacturingRange.Text = String.Format(
                CultureConstants.DefaultCulture, "Remote Manufacturing Range: limited to {0}", remoteManufacturingRange);
            lblRemoteResearchingRange.Text = String.Format(
                CultureConstants.DefaultCulture, "Remote Researching Range: limited to {0}", remoteResearchingRange);

            // Supplemental label text
            lblActiveCharManufacturingJobs.Text = String.Format(
                CultureConstants.DefaultCulture, "Character Issued: {0}", m_activeManufJobsIssuedForCharacterCount);
            lblActiveCorpManufacturingJobs.Text = String.Format(
                CultureConstants.DefaultCulture, "Corporation Issued: {0}", m_activeManufJobsIssuedForCorporationCount);
            lblActiveCharResearchingJobs.Text = String.Format(
                CultureConstants.DefaultCulture, "Character Issued: {0}", m_activeResearchJobsIssuedForCharacterCount);
            lblActiveCorpResearchingJobs.Text = String.Format(
                CultureConstants.DefaultCulture, "Corporation Issued: {0}", m_activeResearchJobsIssuedForCorporationCount);
        }

        /// <summary>
        /// Updates expandable panel controls positions.
        /// </summary>
        private void UpdatePanelControlPosition()
        {
            var pad = 5;
            var height = (industryExpPanelControl.ExpandDirection == Direction.Up ? pad : industryExpPanelControl.HeaderHeight);

            lblActiveManufacturingJobs.Location = new Point(5, height);
            lblRemoteManufacturingRange.Location = new Point(lblRemoteManufacturingRange.Location.X, height);
            height += lblActiveManufacturingJobs.Height;
            lblRemoteResearchingRange.Location = new Point(lblRemoteResearchingRange.Location.X, height);
            if (HasActiveCorporationIssuedJobs)
            {
                lblActiveCharManufacturingJobs.Location = new Point(15, height);
                lblActiveCharManufacturingJobs.Visible = true;
                height += lblActiveCharManufacturingJobs.Height;

                lblActiveCorpManufacturingJobs.Location = new Point(15, height);
                lblActiveCorpManufacturingJobs.Visible = true;
                height += lblActiveCorpManufacturingJobs.Height;
                height += pad;
            }
            else
            {
                lblActiveCharManufacturingJobs.Visible = false;
                lblActiveCorpManufacturingJobs.Visible = false;
            }

            lblActiveResearchingJobs.Location = new Point(5, height);
            height += lblActiveResearchingJobs.Height;

            if (HasActiveCorporationIssuedJobs)
            {
                lblActiveCharResearchingJobs.Location = new Point(15, height);
                lblActiveCharResearchingJobs.Visible = true;
                height += lblActiveCharResearchingJobs.Height;

                lblActiveCorpResearchingJobs.Location = new Point(15, height);
                lblActiveCorpResearchingJobs.Visible = true;
                height += lblActiveCorpResearchingJobs.Height;
            }
            else
            {
                lblActiveCharResearchingJobs.Visible = false;
                lblActiveCorpResearchingJobs.Visible = false;
            }

            height += pad;

            // Update panel's expanded height
            industryExpPanelControl.ExpandedHeight = height + (industryExpPanelControl.ExpandDirection == Direction.Up ? industryExpPanelControl.HeaderHeight : pad);
        }

        /// <summary>
        /// Calculates the industry jobs related info for the panel.
        /// </summary>
        private void CalculatePanelInfo()
        {
            var activeManufJobsIssuedForCharacter = m_list.Where(x => (x.State == JobState.Active)
                && x.Activity == BlueprintActivity.Manufacturing && x.IssuedFor == IssuedFor.Character);
            var activeManufJobsIssuedForCorporation = m_list.Where(x => (x.State == JobState.Active)
                && x.Activity == BlueprintActivity.Manufacturing && x.IssuedFor == IssuedFor.Corporation);
            var activeResearchJobsIssuedForCharacter = m_list.Where(x => (x.State == JobState.Active)
                && x.Activity != BlueprintActivity.Manufacturing && x.IssuedFor == IssuedFor.Character);
            var activeResearchJobsIssuedForCorporation = m_list.Where(x => (x.State == JobState.Active)
                && x.Activity != BlueprintActivity.Manufacturing && x.IssuedFor == IssuedFor.Corporation);
            
            // Calculate character's max manufacturing jobs
            m_skillBasedManufacturingJobs = m_character.Skills.FirstOrDefault(x => x.ID == DBConstants.MassProductionSkillID).LastConfirmedLvl
                + m_character.Skills.FirstOrDefault(x => x.ID == DBConstants.AdvancedMassProductionSkillID).LastConfirmedLvl;

            // Calculate character's max researching jobs
            m_skillBasedResearchingJobs = m_character.Skills.FirstOrDefault(x => x.ID == DBConstants.LaboratoryOperationSkillID).LastConfirmedLvl
                + m_character.Skills.FirstOrDefault(x => x.ID == DBConstants.AdvancedLaboratoryOperationSkillID).LastConfirmedLvl;

            // Calculate character's remote manufacturing range
            m_remoteManufacturingRange = m_character.Skills.FirstOrDefault(x => x.ID == DBConstants.SupplyChainManagementSkillID).LastConfirmedLvl;

            // Calculate character's remote researching range
            m_remoteResearchingRange = m_character.Skills.FirstOrDefault(x => x.ID == DBConstants.ScientificNetworkingSkillID).LastConfirmedLvl;

            // Calculate active manufacturing & researching jobs count (character & corporation issued separately)
            m_activeManufJobsIssuedForCharacterCount = activeManufJobsIssuedForCharacter.Count();
            m_activeManufJobsIssuedForCorporationCount = activeManufJobsIssuedForCorporation.Count();
            m_activeResearchJobsIssuedForCharacterCount = activeResearchJobsIssuedForCharacter.Count();
            m_activeResearchJobsIssuedForCorporationCount = activeResearchJobsIssuedForCorporation.Count();

        }

        # endregion


        #region Initialize Expandable Panel Controls

        // Basic labels constructor
        private Label lblActiveManufacturingJobs = new Label();
        private Label lblActiveResearchingJobs = new Label();
        private Label lblRemoteManufacturingRange = new Label();
        private Label lblRemoteResearchingRange = new Label();

        // Supplemental labels constructor
        private Label lblActiveCharManufacturingJobs = new Label();
        private Label lblActiveCorpManufacturingJobs = new Label();
        private Label lblActiveCharResearchingJobs = new Label();
        private Label lblActiveCorpResearchingJobs = new Label();

        private void InitializeExpandablePanelControls()
        {
            // Add basic labels to panel
            industryExpPanelControl.Controls.AddRange(new Label[]
            { 
                lblActiveManufacturingJobs,
                lblActiveResearchingJobs,
                lblRemoteManufacturingRange,
                lblRemoteResearchingRange,
            });

            // Add supplemental labels to panel
            industryExpPanelControl.Controls.AddRange(new Label[]
            { 
                lblActiveCharManufacturingJobs,
                lblActiveCorpManufacturingJobs,
                lblActiveCharResearchingJobs,
                lblActiveCorpResearchingJobs,
            });

            // Apply properties
            foreach (Label label in industryExpPanelControl.Controls.OfType<Label>())
            {
                label.ForeColor = SystemColors.ControlText;
                label.BackColor = Color.Transparent;
                label.AutoSize = true;
            }

            // Special properties
            lblRemoteManufacturingRange.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            lblRemoteResearchingRange.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            lblRemoteManufacturingRange.Location = new Point(170, 0);
            lblRemoteResearchingRange.Location = new Point(170, 0);

            // Subscribe events
            industryExpPanelControl.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
            industryExpPanelControl.Header.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
            lblActiveManufacturingJobs.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
            lblActiveResearchingJobs.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
            lblRemoteManufacturingRange.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
            lblRemoteResearchingRange.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
            
            lblActiveCharManufacturingJobs.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
            lblActiveCorpManufacturingJobs.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
            lblActiveCharResearchingJobs.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
            lblActiveCorpResearchingJobs.MouseClick += new MouseEventHandler(jobExpPanelControl_MouseClick);
        }

        #endregion
    }
}
