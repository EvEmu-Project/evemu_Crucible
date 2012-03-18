using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.SettingsObjects;
using EVEMon.Controls;

namespace EVEMon
{
    public partial class MainWindowResearchPointsList : UserControl
    {
        #region Fields

        private List<ResearchColumnSettings> m_columns = new List<ResearchColumnSettings>();
        private readonly List<ResearchPoint> m_list = new List<ResearchPoint>();

        private ResearchColumn m_sortCriteria;

        private string m_textFilter = String.Empty;
        private bool m_sortAscending = true;
        private bool m_columnsChanged;
        private bool m_isUpdatingColumns;
        private bool m_init;

        #endregion


        #region Constructor

        /// <summary>
        /// Constructor.
        /// </summary>
        public MainWindowResearchPointsList()
        {
            InitializeComponent();

            lvResearchPoints.Visible = false;
            lvResearchPoints.AllowColumnReorder = true;
            lvResearchPoints.Columns.Clear();

            noResearchLabel.Font = FontFactory.GetFont("Tahoma", 11.25F, FontStyle.Bold);

            DoubleBuffered = true;
            ListViewHelper.EnableDoubleBuffer(lvResearchPoints);

            lvResearchPoints.ColumnClick += lvResearchPoints_ColumnClick;
            lvResearchPoints.ColumnWidthChanged += lvResearchPoints_ColumnWidthChanged;
            lvResearchPoints.ColumnReordered += lvResearchPoints_ColumnReordered;

            Resize += MainWindowResearchPointsList_Resize;

            EveClient.TimerTick += EveClient_TimerTick;
            EveClient.CharacterResearchPointsChanged += EveClient_CharacterResearchPointsChanged;
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
        /// Gets or sets the enumeration of research points to display.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public IEnumerable<ResearchPoint> ResearchPoints
        {
            get
            {
                foreach (var researchPoint in m_list)
                {
                    yield return researchPoint;
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
        public IEnumerable<ResearchColumnSettings> Columns
        {
            get
            {
                // Add the visible columns; matching the display order
                var newColumns = new List<ResearchColumnSettings>();
                foreach (var header in lvResearchPoints.Columns.Cast<ColumnHeader>().OrderBy(x => x.DisplayIndex))
                {
                    var columnSetting = m_columns.First(x => x.Column == (ResearchColumn)header.Tag);
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
            EveClient.CharacterResearchPointsChanged -= EveClient_CharacterResearchPointsChanged;
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
            ResearchPoints = (ccpCharacter == null ? null : ccpCharacter.ResearchPoints);
            Columns = Settings.UI.MainWindow.Research.Columns;

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
            lvResearchPoints.BeginUpdate();
            m_isUpdatingColumns = true;

            try
            {
                lvResearchPoints.Columns.Clear();

                foreach (var column in m_columns.Where(x => x.Visible))
                {
                    var header = lvResearchPoints.Columns.Add(column.Column.GetHeader(), column.Column.GetHeader(), column.Width);
                    header.Tag = (object)column.Column;

                    switch (column.Column)
                    {
                        case ResearchColumn.CurrentRP:
                        case ResearchColumn.PointsPerDay:
                            header.TextAlign = HorizontalAlignment.Right;
                            break;
                        case ResearchColumn.Level:
                        case ResearchColumn.Quality:
                            header.TextAlign = HorizontalAlignment.Center;
                            break;
                    }

                }

                // We update the content
                UpdateContent();

                // Force the auto-resize of the columns with -1 width
                var resizeStyle = (lvResearchPoints.Items.Count == 0 ?
                    ColumnHeaderAutoResizeStyle.HeaderSize :
                    ColumnHeaderAutoResizeStyle.ColumnContent);

                int index = 0;
                foreach (var column in m_columns.Where(x => x.Visible))
                {
                    if (column.Width == -1)
                        lvResearchPoints.AutoResizeColumn(index, resizeStyle);

                    index++;
                }
            }
            finally
            {
                lvResearchPoints.EndUpdate();
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
            int selectedItem = (lvResearchPoints.SelectedItems.Count > 0 ?
                                lvResearchPoints.SelectedItems[0].Tag.GetHashCode() : 0);

            lvResearchPoints.BeginUpdate();
            try
            {
                var text = m_textFilter.ToLowerInvariant();
                var researhPoints = m_list.Where(x => IsTextMatching(x, text));

                UpdateSort();

                lvResearchPoints.Items.Clear();

                // Add the items in every group
                foreach (var researchPoint in researhPoints)
                {
                    if (String.IsNullOrEmpty(researchPoint.AgentName) || String.IsNullOrEmpty(researchPoint.Field))
                        continue;

                    var item = new ListViewItem(researchPoint.AgentName);
                    item.UseItemStyleForSubItems = false;
                    item.Tag = researchPoint;

                    // Add enough subitems to match the number of columns
                    while (item.SubItems.Count < lvResearchPoints.Columns.Count + 1)
                    {
                        item.SubItems.Add(String.Empty);
                    }

                    // Creates the subitems
                    for (int i = 0; i < lvResearchPoints.Columns.Count; i++)
                    {
                        var header = lvResearchPoints.Columns[i];
                        var column = (ResearchColumn)header.Tag;
                        SetColumn(researchPoint, item.SubItems[i], column);
                    }

                    lvResearchPoints.Items.Add(item);
                }

                // Restore the selected item (if any)
                if (selectedItem > 0)
                {
                    foreach (ListViewItem lvItem in lvResearchPoints.Items)
                    {
                        if (lvItem.Tag.GetHashCode() == selectedItem)
                            lvItem.Selected = true;
                    }
                }

                // Display or hide the "no research points" label
                if (m_init)
                {
                    noResearchLabel.Visible = researhPoints.IsEmpty();
                    lvResearchPoints.Visible = !researhPoints.IsEmpty();
                }
            }
            finally
            {
                lvResearchPoints.EndUpdate();
            }
        }

        /// <summary>
        /// Updates the item sorter.
        /// </summary>
        private void UpdateSort()
        {
            lvResearchPoints.ListViewItemSorter = new ListViewItemComparerByTag<ResearchPoint>(
                new ResearchPointComparer(m_sortCriteria, m_sortAscending));

            UpdateSortVisualFeedback();
        }

        /// <summary>
        /// Updates the sort feedback (the arrow on the header).
        /// </summary>
        private void UpdateSortVisualFeedback()
        {
            for (int i = 0; i < lvResearchPoints.Columns.Count; i++)
            {
                var column = (ResearchColumn)lvResearchPoints.Columns[i].Tag;
                if (m_sortCriteria == column)
                {
                    lvResearchPoints.Columns[i].ImageIndex = (m_sortAscending ? 0 : 1);
                }
                else
                {
                    lvResearchPoints.Columns[i].ImageIndex = 2;
                }
            }
        }

        /// <summary>
        /// Updates the listview sub-item.
        /// </summary>
        /// <param name="order"></param>
        /// <param name="item"></param>
        /// <param name="column"></param>
        private void SetColumn(ResearchPoint researchPoint, ListViewItem.ListViewSubItem item, ResearchColumn column)
        {
            switch (column)
            {
                case ResearchColumn.Agent:
                    item.Text = researchPoint.AgentName;
                    break;

                case ResearchColumn.Level:
                    item.Text = researchPoint.AgentLevel.ToString();
                    break;

                case ResearchColumn.Quality:
                    item.Text = researchPoint.AgentQuality.ToString();
                    break;

                case ResearchColumn.Field:
                    item.Text = researchPoint.Field;
                    break;

                case ResearchColumn.CurrentRP:
                    item.Text = researchPoint.CurrentRP.ToString("N2");
                    break;

                case ResearchColumn.PointsPerDay:
                    item.Text = researchPoint.PointsPerDay.ToString("N2");
                    break;

                case ResearchColumn.StartDate:
                    item.Text = researchPoint.StartDate.ToLocalTime().ToString();
                    break;

                case ResearchColumn.Location:
                    item.Text = researchPoint.Station.FullLocation;
                    break;

                case ResearchColumn.Region:
                    item.Text = researchPoint.Station.SolarSystem.Constellation.Region.Name;
                    break;

                case ResearchColumn.SolarSystem:
                    item.Text = researchPoint.Station.SolarSystem.Name;
                    break;

                case ResearchColumn.Station:
                    item.Text = researchPoint.Station.Name;
                    break;

                default:
                    //return;
                    throw new NotImplementedException();
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
        private bool IsTextMatching(ResearchPoint x, string text)
        {
            if (String.IsNullOrEmpty(text)
                || x.AgentName.ToLowerInvariant().Contains(text)
                || x.Field.ToLowerInvariant().Contains(text)
                || x.Station.Name.ToLowerInvariant().Contains(text)
                || x.Station.SolarSystem.Name.ToLowerInvariant().Contains(text)
                || x.Station.SolarSystem.Constellation.Name.ToLowerInvariant().Contains(text)
                || x.Station.SolarSystem.Constellation.Region.Name.ToLowerInvariant().Contains(text))
                return true;

            return false;
        }

        #endregion


        #region Local Event Handlers

        /// <summary>
        /// On resize, updates the controls visibility.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void MainWindowResearchPointsList_Resize(object sender, EventArgs e)
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
        void lvResearchPoints_ColumnReordered(object sender, ColumnReorderedEventArgs e)
        {
            m_columnsChanged = true;
        }

        /// <summary>
        /// When the user manually resizes a column, we make sure to update the column preferences.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void lvResearchPoints_ColumnWidthChanged(object sender, ColumnWidthChangedEventArgs e)
        {
            if (m_isUpdatingColumns || m_columns.Count <= e.ColumnIndex)
                return;

            m_columns[e.ColumnIndex].Width = lvResearchPoints.Columns[e.ColumnIndex].Width;
            m_columnsChanged = true;
        }

        /// <summary>
        /// When the user clicks a column header, we update the sorting.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lvResearchPoints_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            var column = (ResearchColumn)lvResearchPoints.Columns[e.Column].Tag;
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
                Settings.UI.MainWindow.Research.Columns = Columns.Select(x => x.Clone()).ToArray();

                // Recreate the columns
                Columns = Settings.UI.MainWindow.Research.Columns;
            }

            m_columnsChanged = false;
        }

        /// <summary>
        /// When the research points change update the list.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterResearchPointsChanged(object sender, CharacterChangedEventArgs e)
        {
            var ccpCharacter = Character as CCPCharacter;
            if (e.Character != ccpCharacter)
                return;

            ResearchPoints = ccpCharacter.ResearchPoints;
            UpdateColumns();
        }

        # endregion
    }
}
