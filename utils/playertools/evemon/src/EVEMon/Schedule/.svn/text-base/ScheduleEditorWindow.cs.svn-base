using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows.Forms;
using System.Drawing;
using System.Text;
using System.Linq;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.Scheduling;
using EVEMon.Common.Serialization;

namespace EVEMon.Schedule
{
    public partial class ScheduleEditorWindow : EVEMonForm
    {
        private ToolTip m_tooltip = null;
        private DateTime m_currentDate = DateTime.Now;
        private List<ScheduleEntry> lbEntriesData = null;

        /// <summary>
        /// Constructor
        /// </summary>
        public ScheduleEditorWindow()
        {
            InitializeComponent();
            this.calControl.EntryFont = FontFactory.GetFont("Microsoft Sans Serif", 7F);

            // Setup Balloon Tooltip for later use
            m_tooltip = new ToolTip();
            m_tooltip.IsBalloon = true;
            m_tooltip.UseAnimation = true;
            m_tooltip.UseFading = true;
            m_tooltip.AutoPopDelay = 10000;
            m_tooltip.ReshowDelay = 100;
            m_tooltip.InitialDelay = 500;
            m_tooltip.ToolTipIcon = ToolTipIcon.Info;

            // Load Calendar Colors
            calControl.BlockingColor = (Color)Settings.UI.Scheduler.BlockingColor;
            calControl.RecurringColor = (Color)Settings.UI.Scheduler.RecurringEventGradientStart;
            calControl.RecurringColor2 = (Color)Settings.UI.Scheduler.RecurringEventGradientEnd;
            calControl.SingleColor = (Color)Settings.UI.Scheduler.SimpleEventGradientStart;
            calControl.SingleColor2 = (Color)Settings.UI.Scheduler.SimpleEventGradientEnd;
            calControl.TextColor = (Color)Settings.UI.Scheduler.TextColor;
        }

        /// <summary>
        /// On load, initialize the days and months names, and set the calendar to the current date.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ScheduleEditorWindow_Load(object sender, EventArgs e)
        {
            UpdateEntries();

            // Months names
            nudMonth.Items.Clear();
            string[] monthNames = CultureConstants.DefaultCulture.DateTimeFormat.MonthNames;

            // Days names
            nudMonth.Items.Add(monthNames[0]);
            for (int i = 1; i <= CultureConstants.DefaultCulture.Calendar.GetMonthsInYear(m_currentDate.Year); i++)
            {
                nudMonth.Items.Add(
                    monthNames[((CultureConstants.DefaultCulture.Calendar.GetMonthsInYear(m_currentDate.Year)) - i)]);
            }

            // Set controls to current date
            nudMonth.Items.Add(monthNames[CultureConstants.DefaultCulture.Calendar.GetMonthsInYear(m_currentDate.Year) - 1]);
            nudYear.Value = m_currentDate.Year;
            nudMonth.SelectedIndex = ((nudMonth.Items.Count - 1) - m_currentDate.Month);
            nudDay.Maximum = CultureConstants.DefaultCulture.Calendar.GetDaysInMonth(m_currentDate.Year, m_currentDate.Month) + 1;
            nudDay.Value = m_currentDate.Day;
            calControl.Date = m_currentDate;

            // Subscribe to global events
            EveClient.SchedulerChanged += new EventHandler(EveClient_SchedulerChanged);
        }

        /// <summary>
        /// Unsubscribe events on closing.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            EveClient.SchedulerChanged -= new EventHandler(EveClient_SchedulerChanged);
            base.OnClosing(e);
        }

        #region Content creation & update
        /// <summary>
        /// Anytime the scheduler changed, we update the entries.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EveClient_SchedulerChanged(object sender, EventArgs e)
        {
            UpdateEntries();
        }

        /// <summary>
        /// Update both the calendar control and the left listbox
        /// </summary>
        private void UpdateEntries()
        {
            UpdateListBoxEntries();
            UpdateCalendarEntries();
        }

        /// <summary>
        /// Update the calendar control.
        /// </summary>
        private void UpdateCalendarEntries()
        {
            calControl.Entries.Clear();
            foreach (ScheduleEntry temp in Scheduler.Entries)
            {
                calControl.Entries.Add(temp);
            }
            calControl.Invalidate();
        }

        /// <summary>
        /// Update the left listbox containing the scheduled entries list.
        /// </summary>
        private void UpdateListBoxEntries()
        {
            lbEntriesData = null;
            lbEntriesData = new List<ScheduleEntry>(Scheduler.Entries);
            lbEntriesData.Sort(new ScheduleEntryTitleComparer());

            lbEntries.Items.Clear();
            lbEntriesData.ForEach(x => lbEntries.Items.Add(x));

            lbEntries.SelectedIndex = (lbEntriesData.IsEmpty() ? -1 : 0);
        }

        /// <summary>
        /// Update the entry's description
        /// </summary>
        private void UpdateEntryDescription()
        {
            ScheduleEntry temp = lbEntriesData[lbEntries.SelectedIndex];
            StringBuilder sb = new StringBuilder();

            sb.AppendFormat("Title: {0}", temp.Title).AppendLine();

            // Simple entry ?
            if (temp is SimpleScheduleEntry)
            {
                SimpleScheduleEntry simpleEntry = (SimpleScheduleEntry)temp;

                sb.AppendLine("One Off Entry");
                sb.AppendFormat(CultureConstants.DefaultCulture, " Start: {0}", simpleEntry.StartDate).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " End: {0}", simpleEntry.EndDate).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " Expired: {0}", simpleEntry.Expired).AppendLine();
                sb.AppendLine();
                sb.AppendLine("Options");
                sb.AppendFormat(CultureConstants.DefaultCulture, " Blocking: {0}",
                    (simpleEntry.Options & ScheduleEntryOptions.Blocking) != ScheduleEntryOptions.None).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " Silent: {0}",
                    (simpleEntry.Options & ScheduleEntryOptions.Quiet) != ScheduleEntryOptions.None).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " Uses Eve Time: {0}",
                    (simpleEntry.Options & ScheduleEntryOptions.EVETime) != ScheduleEntryOptions.None).AppendLine();
            }
            // Or recurring entry ?
            else
            {
                RecurringScheduleEntry recurringEntry = (RecurringScheduleEntry)temp;

                sb.AppendLine("Recurring Entry");
                sb.AppendFormat(CultureConstants.DefaultCulture, " Start: {0}",
                    recurringEntry.StartDate.ToShortDateString()).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " End: {0}",
                    recurringEntry.EndDate.ToShortDateString()).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " Frequency: {0}",
                    recurringEntry.Frequency).AppendLine();

                if (recurringEntry.Frequency == RecurringFrequency.Monthly)
                {
                    sb.AppendFormat(CultureConstants.DefaultCulture, "  Day of Month: {0}", recurringEntry.DayOfMonth).AppendLine();
                    sb.AppendFormat(CultureConstants.DefaultCulture, "  On Overflow: {0}", recurringEntry.OverflowResolution).AppendLine();
                }
                else if (recurringEntry.Frequency == RecurringFrequency.Weekly)
                {
                    DateTime nowish = DateTime.Now.Date;
                    DateTime initial = recurringEntry.StartDate.AddDays((recurringEntry.DayOfWeek - recurringEntry.StartDate.DayOfWeek + 7) % 7);
                    Double datediff = ((7 * recurringEntry.WeeksPeriod) - (nowish.Subtract(initial).Days % (7 * recurringEntry.WeeksPeriod))) % (7 * recurringEntry.WeeksPeriod);

                    if (((nowish.AddDays(datediff)).Add(TimeSpan.FromSeconds(recurringEntry.StartTimeInSeconds))) < DateTime.Now)
                        datediff = datediff + (7 * recurringEntry.WeeksPeriod);

                    sb.AppendFormat(CultureConstants.DefaultCulture, "  Day of Week: {0}", recurringEntry.DayOfWeek).AppendLine();
                    sb.AppendFormat(CultureConstants.DefaultCulture, "  Every: {0} week{1}",
                        recurringEntry.WeeksPeriod, (recurringEntry.WeeksPeriod == 1 ? String.Empty : "s")).AppendLine();
                    sb.AppendFormat(CultureConstants.DefaultCulture, "  Next: {0}",
                        (nowish.AddDays(datediff)).Add(TimeSpan.FromSeconds(recurringEntry.StartTimeInSeconds)).ToShortDateString()).AppendLine();
                }

                if (recurringEntry.EndTimeInSeconds > 86400)
                    recurringEntry.EndTimeInSeconds -= 86400;

                sb.AppendFormat(CultureConstants.DefaultCulture, " Start Time: {0}",
                    TimeSpan.FromSeconds(recurringEntry.StartTimeInSeconds).ToString()).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " End Time: {0}",
                    TimeSpan.FromSeconds(recurringEntry.EndTimeInSeconds).ToString()).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " Expired: {0}", recurringEntry.Expired).AppendLine();
                sb.AppendLine();
                sb.AppendLine("Options");
                sb.AppendFormat(CultureConstants.DefaultCulture, " Blocking: {0}",
                    (recurringEntry.Options & ScheduleEntryOptions.Blocking) != ScheduleEntryOptions.None).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " Silent: {0}",
                    (recurringEntry.Options & ScheduleEntryOptions.Quiet) != ScheduleEntryOptions.None).AppendLine();
                sb.AppendFormat(CultureConstants.DefaultCulture, " Uses Eve Time: {0}",
                    (recurringEntry.Options & ScheduleEntryOptions.EVETime) != ScheduleEntryOptions.None).AppendLine();
            }

            // Update the description
            lblEntryDescription.Text = sb.ToString();
        }

        /// <summary>
        /// Removes the selected entry.
        /// </summary>
        private void RemoveSelectedEntry()
        {
            int entryIndex = lbEntries.SelectedIndex;
            ScheduleEntry entry = lbEntriesData[entryIndex];
            Scheduler.Remove(entry);

            // When no entries left, clear the description label
            if (lbEntriesData.IsEmpty())
                lbEntries_SelectedIndexChanged(null, EventArgs.Empty);
        }
        #endregion


        #region Calendar control events
        /// <summary>
        /// Anytime the mouse leaves the calendar control, hide the tooltip.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void calControl_MouseLeave(object sender, EventArgs e)
        {
            m_tooltip.Active = false;
        }

        /// <summary>
        /// Anytime the mouse enters the calendar control, display the tooltip.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void calControl_MouseEnter(object sender, EventArgs e)
        {
            m_tooltip.Active = false;
        }

        /// <summary>
        /// When the user double-clicks a day on the calendar control, we allow him to add a new entry.
        /// </summary>
        /// <param name="datetime"></param>
        /// <param name="mouse"></param>
        /// <param name="loc"></param>
        private void calControl_DayDoubleClicked(DateTime datetime, MouseEventArgs mouse, Point loc)
        {
            using (EditScheduleEntryWindow f = new EditScheduleEntryWindow(datetime))
            {
                DialogResult dr = f.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                Scheduler.Add(f.ScheduleEntry);
            }
        }

        /// <summary>
        /// When the user clicks the calendar controls (main control)
        /// </summary>
        /// <param name="datetime"></param>
        /// <param name="mouse"></param>
        /// <param name="location"></param>
        void calControl_DayClicked(DateTime datetime, MouseEventArgs mouse, Point location)
        {
            if (mouse.Button == MouseButtons.Left)
            {
                ShowCalendarTooltip(datetime);
            }
            else if (mouse.Button == MouseButtons.Right)
            {
                ShowCalendarContextMenu(datetime, location);
            }
        }

        /// <summary>
        /// Show the context menu for the given day
        /// </summary>
        /// <param name="datetime"></param>
        /// <param name="location"></param>
        private void ShowCalendarContextMenu(DateTime datetime, Point location)
        {
            // Remove old submenus
            while (calContext.Items.Count > 2)
            {
                calContext.Items.RemoveAt(2);
            }

            // Set date Tag to new entry
            calContext.Items[0].Tag = datetime;

            // Add "Edit" menus for every schedule on this day
            foreach (ScheduleEntry entry in Scheduler.Entries.Where(x => x.IsToday(datetime)))
            {
                ToolStripItem item = new ToolStripMenuItem();
                item.Text = String.Format(CultureConstants.DefaultCulture, "Edit \"{0}\"...", entry.Title);
                item.Tag = entry;
                item.Click += new EventHandler(editMenuItem_Click);

                calContext.Items.Add(item);
            }

            // Display the menu
            calContext.Show(calControl, location);
        }

        /// <summary>
        /// Shows a tooltip enumerating all of entries for this day
        /// </summary>
        /// <param name="datetime"></param>
        private void ShowCalendarTooltip(DateTime datetime)
        {
            // How can you only localize the date?
            StringBuilder content = new StringBuilder();

            foreach (ScheduleEntry entry in Scheduler.Entries.Where(x => x.IsToday(datetime)).OrderBy(x => x.Title))
            {
                DateTime from = datetime;
                DateTime to = datetime;

                // Simple entry ?
                if (entry is SimpleScheduleEntry)
                {
                    SimpleScheduleEntry simple = (SimpleScheduleEntry)entry;

                    from = simple.StartDate;
                    to = simple.EndDate;
                }
                // Or recurring entry ?
                else
                {
                    RecurringScheduleEntry recurring = (RecurringScheduleEntry)entry;

                    // Does this always have one entry?
                    IEnumerable<ScheduleDateTimeRange> ranges = recurring.GetRangesInPeriod(new DateTime(datetime.Year, datetime.Month, datetime.Day, 0, 0, 0), new DateTime(datetime.Year, datetime.Month, datetime.Day, 23, 59, 59));
                    IEnumerator<ScheduleDateTimeRange> enumranges = ranges.GetEnumerator();
                    while (enumranges.MoveNext())
                    {
                        ScheduleDateTimeRange r = enumranges.Current;
                        from = r.From;
                        to = r.To;
                    }
                }

                // If the "from" date is before the selected date
                if (!(from.Year == datetime.Year && from.Month == datetime.Month && from.Day == datetime.Day))
                {
                    // Set date to midnight today
                    from = new DateTime(datetime.Year, datetime.Month, datetime.Day, 0, 0, 0);
                }

                // If the "to" date is after the selected date
                if (!(to.Year == datetime.Year && to.Month == datetime.Month && to.Day == datetime.Day))
                {
                    // Set date to last second before tomorrows midnight
                    to = new DateTime(datetime.Year, datetime.Month, datetime.Day, 23, 59, 59);
                }

                // Append the tooltip content
                content.Append(entry.Title);

                if ((entry.Options & ScheduleEntryOptions.EVETime) != ScheduleEntryOptions.None)
                {
                    // In case local time conversion extends beyond the entry date,
                    // we display also the ending date
                    var toLocalTime = (to.Day == to.ToLocalTime().Day ?
                        to.ToLocalTime().ToString("HH:mm") : to.ToLocalTime().ToString());

                    content.AppendFormat(" [ EVE Time: {0} - {1} ] ", from.ToString("HH:mm"), to.ToString("HH:mm"));
                    content.AppendFormat(" [ Local Time: {0} - {1} ] ", from.ToLocalTime().ToString("HH:mm"), toLocalTime);
                }
                else
                {
                    content.AppendFormat(" [ {0} - {1} ] ", from.ToString("HH:mm"), to.ToString("HH:mm"));
                }
                content.AppendLine();
            }

            m_tooltip.ToolTipTitle = String.Format(CultureConstants.DefaultCulture, "Entries for {0}", datetime.ToString("d"));
            m_tooltip.SetToolTip(calControl, content.ToString());
            m_tooltip.Active = true;
        }

        /// <summary>
        /// Context menu > Edit "entry name"
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void editMenuItem_Click(object sender, EventArgs e)
        {
            var menu = (ToolStripMenuItem)sender;
            ScheduleEntry entry = (ScheduleEntry)menu.Tag;

            // Allow the user to edit the entry
            using (EditScheduleEntryWindow f = new EditScheduleEntryWindow())
            {
                f.ScheduleEntry = entry;
                DialogResult dr = f.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                Scheduler.Remove(entry);
                Scheduler.Add(f.ScheduleEntry);
            }
        }


        /// <summary>
        /// Context menu > New schedule entry
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void newEntryMenuItem_Click(object sender, EventArgs e)
        {
            var menu = (ToolStripMenuItem)sender;
            DateTime datetime = (DateTime)menu.Tag;

            // Allow the user to define the new entry
            using (EditScheduleEntryWindow f = new EditScheduleEntryWindow(datetime))
            {
                DialogResult dr = f.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                // Add an entry
                Scheduler.Add(f.ScheduleEntry);
            }
        }
        #endregion


        #region Controls' handlers
        /// <summary>
        /// Toolbar > Delete entry.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsbDeleteEntry_Click(object sender, EventArgs e)
        {
            if (lbEntries.SelectedIndex != -1)
                RemoveSelectedEntry();
        }

        /// <summary>
        /// Toolbar > Clear expired
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsbClearExpired_Click(object sender, EventArgs e)
        {
            Scheduler.ClearExpired();

            // When no entries left, clear the description label
            if (lbEntriesData.IsEmpty())
                lbEntries_SelectedIndexChanged(null, EventArgs.Empty);
        }

        /// <summary>
        /// When the user selects another entry on the left listbox, we update the labels below.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbEntries_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lbEntries.SelectedIndex == -1)
            {
                lblEntryDescription.Text = String.Empty;
                return;
            }

            UpdateEntryDescription();
        }


        /// <summary>
        /// When the user double-clicks an entry on the left list box, we open the edition box for this entry.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbEntries_DoubleClick(object sender, EventArgs e)
        {
            if (lbEntries.SelectedIndex == -1)
                return;

            int entryIndex = lbEntries.SelectedIndex;
            ScheduleEntry entry = lbEntriesData[entryIndex];
            using (EditScheduleEntryWindow f = new EditScheduleEntryWindow())
            {
                f.ScheduleEntry = entry;
                DialogResult dr = f.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                Scheduler.Remove(entry);
                Scheduler.Add(f.ScheduleEntry);
            }

            lbEntries.SelectedIndex = entryIndex;
        }

        /// <summary>
        /// Toolbar > Add entry
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void addEntryMenuItem_Click(object sender, EventArgs e)
        {
            using (EditScheduleEntryWindow f = new EditScheduleEntryWindow())
            {
                DialogResult dr = f.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                Scheduler.Add(f.ScheduleEntry);
            }
        }

        /// <summary>
        /// Occur whenever the user changes the numeric box for the year
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void changedYear(object sender, EventArgs e)
        {
            int oldyearnum = m_currentDate.Year;
            m_currentDate = m_currentDate.AddYears((int)nudYear.Value - m_currentDate.Year);
            if (m_currentDate.Month == 2 &&
                (CultureConstants.DefaultCulture.Calendar.IsLeapYear(m_currentDate.Year) ||
                 CultureConstants.DefaultCulture.Calendar.IsLeapYear(oldyearnum)))
            {
                nudDay.Maximum =
                    CultureConstants.DefaultCulture.Calendar.GetDaysInMonth(m_currentDate.Year, m_currentDate.Month) + 1;

                if (CultureConstants.DefaultCulture.Calendar.IsLeapYear(oldyearnum) && nudDay.Value > nudDay.Maximum)
                    nudDay.Value = nudDay.Maximum;
            }
            calControl.Date = m_currentDate;
        }

        /// <summary>
        /// Occur whenever the user changes the numeric box for the day
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void changedDay(object sender, EventArgs e)
        {
            bool donex = false;
            bool doney = false;
            if (nudDay.Value == 0)
            {
                if (nudMonth.SelectedIndex == (nudMonth.Items.Count - 2) && nudYear.Value == nudYear.Minimum)
                    nudDay.Value = 1;

                m_currentDate = m_currentDate.AddDays((int)nudDay.Value - m_currentDate.Day);
                nudDay.Maximum =
                    CultureConstants.DefaultCulture.Calendar.GetDaysInMonth(m_currentDate.Year, m_currentDate.Month) + 1;

                if (nudDay.Value == 0)
                    nudDay.Value =
                        CultureConstants.DefaultCulture.Calendar.GetDaysInMonth(m_currentDate.Year, m_currentDate.Month);

                donex = true;
            }
            else if (!donex && nudDay.Value == nudDay.Maximum)
            {
                if (nudMonth.SelectedIndex == 1 && nudYear.Value == nudYear.Maximum)
                {
                    nudDay.Value =
                        CultureConstants.DefaultCulture.Calendar.GetDaysInMonth(m_currentDate.Year, m_currentDate.Month);

                    doney = true;
                }
                else if (!doney)
                {
                    m_currentDate = m_currentDate.AddDays((int)nudDay.Value - m_currentDate.Day);
                    nudDay.Value = 1;
                    nudDay.Maximum =
                        CultureConstants.DefaultCulture.Calendar.GetDaysInMonth(m_currentDate.Year, m_currentDate.Month) + 1;
                }
                donex = true;
            }
            else if (!donex)
            {
                m_currentDate = m_currentDate.AddDays((int)nudDay.Value - m_currentDate.Day);
            }
            calControl.Date = m_currentDate;
            nudYear.Value = m_currentDate.Year;
            nudMonth.SelectedIndex = (CultureConstants.DefaultCulture.Calendar.GetMonthsInYear(m_currentDate.Year) -
                                      m_currentDate.Month) + 1;
        }

        /// <summary>
        /// Occur whenever the user changes the numeric box for the month
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void changedMonth(object sender, EventArgs e)
        {
            if (nudMonth.SelectedIndex == nudMonth.Items.Count - 1 && nudYear.Value == nudYear.Minimum)
                nudMonth.SelectedIndex = nudMonth.Items.Count - 2;

            if (nudMonth.SelectedIndex == 0 && nudYear.Value == nudYear.Maximum)
                nudMonth.SelectedIndex = 1;

            m_currentDate =
                m_currentDate.AddMonths((((nudMonth.Items.Count - 1) - nudMonth.SelectedIndex) - m_currentDate.Month));

            nudMonth.SelectedIndex = ((nudMonth.SelectedIndex + (nudMonth.Items.Count - 3)) % (nudMonth.Items.Count - 2)) + 1;

            if (nudDay.Value > CultureConstants.DefaultCulture.Calendar.GetDaysInMonth(m_currentDate.Year, m_currentDate.Month))
                nudDay.Value = CultureConstants.DefaultCulture.Calendar.GetDaysInMonth(m_currentDate.Year, m_currentDate.Month);

            nudDay.Maximum = CultureConstants.DefaultCulture.Calendar.GetDaysInMonth(m_currentDate.Year, m_currentDate.Month) + 1;
            calControl.Date = m_currentDate;
            nudYear.Value = m_currentDate.Year;
        }

        private void lbEntries_KeyDown(object sender, KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Delete:
                    RemoveSelectedEntry();
                    break;
                default:
                    break;
            }
        }
        #endregion
    }
}
