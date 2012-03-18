using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Net;
using System.Net.Mail;
using System.Text;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
	/// <summary>
	/// Provides SMTP based e-mail services taylored to Skill Completion.
	/// </summary>
    public static class Emailer
	{
        /// <summary>
        /// Sends a test mail
        /// </summary>
        /// <param name="settings">NotificationSettings object</param>
        /// <remarks>
        /// A notification settings object is required, as this function
        /// is called from the Settings Window, and assumibly the user
        /// is changing settings.
        /// </remarks>
        /// <returns>False if an exception was thrown, otherwise True.</returns>
		public static bool SendTestMail(NotificationSettings settings)
		{
            return SendMail(settings, "EVEMon Test Mail", "This is a test email sent by EVEMon");
		}

        /// <summary>
        /// Sends a mail alert for a skill completion
        /// </summary>
        /// <param name="queueList">Current Skill Queue</param>
        /// <param name="skill">Skill that has just completed</param>
        /// <param name="character">Character affected</param>
        /// <returns></returns>
		public static bool SendSkillCompletionMail(IList<QueuedSkill> queueList, QueuedSkill skill, Character character)
		{
            CCPCharacter ccpCharacter = character as CCPCharacter;

            // Current character isn't a CCP character, so can't have a Queue.
            if (ccpCharacter == null)
                return false;
            
            string charName = character.Name;
            string skillName = skill.SkillName;
            string skillLevelString = Skill.GetRomanForInt(skill.Level);

            var skillQueueEndTime = ccpCharacter.SkillQueue.EndTime;
            bool freeTime = skillQueueEndTime < DateTime.UtcNow.AddHours(24);
            TimeSpan timeLeft = DateTime.UtcNow.AddHours(24).Subtract(skillQueueEndTime);
            string timeLeftText = timeLeft.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas, false);

            // Message's first line
            StringBuilder body = new StringBuilder();
            body.AppendFormat(CultureConstants.DefaultCulture, "{0} has finished training {1} {2}.{3}{3}", charName, skillName, skillLevelString, Environment.NewLine);
            
            // Next skills in queue
            if (queueList[0] != null)
            {
                body.AppendFormat(CultureConstants.DefaultCulture, "Next skill{0} in queue:{1}", (queueList.Count > 1 ? "s" : String.Empty), Environment.NewLine);
                foreach (var qskill in queueList)
                {
                    body.AppendFormat(CultureConstants.DefaultCulture, "- {0}{1}", qskill, Environment.NewLine);
                }
                body.AppendLine();
            }
            else
            {
                body.AppendFormat(CultureConstants.DefaultCulture, "Character is not training.{0}{0}", Environment.NewLine);
            }

            // Free room in skill queue
            if (freeTime)
                body.AppendFormat(CultureConstants.DefaultCulture, "There is also {0} free room in skill queue.{1}", timeLeftText, Environment.NewLine);

            // Short format (also for SMS)
            if (Settings.Notifications.UseEmailShortFormat)
            {
                return SendMail(Settings.Notifications, String.Format(CultureConstants.DefaultCulture, "[STC] {0} :: {1} {2}", charName, skillName, skillLevelString), body.ToString());
            }

            // Long format
            if (character.Plans.Count > 0) body.AppendFormat(CultureConstants.DefaultCulture, "Next skills listed in plans:{0}{0}", Environment.NewLine);

            foreach (var plan in character.Plans)
			{
				if (plan.Count > 0)
				{
                    // Print plan name
                    CharacterScratchpad scratchpad = new CharacterScratchpad(character);
                    body.AppendFormat(CultureConstants.DefaultCulture, "{0}:{1}", plan.Name, Environment.NewLine);

                    // Scroll through entries
					int i = 0;
					int minDays = 1;
					foreach (PlanEntry entry in plan)
					{
						TimeSpan trainTime = scratchpad.GetTrainingTime(entry.Skill, entry.Level, TrainingOrigin.FromPreviousLevelOrCurrent);

						// Only print the first three skills, and the very long skills
                        // (first limit is one day, then we add skills duration)
						if (++i <= 3 || trainTime.Days > minDays)
						{
							if (i > 3)
							{
								// Print long message once
								if (minDays == 1)
								{
                                    body.AppendFormat(CultureConstants.DefaultCulture, "{1}Longer skills from {0}:{1}", plan.Name, Environment.NewLine);
								}

								minDays = trainTime.Days + minDays;
							}
                            body.AppendFormat(CultureConstants.DefaultCulture, "\t{0}", entry);

                            // Notes
							if (entry.Notes != null && entry.Notes.Length > 0)
							{
                                body.AppendFormat(CultureConstants.DefaultCulture, " ({0})", entry.Notes);
							}

                            // Training time
							if (trainTime.Days > 0)
							{
                                body.AppendFormat(CultureConstants.DefaultCulture, " - {0}d, {1}", trainTime.Days, trainTime);
							}
							else
							{
                                body.AppendFormat(CultureConstants.DefaultCulture, " - {0}", trainTime);
							}

							body.AppendLine();
						}
					}
					body.AppendLine();
				}
			}

            string subject = String.Format(CultureConstants.DefaultCulture, "{0} has finished training {1} {2}", charName, skillName, skillLevelString);
			return SendMail(Settings.Notifications, subject, body.ToString());
		}

        /// <summary>
        /// Triggers on when a SMTP client has finished (success or failure)
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private static void SendCompleted(object sender, AsyncCompletedEventArgs e)
        {
            if (e.Cancelled)
            {
                EveClient.Trace("Emailer.SendCompleted - The last message was cancelled");
            }
            if (e.Error != null)
            {
                EveClient.Trace("Emailer.SendCompleted - An error occurred");
                ExceptionHandler.LogException(e.Error, false);
            }
            else
            {
                EveClient.Trace("Emailer.SendCompleted - Message sent.");
            }
        }

        /// <summary>
        /// Performs the sending of the mail
        /// </summary>
        /// <param name="settings">Settings object to use when sending</param>
        /// <param name="subject">Subject of the message</param>
        /// <param name="body">Body of the message</param>
        /// <returns>True if no exceptions thrown, otherwise false</returns>
        /// <remarks>
        /// NotificationSettings object is required to support
        /// alternative settings from Tols -> Options. Use
        /// Settings.Notifications unless using an alternative
        /// configuration.
        /// </remarks>
		private static bool SendMail(NotificationSettings settings, string subject, string body)
		{
            // trace something to the logs so we can identify the time the message was sent.
            EveClient.Trace("Emailer.SendMail: Subject - {0}; Server - {1}:{2}",
                subject,
                settings.EmailSmtpServer,
                settings.EmailPortNumber
                );

			try
			{
                // Set up message
                MailMessage msg = new MailMessage(settings.EmailFromAddress, settings.EmailToAddress, subject, body);

                // Set up client
                SmtpClient client = new SmtpClient(settings.EmailSmtpServer);
                client.SendCompleted += new SendCompletedEventHandler(SendCompleted); 
                if (settings.EmailPortNumber > 0)
				{
                    client.Port = settings.EmailPortNumber;
				}

                // Enter crendtials
                if (settings.EmailAuthenticationRequired)
				{
                    client.UseDefaultCredentials = false;
                    client.Credentials = new NetworkCredential(
                        settings.EmailAuthenticationUserName,
                        settings.EmailAuthenticationPassword);
				}

                // SSL
                client.EnableSsl = settings.EmailServerRequiresSSL;
                
                // Send message
				client.SendAsync(msg, null);
				return true;
			}
			catch (Exception e)
			{
				ExceptionHandler.LogException(e, true);
				return false;
			}
		}
	}
}
