using System;
using System.Collections.Generic;
using System.Text;
using System.Data;
using System.Collections;
using Google.GData.AccessControl;
using Google.GData.Calendar;
using Google.GData.Client;
using Google.GData.Extensions;
using EVEMon.Common;
using System.Globalization;

namespace SNCalendar
{
    /// <summary>
    /// Common Appointment Class used for both instances of the calendar.
    /// </summary>
    abstract class AppointmentFilter
    {
        #region Protected Internals
        protected DateTime startDate;
        protected DateTime endDate;
        protected string subject;
        protected bool reminder;
        protected int minutes;
        protected string entryID;
        protected bool alternateReminder;
        protected DateTime earlyReminder;
        protected DateTime lateReminder;
        protected ArrayList appointmentArray;
        #endregion

        #region Abstract Methods
        public abstract void ReadAppointments();
        public abstract bool GetAppointment(int iIndex);
        public abstract bool AddAppointment();
        public abstract void UpdateAppointment(int iIndex);
        public abstract void DeleteAppointment(int iIndex);
        #endregion

        #region Public Properties
        public DateTime StartDate
        {
            get
            {
                return startDate;
            }
            set
            {
                startDate = value;
            }
        }
        public DateTime EndDate
        {
            get
            {
                return endDate;
            }
            set
            {
                endDate = value;
            }
        }
        public string Subject
        {
            get
            {
                return subject;
            }
            set
            {
                subject = value;
            }
        }
        public bool ItemReminder
        {
            get
            {
                return reminder;
            }
            set
            {
                reminder = value;
            }
        }
        public int Minutes
        {
            get
            {
                return minutes;
            }
            set
            {
                minutes = value;
            }
        }
        public string EntryID
        {
            get
            {
                return entryID;
            }
            set
            {
                entryID = value;
            }
        }
        public bool AlternateReminder
        {
            get
            {
                return alternateReminder;
            }
            set
            {
                alternateReminder = value;
            }
        }
        public DateTime EarlyReminder
        {
            get
            {
                return earlyReminder;
            }
            set
            {
                earlyReminder = value;
            }
        }
        public DateTime LateReminder
        {
            get
            {
                return lateReminder;
            }
            set
            {
                lateReminder = value;
            }
        }
        public int ItemCount
        {
            get
            {
                return appointmentArray.Count;
            }
        }
        #endregion

        #region Public Methods
        public AppointmentFilter()
        {
            startDate = DateTime.Now;
            endDate = DateTime.Now.AddDays(1);
            subject = String.Empty;
            reminder = true;
            minutes = 5;
            entryID = String.Empty;
            alternateReminder = false;
            earlyReminder = new DateTime(DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day, 8, 0, 0);
            lateReminder = new DateTime(DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day, 20, 0, 0);
            appointmentArray = new ArrayList();
        }
        public DateTime WorkOutAlternateReminders()
        {
            // See whether the appointment falls within the middle of the two, it it does, set the early reminder.
            if ((startDate >= earlyReminder) && (startDate <= lateReminder))
            {
                return earlyReminder;
            }

            // If the appointment falls outside the two reminders: if the appointment is before the early reminder, set 
            // the late reminder to the day before....
            if (startDate < earlyReminder)
            {
                DateTime lateRem = lateReminder;
                lateRem = lateRem.AddDays(-1);
                return lateRem;
            }
            return lateReminder;
        }
        #endregion
    }

    #region Outlook Class
    class OutlookAppointmentFilter : AppointmentFilter
    {
        #region Private Internals
        private static Outlook.Application outlookApplication;
        Outlook.AppointmentItem appointmentItem;
        #endregion

        #region Public Properties
        public static Outlook.Application OutlookApplication
        {
            get
            {
                if (outlookApplication == null)
                {
                    try
                    {
                        outlookApplication = new Outlook.Application();
                    }
                    catch (Exception)
                    {
                        throw;
                    }
                }
                return (outlookApplication);
            }
        }
        #endregion

        #region Constructor
        public OutlookAppointmentFilter()
        {
/*            startDate = DateTime.Now;
            endDate = DateTime.Now.AddDays(1);
            subject = String.Empty;
            reminder = true;
            minutes = 5;
            entryID = String.Empty;
            alternateReminder = false;
            earlyReminder = new DateTime(DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day, 8, 0, 0);
            lateReminder = new DateTime(DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day, 20, 0, 0);*/
        }
        #endregion

        #region Public Methods and Implementations
        public override bool AddAppointment()
        {
            try
            {
                Outlook._AppointmentItem appointmentItem = (Outlook._AppointmentItem)
                    OutlookApplication.CreateItem(Outlook.OlItemType.olAppointmentItem);

                appointmentItem.Subject = subject;
                appointmentItem.Start = startDate;
                appointmentItem.End = endDate;
                appointmentItem.Body = String.Format(CultureConstants.DefaultCulture, "Added: {0}", DateTime.Now);

                appointmentItem.ReminderSet = reminder;

                appointmentItem.BusyStatus = Outlook.OlBusyStatus.olBusy;
                appointmentItem.AllDayEvent = false;
                appointmentItem.Location = String.Empty;

                if (alternateReminder)
                {
                    earlyReminder = new DateTime(startDate.Year, startDate.Month, startDate.Day, earlyReminder.Hour, earlyReminder.Minute, earlyReminder.Second);
                    lateReminder = new DateTime(startDate.Year, startDate.Month, startDate.Day, lateReminder.Hour, lateReminder.Minute, lateReminder.Second);

                    DateTime dtReminder = WorkOutAlternateReminders();
                    // Subtract the reminder time from the appointment time.
                    TimeSpan timeSpan = appointmentItem.Start.Subtract(dtReminder);
                    appointmentItem.ReminderMinutesBeforeStart = Math.Abs((timeSpan.Hours * 60) + timeSpan.Minutes);
                    minutes = appointmentItem.ReminderMinutesBeforeStart;
                }

                if (reminder)
                    appointmentItem.ReminderMinutesBeforeStart = minutes;
                else
                    appointmentItem.ReminderMinutesBeforeStart = 0;

                appointmentItem.Save();
                return true;
            }
            catch (Exception)
            {
                throw;
            }
        }

        public override void UpdateAppointment(int iIndex)
        { 
            appointmentItem = (Outlook.AppointmentItem)appointmentArray[iIndex];

            appointmentItem.Subject = subject;
            appointmentItem.Start = startDate;
            appointmentItem.End = endDate;
            appointmentItem.Body = String.Format(CultureConstants.DefaultCulture, "{0} \r\nUpdated: {1}", appointmentItem.Body, DateTime.Now);

            appointmentItem.ReminderSet = reminder;
            if (reminder)
                appointmentItem.ReminderMinutesBeforeStart = minutes;
            else
                appointmentItem.ReminderMinutesBeforeStart = 0;

            appointmentItem.BusyStatus = Outlook.OlBusyStatus.olBusy;
            appointmentItem.AllDayEvent = false;
            appointmentItem.Location = String.Empty;

            if (alternateReminder)
            {
                earlyReminder = new DateTime(startDate.Year, startDate.Month, startDate.Day, earlyReminder.Hour, earlyReminder.Minute, earlyReminder.Second);
                lateReminder = new DateTime(startDate.Year, startDate.Month, startDate.Day, lateReminder.Hour, lateReminder.Minute, lateReminder.Second);

                DateTime dtReminder = WorkOutAlternateReminders();
                // Subtract the reminder time from the appointment time.
                TimeSpan timeSpan = appointmentItem.Start.Subtract(dtReminder);
                appointmentItem.ReminderMinutesBeforeStart = Math.Abs((timeSpan.Hours * 60) + timeSpan.Minutes);
                minutes = appointmentItem.ReminderMinutesBeforeStart;
            }
            appointmentItem.Save();
        }

        public override bool GetAppointment(int iIndex)
        {
            if (iIndex > appointmentArray.Count - 1)
                return false;
            if (iIndex < 0)
                return false;

            appointmentItem = (Outlook.AppointmentItem)appointmentArray[iIndex];
            startDate = appointmentItem.Start;
            endDate = appointmentItem.End;
            subject = appointmentItem.Subject;
            reminder = appointmentItem.ReminderSet;
            minutes = appointmentItem.ReminderMinutesBeforeStart;
            entryID = appointmentItem.EntryID;
            return true;
        }

        public override void ReadAppointments()
        {
            // Appointment Filter class that will handle any data attached
            // to the appointment with which we are currently dealing.
            appointmentArray = RecurringItems();
        }

        private ArrayList RecurringItems()
        {
            // Filter all the objects we are looking for.
            Outlook.AppointmentItem appoinmentItem = null;
            Outlook.MAPIFolder folder = OutlookApplication.Session.GetDefaultFolder(
                Outlook.OlDefaultFolders.olFolderCalendar)
                as Outlook.MAPIFolder;
            // Use a Jet Query to filter the details we need initially between
            // the two spcified dates.
            string dateFilter = String.Format(
                CultureConstants.DefaultCulture, 
                "[Start] >= '{0:g}' and [End] <= '{1:g}'", 
                startDate, 
                endDate);
            Outlook.Items calendarItems = folder.Items.Restrict(dateFilter);
            calendarItems.Sort("[Start]", Type.Missing);
            calendarItems.IncludeRecurrences = true;

            // Must use 'like' comparison for Find/FindNext
            string subjectFilter = String.Empty;
            if (String.IsNullOrEmpty(subject))
            {
                subjectFilter = "@SQL="
                    + "\"" + "urn:schemas:httpmail:subject" + "\""
                    + " like '%" + subject + "%'";
            }
            else
            {
                subjectFilter = "@SQL="
                    + "\"" + "urn:schemas:httpmail:subject" + "\""
                    + " <> '!@#'";
            }
            // Use Find and FindNext methods to get all the items.
            ArrayList resultArray = new ArrayList();
            appoinmentItem = calendarItems.Find(subjectFilter)
                as Outlook.AppointmentItem;
            while (appoinmentItem != null)
            {
                resultArray.Add(appoinmentItem);
                // Find the next appointment.
                appoinmentItem = calendarItems.FindNext()
                    as Outlook.AppointmentItem;
            }
            return resultArray;
        }

        public override void DeleteAppointment(int iIndex)
        {
            appointmentItem = (Outlook.AppointmentItem)appointmentArray[iIndex];
            appointmentItem.Delete();
        }
        #endregion
    }
    #endregion

    #region Google Class
    class GoogleAppointmentFilter : AppointmentFilter
    {
        #region Private Internals
        private string userName;
        private string password;
        private string uri;
        private int reminderMethod;
        private CalendarService service;
        private EventEntry appointmentItem;
        private ArrayList googleMinutes = new ArrayList();
        private ArrayList googleReminders = new ArrayList();
        #endregion

        #region Public Properties
        public string UserName
        {
            set
            {
                userName = value;
            }
        }
        public string Password
        {
            set
            {
                password = value;
            }
        }
        public string URI
        { 
            set 
            {
                uri = value;
            }
        }
        public int ReminderMethod
        {
            get
            {
                return reminderMethod;
            }
            set
            {
                reminderMethod = value;
            }
        }
        public CalendarService Service
        {
            get
            {
                return service;
            }
        }
        #endregion

        #region Constructor
        public GoogleAppointmentFilter(string serviceName)
        {
            userName = String.Empty;
            password = String.Empty;
            uri = NetworkConstants.GoogleCalendarURL;
            service = new CalendarService(serviceName);
            appointmentItem = new EventEntry();
            PopulateGoogleMinutes();
            PopulateGoogleReminders();
        }
        #endregion

        #region Public and Private Methods and Implementations
        public void Logon()
        {
            service.setUserCredentials(userName, password);
        }

        private void PopulateGoogleMinutes()
        {
            googleMinutes.Add(5);
            googleMinutes.Add(10);
            googleMinutes.Add(15);
            googleMinutes.Add(20);
            googleMinutes.Add(25);
            googleMinutes.Add(30);
            googleMinutes.Add(45);
            googleMinutes.Add(60);  // 1 hour
            googleMinutes.Add(120); // 2 hours
            googleMinutes.Add(180); // 3 hours
            googleMinutes.Add(720); // 12 hours
        }

        private void PopulateGoogleReminders()
        {
            googleReminders.Add(Reminder.ReminderMethod.alert);
            googleReminders.Add(Reminder.ReminderMethod.all);
            googleReminders.Add(Reminder.ReminderMethod.email);
            googleReminders.Add(Reminder.ReminderMethod.none);
            googleReminders.Add(Reminder.ReminderMethod.sms);
            googleReminders.Add(Reminder.ReminderMethod.unspecified);
        }

        public override bool AddAppointment()
        {
            Exception googleProblem = null;
            EventEntry entry = new EventEntry();
            int defaultMinutes = minutes;
            // Set the title and content of the entry.
            entry.Title.Text = subject;

            When eventTime = new When();
            eventTime.StartTime = startDate;
            eventTime.EndTime = endDate;
            entry.Times.Add(eventTime);

            if (alternateReminder)
            {
                earlyReminder = new DateTime(startDate.Year, startDate.Month, startDate.Day, earlyReminder.Hour, earlyReminder.Minute, earlyReminder.Second);
                lateReminder = new DateTime(startDate.Year, startDate.Month, startDate.Day, lateReminder.Hour, lateReminder.Minute, lateReminder.Second); 

                DateTime dtReminder = WorkOutAlternateReminders();
                // Subtract the reminder time from the appointment time.
                TimeSpan timeSpan = entry.Times[0].StartTime.Subtract(dtReminder);
                minutes = Math.Abs((timeSpan.Hours * 60) + timeSpan.Minutes);
            }

            if (reminder)
            {
                entry.Reminder = new Reminder();
                try
                {
                    entry.Reminder.Minutes = GetGoogleMinutes(minutes, defaultMinutes);
                }
                catch (Exception ex)
                {
                    // We set the minutes to the default.
                    entry.Reminder.Minutes = minutes;
                    googleProblem = ex;
                }
                entry.Reminder.Method = (Reminder.ReminderMethod)googleReminders[reminderMethod];
            }
            else
            {
                entry.Reminder = null;
            }

            // Send the request and receive the response:
            Uri postUri = new Uri(uri);

            try
            {
                AtomEntry insertedEntry = service.Insert(postUri, entry);
            }
            catch (Exception)
            {
                throw;
            }
            if (googleProblem != null)
                throw googleProblem;
            return true;
        }

        public override void UpdateAppointment(int iIndex)
        {
            Exception googleProblem = null;
            int defaultMinutes = minutes;

            appointmentItem = (EventEntry)appointmentArray[iIndex];

            appointmentItem.Times[0].StartTime = startDate;
            appointmentItem.Times[0].EndTime = endDate;
            appointmentItem.Title.Text = subject;

            if (alternateReminder)
            {
                earlyReminder = new DateTime(startDate.Year, startDate.Month, startDate.Day, earlyReminder.Hour, earlyReminder.Minute, earlyReminder.Second);
                lateReminder = new DateTime(startDate.Year, startDate.Month, startDate.Day, lateReminder.Hour, lateReminder.Minute, lateReminder.Second);

                DateTime dtReminder = WorkOutAlternateReminders();
                // Subtract the reminder time from the appointment time.
                TimeSpan timeSpan = appointmentItem.Times[0].StartTime.Subtract(dtReminder);
                minutes = Math.Abs((timeSpan.Hours * 60) + timeSpan.Minutes);
            }

            if (reminder)
            {
                // They may not have had a reminder on the inital appointment.
                if (appointmentItem.Reminder == null)
                    appointmentItem.Reminder = new Reminder();
                try
                {
                    appointmentItem.Reminder.Minutes = GetGoogleMinutes(minutes, defaultMinutes);
                }
                catch (Exception ex)
                {
                    googleProblem = ex;
                    // We set the minutes.
                    appointmentItem.Reminder.Minutes = minutes;
                }
                appointmentItem.Reminder.Method = (Reminder.ReminderMethod)googleReminders[reminderMethod];
            }
            else
            {
                appointmentItem.Reminder = null;
            }
            appointmentItem.Update();
            // If there was a problem, throw the excpetion to let the calling app know.
            if (googleProblem != null)
                throw googleProblem;
        }

        private int GetGoogleMinutes(int inMinutes, int defaultMinutes)
        { 
            // Google can't handle superfluous minites so we have to change
            // the reminder to the closest number of minutes that Google can handle.

            // Run through the list and see which fits the bill.
            // 5, 10, 15, 20, 25, 30, 45, 1 hour, 2 hours, 3 hours, 12 hours.
            int nextBestTime = 0;
            bool timeFound = false;
            for (int i = 0; i < googleMinutes.Count; i++)
            {
                nextBestTime = (int)googleMinutes[i];
                if (nextBestTime >= inMinutes)
                {
                    inMinutes = nextBestTime;
                    timeFound = true;
                    break;
                }
            }
            // If we couldn't find a suitable reminder.
            if (!timeFound)
            {
                // Change the reminder minutes so that it still gets set.
                minutes = defaultMinutes;
                throw new Exception(String.Format(CultureConstants.DefaultCulture, "Google could not cater for your early/late reminder. Defaulting to default reminder: {0} minutes", defaultMinutes));
            }

            // If what they sent us has changed....
            if (inMinutes != minutes)
            {
                // Change the reminder minutes so that it still gets set.
                minutes = inMinutes;
                throw new Exception("Early/late reminder had to be changed to cater for Google reminders.");
            }
            return inMinutes;
        }

        public override bool GetAppointment(int iIndex)
        {
            if (iIndex > appointmentArray.Count - 1)
                return false;

            // The Google query doesn't automatically filter on Subject
            // so we need to manually process this.
            if (iIndex < 0)
            {
                bool result = false;
                for (int i = 0; i < appointmentArray.Count; i++)
                {
                    appointmentItem = (EventEntry)appointmentArray[i];
                    if (appointmentItem.Title.Text == subject)
                    {
                        iIndex = i;
                        result = true;
                        break;
                    }
                }

                if (!result)
                    return false;
            }

            appointmentItem = (EventEntry)appointmentArray[iIndex];
            startDate = appointmentItem.Times[0].StartTime;
            endDate = appointmentItem.Times[0].EndTime;
            subject = appointmentItem.Title.Text;

            if (appointmentItem.Reminder != null)
            {
                Reminder.ReminderMethod r = new Reminder.ReminderMethod();
                r = (Reminder.ReminderMethod)appointmentItem.Reminder.Method;
                if (r == Google.GData.Extensions.Reminder.ReminderMethod.none)
                    reminder = false;
                else
                    reminder = true;
                reminderMethod = googleReminders.IndexOf(r);
                minutes = appointmentItem.Reminder.Minutes;
            }
            else
            {
                reminder = false;
                minutes = 5;
            }
            entryID = appointmentItem.Id.Uri.ToString();
            return true;
        }

        public override void ReadAppointments()
        {
            EventQuery myQuery = new EventQuery(uri);
            myQuery.StartTime = startDate;
            myQuery.EndTime = endDate;
            if (String.IsNullOrEmpty(subject))
                myQuery.Query = subject;

            try
            {
                appointmentArray.Clear();

                EventFeed myResultsFeed = service.Query(myQuery) as EventFeed;
                for (int i = 0; i < myResultsFeed.Entries.Count; i++)
                {
                    EventEntry eE = myResultsFeed.Entries[i] as EventEntry;
                    appointmentArray.Add(eE);
                }
            }
            catch (Exception)
            {
                throw;
            }
        }

        public override void DeleteAppointment(int iIndex)
        {
            appointmentItem = (EventEntry)appointmentArray[iIndex];
            appointmentItem.Delete();
        }
        #endregion
    }
    #endregion
}



