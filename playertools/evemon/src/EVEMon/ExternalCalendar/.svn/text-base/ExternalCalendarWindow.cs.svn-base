using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common.Controls;

namespace EVEMon.ExternalCalendar
{
    /// <summary>
    /// Reqests confirmation of settings before updating Google Calendar/Outlook
    /// </summary>
    public partial class ExternalCalendarWindow : EVEMonForm
    {
        private Character m_character;

        public ExternalCalendarWindow(Character character)
        {
            InitializeComponent();

            m_character = character;

            if (Settings.Calendar.Provider == CalendarProvider.Outlook) OutlookRadioButton.Checked = true;
            else GoogleRadioButton.Checked = true;

            GoogleEmailTextBox.Text = Settings.Calendar.GoogleEmail;
            GooglePasswordTextBox.Text = Settings.Calendar.GooglePassword;
            GoogleUriTextBox.Text = Settings.Calendar.GoogleURL;
            GoogleReminderComboBox.SelectedIndex = (int)Settings.Calendar.GoogleReminder;
            SetReminderCheckBox.Checked = Settings.Calendar.UseReminding;
            ReminderTextBox.Text = Settings.Calendar.RemindingInterval.ToString();
            UseAlterateReminderCheckBox.Checked = Settings.Calendar.UseRemindingRange;
            EarlyReminderDateTimePicker.Value = Settings.Calendar.EarlyReminding;
            LateReminderDateTimePicker.Value = Settings.Calendar.LateReminding;
        }

        private void rbMSOutlook_Click(object sender, EventArgs e)
        {
            GoogleGroupBox.Enabled = GoogleRadioButton.Checked;
        }

        private void btnConfirm_Click(object sender, EventArgs e)
        {
            if (OutlookRadioButton.Checked)
                DoOutlookAppointment();
            else
            {
                try
                {
                    DoGoogleAppointment();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
            }
            this.Close();
        }

        void tbReminder_Validating(object sender, System.ComponentModel.CancelEventArgs e)
        {
            int value;
            if (!Int32.TryParse(ReminderTextBox.Text, out value) && value > 0)
            {
                MessageBox.Show("The interval must be a strictly positive integer");
            }
        }

        private void DoOutlookAppointment()
        {
            try
            {
                SNCalendar.OutlookAppointmentFilter outlookAppointmentFilter = new SNCalendar.OutlookAppointmentFilter();

                // Check whether this is a new appointment or an update.
                outlookAppointmentFilter.StartDate = DateTime.Now.AddDays(-40);
                outlookAppointmentFilter.EndDate = DateTime.Now.AddDays(100);
                // Set the subject to the character name and the skill being trained for uniqueness sakes.
                outlookAppointmentFilter.Subject = String.Format(CultureConstants.DefaultCulture, "{0} - {1}", m_character.Name, m_character.CurrentlyTrainingSkill.SkillName);
                // Pull the list of appointments. Hopefully we should either get 1 or none back.
                outlookAppointmentFilter.ReadAppointments();

                // If there is an appointment, get the first one.
                if (outlookAppointmentFilter.ItemCount > 0)
                    outlookAppointmentFilter.GetAppointment(0);

                // Update the appointment we may have pulled or the new one.
                // Set the appoinment length to 5 minutes, starting at the estimated completion date and time.
                outlookAppointmentFilter.StartDate = m_character.CurrentlyTrainingSkill.EndTime;
                outlookAppointmentFilter.EndDate = m_character.CurrentlyTrainingSkill.EndTime.AddMinutes(5);
                outlookAppointmentFilter.ItemReminder = Settings.Calendar.UseReminding;
                outlookAppointmentFilter.AlternateReminder = Settings.Calendar.UseRemindingRange;
                outlookAppointmentFilter.EarlyReminder = Settings.Calendar.EarlyReminding;
                outlookAppointmentFilter.LateReminder = Settings.Calendar.LateReminding;
                outlookAppointmentFilter.Minutes = Settings.Calendar.RemindingInterval;
                if (outlookAppointmentFilter.ItemCount > 0)
                    outlookAppointmentFilter.UpdateAppointment(0);
                else
                    outlookAppointmentFilter.AddAppointment();
            }
            catch (Exception ex)
            {
                MessageBox.Show(String.Format(CultureConstants.DefaultCulture, "{0} {1}{1} {2}", "There was a problem accessing the Outlook Libraries, is Outlook installed on this machine?", Environment.NewLine, ex.Message), "Problem intergrating to Outlook");
                return;
            }
        }

        private void DoGoogleAppointment()
        {
            try
            {
                SNCalendar.GoogleAppointmentFilter googleAppointmentFilter = new SNCalendar.GoogleAppointmentFilter("serviceName");

                googleAppointmentFilter.UserName = Settings.Calendar.GoogleEmail;
                googleAppointmentFilter.Password = Settings.Calendar.GooglePassword;
                googleAppointmentFilter.URI = Settings.Calendar.GoogleURL;
                googleAppointmentFilter.Logon();

                // Check whether this is a new appointment or an update.
                googleAppointmentFilter.StartDate = DateTime.Now.AddDays(-40);
                googleAppointmentFilter.EndDate = DateTime.Now.AddDays(100);
                // Set the subject to the chanracter name and the skill being trained for uniqueness sakes.
                googleAppointmentFilter.Subject = String.Format(CultureConstants.DefaultCulture, "{0} - {1}", m_character.Name, m_character.CurrentlyTrainingSkill.SkillName);
                // Pull the list of appointments. Hopefully we should either get 1 or none back.
                googleAppointmentFilter.ReadAppointments();

                // If there is are appointments, see if any match the subject.
                bool foundAppointment = false;
                if (googleAppointmentFilter.ItemCount > 0)
                    if (googleAppointmentFilter.GetAppointment(-1))
                        foundAppointment = true;

                // Update the appointment we may have pulled or the new one.
                // Set the appointment length to 5 minutes, starting at the estimated completion date and time.
                googleAppointmentFilter.StartDate = m_character.CurrentlyTrainingSkill.EndTime;
                googleAppointmentFilter.EndDate = m_character.CurrentlyTrainingSkill.EndTime.AddMinutes(5);
                googleAppointmentFilter.ItemReminder = Settings.Calendar.UseReminding;
                googleAppointmentFilter.AlternateReminder = Settings.Calendar.UseRemindingRange;
                googleAppointmentFilter.EarlyReminder = Settings.Calendar.EarlyReminding;
                googleAppointmentFilter.LateReminder = Settings.Calendar.LateReminding;
                googleAppointmentFilter.Minutes = Settings.Calendar.RemindingInterval;
                googleAppointmentFilter.ReminderMethod = (int)Settings.Calendar.GoogleReminder;
                if (foundAppointment)
                {
                    try
                    {
                        googleAppointmentFilter.UpdateAppointment(0);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(ex.Message);
                    }
                }
                else
                {
                    try
                    {
                        googleAppointmentFilter.AddAppointment();
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(ex.Message);
                    }
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void SaveChangesButton_Click(object sender, EventArgs e)
        {
            // Save temporary settings for this appointment.
            if (OutlookRadioButton.Checked) Settings.Calendar.Provider = CalendarProvider.Outlook;
            else Settings.Calendar.Provider = CalendarProvider.Google;

            Settings.Calendar.GoogleEmail = GoogleEmailTextBox.Text;
            Settings.Calendar.GooglePassword = GooglePasswordTextBox.Text;
            Settings.Calendar.GoogleURL = GoogleUriTextBox.Text;
            Settings.Calendar.GoogleReminder = (GoogleCalendarReminder)GoogleReminderComboBox.SelectedIndex;
            Settings.Calendar.UseReminding = SetReminderCheckBox.Checked;
            Settings.Calendar.RemindingInterval = Int32.Parse(ReminderTextBox.Text);
            Settings.Calendar.UseRemindingRange = UseAlterateReminderCheckBox.Checked;
            Settings.Calendar.EarlyReminding = EarlyReminderDateTimePicker.Value;
            Settings.Calendar.LateReminding = LateReminderDateTimePicker.Value;
        }
    }
}
