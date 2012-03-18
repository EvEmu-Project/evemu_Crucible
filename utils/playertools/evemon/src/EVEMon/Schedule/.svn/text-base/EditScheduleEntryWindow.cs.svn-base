using System;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Scheduling;
using EVEMon.Common.Controls;

namespace EVEMon.Schedule
{
    public partial class EditScheduleEntryWindow : EVEMonForm
    {
        private ScheduleEntry m_scheduleEntry = null;

        private DateTime m_recurringDateFrom;
        private DateTime m_recurringDateTo;
        private DateTime m_oneTimeStartDate;
        private DateTime m_oneTimeEndDate;
        private int m_oneTimeStartTime = 0;
        private int m_oneTimeEndTime = 0;
        private int m_recurringStartTime = 0;
        private int m_recurringEndTime = 0;

        public EditScheduleEntryWindow()
        {
            InitializeComponent();

            InitialEntry();
        }

        public EditScheduleEntryWindow(DateTime defaultTime)
            : this()
        {
            DateTime start = new DateTime(defaultTime.Year, defaultTime.Month, defaultTime.Day, 0, 0,0);
            DateTime end = new DateTime(defaultTime.Year, defaultTime.Month, defaultTime.Day, 23, 59, 59);

            SetOneTimeStartDate(start);
            SetOneTimeEndDate(end);

            SetRecurringDateFrom(start);
            SetRecurringDateTo(end);
        }

        public ScheduleEntry ScheduleEntry
        {
            get { return m_scheduleEntry; }
            set
            {
                m_scheduleEntry = value;
                UpdateFromEntry();
            }
        }

        private void InitialEntry()
        {
            tbTitle.Text = String.Empty;
            SetTypeFlags(ScheduleEntryOptions.None);
            rbOneTime.Checked = true;
            rbRecurring.Checked = false;
            SetOneTimeStartDate(DateTime.Today);
            tbOneTimeStartTime.Text = DateTime.Today.ToCustomShortTimeString();
            DateTime dtto = DateTime.Today + TimeSpan.FromDays(1) - TimeSpan.FromMinutes(1);
            SetOneTimeEndDate(dtto);
            tbOneTimeEndTime.Text = dtto.ToCustomShortTimeString();
            SetRecurringDateFrom(DateTime.MinValue);
            SetRecurringDateTo(DateTime.MaxValue);
            cbRecurringFrequency.SelectedIndex = 0;
            nudWeeklyFrequency.Value = 1;
            nudRecurDayOfMonth.Value = 1;
            cbRecurOnOverflow.SelectedIndex = 0;
            tbRecurringTimeFrom.Text = DateTime.Today.ToCustomShortTimeString();
            tbRecurringTimeTo.Text = dtto.ToCustomShortTimeString();
        }

        private void UpdateFromEntry()
        {
            if (m_scheduleEntry == null)
                return;

            tbTitle.Text = m_scheduleEntry.Title;
            SetTypeFlags(m_scheduleEntry.Options);
            if (m_scheduleEntry is SimpleScheduleEntry)
            {
                SimpleScheduleEntry sse = (SimpleScheduleEntry) m_scheduleEntry;
                rbOneTime.Checked = true;
                rbRecurring.Checked = false;
                SetOneTimeStartDate(sse.StartDate);
                tbOneTimeStartTime.Text = sse.StartDate.ToCustomShortTimeString();
                SetOneTimeEndDate(sse.EndDate);
                tbOneTimeEndTime.Text = sse.EndDate.ToCustomShortTimeString();
            }
            else if (m_scheduleEntry is RecurringScheduleEntry)
            {
                RecurringScheduleEntry rse = (RecurringScheduleEntry) m_scheduleEntry;
                rbOneTime.Checked = false;
                rbRecurring.Checked = true;
                SetRecurringDateFrom(rse.StartDate);
                SetRecurringDateTo(rse.EndDate);
                SetRecurringFrequencyDropdown(rse.Frequency, rse.DayOfWeek, rse.WeeksPeriod);
                nudRecurDayOfMonth.Value = rse.DayOfMonth;
                SetRecurringOverflowDropdown(rse.OverflowResolution);
                DateTime tstart = DateTime.Today + TimeSpan.FromSeconds(rse.StartTimeInSeconds);
                DateTime tend = DateTime.Today + TimeSpan.FromSeconds(rse.EndTimeInSeconds);
                tbRecurringTimeFrom.Text = tstart.ToCustomShortTimeString();
                tbRecurringTimeTo.Text = tend.ToCustomShortTimeString();
            }

            ValidateData();
        }

        private void SetRecurringOverflowDropdown(MonthlyOverflowResolution monthlyOverflowResolution)
        {
            switch (monthlyOverflowResolution)
            {
                default:
                case MonthlyOverflowResolution.Drop:
                    cbRecurOnOverflow.SelectedIndex = 0;
                    break;
                case MonthlyOverflowResolution.OverlapForward:
                    cbRecurOnOverflow.SelectedIndex = 1;
                    break;
                case MonthlyOverflowResolution.ClipBack:
                    cbRecurOnOverflow.SelectedIndex = 2;
                    break;
            }
        }

        private MonthlyOverflowResolution GetRecurringOverflowDropdown()
        {
            switch (cbRecurOnOverflow.SelectedIndex)
            {
                default:
                case 0:
                    return MonthlyOverflowResolution.Drop;
                case 1:
                    return MonthlyOverflowResolution.OverlapForward;
                case 2:
                    return MonthlyOverflowResolution.ClipBack;
            }
        }

        private void SetRecurringFrequencyDropdown(RecurringFrequency recurFrequency, DayOfWeek recurDow, int nWeekly)
        {
            switch (recurFrequency)
            {
                default:
                case RecurringFrequency.Daily:
                    cbRecurringFrequency.SelectedIndex = 0;
                    break;
                case RecurringFrequency.Weekdays:
                    cbRecurringFrequency.SelectedIndex = 1;
                    break;
                case RecurringFrequency.Weekends:
                    cbRecurringFrequency.SelectedIndex = 2;
                    break;
                case RecurringFrequency.Weekly:
                    switch (recurDow)
                    {
                        default:
                        case DayOfWeek.Monday:
                            cbRecurringFrequency.SelectedIndex = 3;
                            break;
                        case DayOfWeek.Tuesday:
                            cbRecurringFrequency.SelectedIndex = 4;
                            break;
                        case DayOfWeek.Wednesday:
                            cbRecurringFrequency.SelectedIndex = 5;
                            break;
                        case DayOfWeek.Thursday:
                            cbRecurringFrequency.SelectedIndex = 6;
                            break;
                        case DayOfWeek.Friday:
                            cbRecurringFrequency.SelectedIndex = 7;
                            break;
                        case DayOfWeek.Saturday:
                            cbRecurringFrequency.SelectedIndex = 8;
                            break;
                        case DayOfWeek.Sunday:
                            cbRecurringFrequency.SelectedIndex = 9;
                            break;
                    }
                    nudWeeklyFrequency.Value = nWeekly;
                    break;
                case RecurringFrequency.Monthly:
                    cbRecurringFrequency.SelectedIndex = 10;
                    break;
            }
        }

        private RecurringFrequency GetRecurringFrequencyDropdown(ref DayOfWeek dow)
        {
            switch (cbRecurringFrequency.SelectedIndex)
            {
                default:
                case 0:
                    return RecurringFrequency.Daily;
                case 1:
                    return RecurringFrequency.Weekdays;
                case 2:
                    return RecurringFrequency.Weekends;
                case 3:
                    dow = DayOfWeek.Monday;
                    return RecurringFrequency.Weekly;
                case 4:
                    dow = DayOfWeek.Tuesday;
                    return RecurringFrequency.Weekly;
                case 5:
                    dow = DayOfWeek.Wednesday;
                    return RecurringFrequency.Weekly;
                case 6:
                    dow = DayOfWeek.Thursday;
                    return RecurringFrequency.Weekly;
                case 7:
                    dow = DayOfWeek.Friday;
                    return RecurringFrequency.Weekly;
                case 8:
                    dow = DayOfWeek.Saturday;
                    return RecurringFrequency.Weekly;
                case 9:
                    dow = DayOfWeek.Sunday;
                    return RecurringFrequency.Weekly;
                case 10:
                    return RecurringFrequency.Monthly;
            }
        }

        private void SetTypeFlags(ScheduleEntryOptions scheduleEntryOptions)
        {
            cbBlocking.Checked = ((scheduleEntryOptions & ScheduleEntryOptions.Blocking) != 0);
            cbSilent.Checked = ((scheduleEntryOptions & ScheduleEntryOptions.Quiet) != 0);
            cbUseEVETime.Checked = ((scheduleEntryOptions & ScheduleEntryOptions.EVETime) != 0);
        }

        private ScheduleEntryOptions GetTypeFlags()
        {
            ScheduleEntryOptions result = ScheduleEntryOptions.None;
            if (cbBlocking.Checked)
                result |= ScheduleEntryOptions.Blocking;

            if (cbSilent.Checked)
                result |= ScheduleEntryOptions.Quiet;

            if (cbUseEVETime.Checked)
                result |= ScheduleEntryOptions.EVETime;

            return result;
        }

        private void SetRecurringDateTo(DateTime dateTime)
        {
            if (dateTime == DateTime.MinValue || dateTime == DateTime.MaxValue)
            {
                tbRecurringEndDate.Text = "(Forever)";
                m_recurringDateTo = DateTime.MaxValue;
            }
            else
            {
                tbRecurringEndDate.Text = dateTime.ToLongDateString();
                m_recurringDateTo = StripToDate(dateTime);
            }
        }

        private void SetRecurringDateFrom(DateTime dateTime)
        {
            if (dateTime == DateTime.MinValue || dateTime == DateTime.MaxValue)
            {
                tbRecurringStartDate.Text = "(Forever)";
                m_recurringDateFrom = DateTime.MinValue;
            }
            else
            {
                tbRecurringStartDate.Text = dateTime.ToLongDateString();
                m_recurringDateFrom = StripToDate(dateTime);
            }
        }

        private DateTime StripToDate(DateTime dateTime)
        {
            return dateTime -
                   TimeSpan.FromHours(dateTime.Hour) -
                   TimeSpan.FromMinutes(dateTime.Minute) -
                   TimeSpan.FromSeconds(dateTime.Second) -
                   TimeSpan.FromMilliseconds(dateTime.Millisecond);
        }

        private void SetOneTimeStartDate(DateTime dateTime)
        {
            tbOneTimeStartDate.Text = dateTime.ToLongDateString();
            m_oneTimeStartDate = StripToDate(dateTime);
        }

        private void SetOneTimeEndDate(DateTime dateTime)
        {
            tbOneTimeEndDate.Text = dateTime.ToLongDateString();
            m_oneTimeEndDate = StripToDate(dateTime);
        }

        private void rbOneTime_CheckedChanged(object sender, EventArgs e)
        {
            pnlOneTime.Enabled = rbOneTime.Checked;
            
            ValidateData();
        }

        private void rbRecurring_CheckedChanged(object sender, EventArgs e)
        {
            pnlRecurring.Enabled = rbRecurring.Checked;

            ValidateData();
        }

        private void cbRecurringFrequency_SelectedIndexChanged(object sender, EventArgs e)
        {
            pnlRecurMonthly.Enabled = (cbRecurringFrequency.SelectedIndex == 10);
            pnlRecurWeekly.Enabled = (cbRecurringFrequency.SelectedIndex >= 3 && cbRecurringFrequency.SelectedIndex <= 9);
        }

        private void btnRecurringNoStartDate_Click(object sender, EventArgs e)
        {
            SetRecurringDateFrom(DateTime.MinValue);
        }

        private void btnRecurringNoEndDate_Click(object sender, EventArgs e)
        {
            SetRecurringDateTo(DateTime.MaxValue);
        }

        private bool TryParseTime(string text, out int seconds)
        {
            DateTime res;
            if (!DateTime.TryParse("2000/01/01 " + text, out res))
            {
                seconds = 0;
                return false;
            }

            DateTime b = new DateTime(2000, 1, 1);
            TimeSpan diff = res - b;
            seconds = Convert.ToInt32(diff.TotalSeconds);
            return true;
        }

        private void ValidateData()
        {
            bool valid = true;
            if (String.IsNullOrEmpty(tbTitle.Text) || String.IsNullOrEmpty(tbTitle.Text.Trim()))
            {
                valid = false;
            }
            else
            {
                if (rbOneTime.Checked)
                {
                    int startSec;
                    int endSec;
                    if (!TryParseTime(tbOneTimeStartTime.Text, out startSec) ||
                        !TryParseTime(tbOneTimeEndTime.Text, out endSec))
                    {
                        valid = false;
                    }
                    else
                    {
                        DateTime startDate = m_oneTimeStartDate + TimeSpan.FromSeconds(startSec);
                        DateTime endDate = m_oneTimeEndDate + TimeSpan.FromSeconds(endSec);
                        if (startDate >= endDate)
                        {
                            valid = false;
                        }
                        else
                        {
                            m_oneTimeStartTime = startSec;
                            m_oneTimeEndTime = endSec;
                        }
                    }
                }
                else if (rbRecurring.Checked)
                {
                    if (m_recurringDateFrom > m_recurringDateTo)
                    {
                        valid = false;
                    }
                    else
                    {
                        int startSec;
                        int endSec;
                        if (!TryParseTime(tbRecurringTimeFrom.Text, out startSec) ||
                            !TryParseTime(tbRecurringTimeTo.Text, out endSec))
                        {
                            valid = false;
                        }
                        else
                        {
                            if (startSec >= endSec)
                            {
                                endSec += RecurringScheduleEntry.SecondsPerDay;
                            }
                            m_recurringStartTime = startSec;
                            m_recurringEndTime = endSec;
                        }
                    }
                }
                else
                {
                    valid = false;
                }
            }

            btnOk.Enabled = valid;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            m_scheduleEntry = GenerateScheduleEntry();
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private ScheduleEntry GenerateScheduleEntry()
        {
            ScheduleEntry result = null;
            if (rbOneTime.Checked)
            {
                SimpleScheduleEntry sse = new SimpleScheduleEntry();
                sse.StartDate = new DateTime(
                    (m_oneTimeStartDate + TimeSpan.FromSeconds(m_oneTimeStartTime)).Ticks, DateTimeKind.Unspecified);
                sse.EndDate = new DateTime(
                    (m_oneTimeEndDate + TimeSpan.FromSeconds(m_oneTimeEndTime)).Ticks, DateTimeKind.Unspecified);

                result = sse;
            }
            else if (rbRecurring.Checked)
            {
                RecurringScheduleEntry rse = new RecurringScheduleEntry();
                rse.StartDate = m_recurringDateFrom;
                rse.EndDate = m_recurringDateTo;
                DayOfWeek dow = DayOfWeek.Monday;
                rse.Frequency = GetRecurringFrequencyDropdown(ref dow);
                rse.DayOfWeek = dow;
                if (rse.Frequency == RecurringFrequency.Weekly)
                {
                    rse.WeeksPeriod = Convert.ToInt32(nudWeeklyFrequency.Value);
                }
                rse.DayOfMonth = Convert.ToInt32(nudRecurDayOfMonth.Value);
                rse.OverflowResolution = GetRecurringOverflowDropdown();
                rse.StartTimeInSeconds = m_recurringStartTime;
                rse.EndTimeInSeconds = m_recurringEndTime;
                result = rse;
            }

            if (result != null)
            {
                result.Title = tbTitle.Text;
                result.Options = GetTypeFlags();
            }

            return result;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (!btnOk.Enabled)
            {
                return;
            }

            ScheduleEntry ise = GenerateScheduleEntry();
            this.ScheduleEntry = ise;
        }

        private void tbTitle_TextChanged(object sender, EventArgs e)
        {
            ValidateData();
        }

        private void tbOneTimeStartTime_TextChanged(object sender, EventArgs e)
        {
            ValidateData();
        }

        private void tbOneTimeEndTime_TextChanged(object sender, EventArgs e)
        {
            ValidateData();
        }

        private void tbRecurringTimeFrom_TextChanged(object sender, EventArgs e)
        {
            ValidateData();
        }

        private void tbRecurringTimeTo_TextChanged(object sender, EventArgs e)
        {
            ValidateData();
        }

        private void EditScheduleEntryWindow_Load(object sender, EventArgs e)
        {
        }

        private bool GetDate(ref DateTime res)
        {
            using (DateSelectWindow f = new DateSelectWindow())
            {
                if (res == DateTime.MinValue || res == DateTime.MaxValue)
                {
                    f.SelectedDate = DateTime.Today;
                }
                else
                {
                    f.SelectedDate = res;
                }

                DialogResult dr = f.ShowDialog();

                if (dr == DialogResult.Cancel)
                    return false;

                res = f.SelectedDate;
                return true;
            }
        }

        private void btnOneTimeStartDateChoose_Click(object sender, EventArgs e)
        {
            if (GetDate(ref m_oneTimeStartDate))
                SetOneTimeStartDate(m_oneTimeStartDate);

            ValidateData();
        }

        private void btnOneTimeEndDateChoose_Click(object sender, EventArgs e)
        {
            if (GetDate(ref m_oneTimeEndDate))
                SetOneTimeEndDate(m_oneTimeEndDate);

            ValidateData();
        }

        private void btnRecurringStartDateChoose_Click(object sender, EventArgs e)
        {
            if (GetDate(ref m_recurringDateFrom))
                SetRecurringDateFrom(m_recurringDateFrom);

            ValidateData();
        }

        private void btnRecurringEndDateChoose_Click(object sender, EventArgs e)
        {
            if (GetDate(ref m_recurringDateTo))
                SetRecurringDateTo(m_recurringDateTo);

            ValidateData();
        }
    }
}
