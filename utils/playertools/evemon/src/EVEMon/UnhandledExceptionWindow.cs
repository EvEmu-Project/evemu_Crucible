using System;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Controls;

using CommonProperties = EVEMon.Common.Properties;

namespace EVEMon
{
    /// <summary>
    /// Form to handle the display of the error report for easy bug reporting
    /// </summary>
    public partial class UnhandledExceptionWindow : EVEMonForm
    {
        public UnhandledExceptionWindow()
        {
            InitializeComponent();
            this.WhatCanYouDoLabel.Font = FontFactory.GetFont("Tahoma", 10F);
        }

        public UnhandledExceptionWindow(Exception err)
            : this()
        {
            m_exception = err;
        }

        private Exception m_exception;

        /// <summary>
        /// Loads resources, generates the report
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UnhandledExceptionWindow_Load(object sender, EventArgs e)
        {
            try
            {
                var i = CommonProperties.Resources.Bug;

                int oHeight = i.Height;
                int oWidth = i.Width;
                if (i.Height > BugPictureBox.ClientSize.Height)
                {
                    double scale = (double)(BugPictureBox.ClientSize.Height) / (double)(i.Height);
                    oHeight = (int)(oHeight * scale);
                    oWidth = (int)(oWidth * scale);
                    Bitmap b = new Bitmap(i, new Size(oWidth, oHeight));

                    int oRight = BugPictureBox.Right;
                    BugPictureBox.ClientSize = new Size(oWidth, oHeight);
                    BugPictureBox.Image = b;
                }
            }
            catch (Exception ex)
            {
                ExceptionHandler.LogException(ex, false);
            }

            string trace;
            EveClient.StopTraceLogging();
            
            try
            {
                using (FileStream traceStream = new FileStream(EveClient.TraceFileNameFullPath, FileMode.Open, FileAccess.Read))
                {
                    using (StreamReader traceReader = new StreamReader(traceStream))
                    {
                        trace = traceReader.ReadToEnd();
                    }
                }
            }
            catch (Exception ex)
            {
                ExceptionHandler.LogException(ex, false);
                trace = "Unable to load trace file for inclusion in report";
            }

            try
            {
                StringBuilder exceptionReport = new StringBuilder();
                OperatingSystem os = Environment.OSVersion;

                exceptionReport.AppendFormat(CultureConstants.DefaultCulture, "EVEMon Version: {0}{1}", Application.ProductVersion, Environment.NewLine);
                exceptionReport.AppendFormat(CultureConstants.DefaultCulture, ".NET Runtime Version: {0}{1}", Environment.Version, Environment.NewLine);
                exceptionReport.AppendFormat(CultureConstants.DefaultCulture, "Operating System: {0}{1}", os.VersionString, Environment.NewLine);
                exceptionReport.AppendFormat(CultureConstants.DefaultCulture, "Executable Path: {0}{1}", Environment.CommandLine, Environment.NewLine);
                exceptionReport.AppendLine();
                exceptionReport.Append(RecursiveStackTrace).AppendLine();
                exceptionReport.AppendLine();
                exceptionReport.Append(DatafileReport).AppendLine();
                exceptionReport.AppendLine();
                exceptionReport.Append("Diagnostic Log:").AppendLine();
                exceptionReport.Append(trace.Trim()).AppendLine();

                TechnicalDetailsTextBox.Text = exceptionReport.ToString();
            }
            catch (Exception ex)
            {
                ExceptionHandler.LogException(ex, true);
                TechnicalDetailsTextBox.Text = "Error retrieving error data. Wow, things are *really* screwed up!";
            }
        }

        private string DatafileReport
        {
            get
            {
                StringBuilder datafileReport = new StringBuilder();

                try
                {
                    string[] datafiles = Directory.GetFiles(EveClient.EVEMonDataDir, "*.gz", SearchOption.TopDirectoryOnly);

                    datafileReport.AppendLine("Datafile report:");

                    foreach (string datafile in datafiles)
                    {
                        FileInfo info = new FileInfo(datafile);
                        Datafile file = new Datafile(Path.GetFileName(datafile));

                        datafileReport.AppendFormat(CultureConstants.DefaultCulture, "  {0} ({1}KiB - {2}){3}", info.Name, info.Length / 1024, file.MD5Sum, Environment.NewLine);
                    }
                }
                catch
                {
                    datafileReport.Append("Unable to create datafile report").AppendLine();
                }
                return datafileReport.ToString();
            }
        }

        private string RecursiveStackTrace
        {
            get
            {
                StringBuilder stackTraceBuilder = new StringBuilder();
                Exception ex = m_exception;

                stackTraceBuilder.Append(ex.ToString()).AppendLine();

                while (ex.InnerException != null)
                {
                    ex = ex.InnerException;

                    stackTraceBuilder.AppendLine();
                    stackTraceBuilder.Append(ex.ToString()).AppendLine();
                }

                stackTraceBuilder = stackTraceBuilder.Replace(@"D:\EVEMon\", String.Empty); // Richard Slater's local installer builder path
                stackTraceBuilder = stackTraceBuilder.Replace(@"G:\EVEMon Project\EVEMon\", String.Empty); // Jimi's local installer builder path
                stackTraceBuilder = stackTraceBuilder.Replace(@"d:\tmp\evemon_installer\", String.Empty); // TeamCity's installer builder path
                stackTraceBuilder = stackTraceBuilder.Replace(@"d:\tmp\evemon\", String.Empty); // TeamCity's snapshot builder path
                return stackTraceBuilder.ToString();
            }
        }

        private void CloseButton_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void CopyDetailsLinkLabel_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            try
            {
                Clipboard.SetText(TechnicalDetailsTextBox.Text, TextDataFormat.Text);
                MessageBox.Show("The error details have been copied to the clipboard.", "Copy", MessageBoxButtons.OK,
                                MessageBoxIcon.Information);
            }
            catch (ExternalException ex)
            {
                // Occurs when another process is using the clipboard
                ExceptionHandler.LogException(ex, true);
                MessageBox.Show("Couldn't complete the operation, the clipboard is being used by another process. Wait a few moments and try again.");
            }
        }

        private void ResetButtonLinkLabel_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show(
                "This will clear all your saved settings, including skill plans and " +
                "character logins. You should only try this if EVEMon has errored more " +
                "than once.\r\n\r\nClear settings?",
                "Clear Settings?",
                MessageBoxButtons.YesNo,
                MessageBoxIcon.Warning,
                MessageBoxDefaultButton.Button2);

            if (dr == DialogResult.Yes)
            {
                Settings.Reset();
                MessageBox.Show("Your settings have been reset.",
                                "Settings Reset", MessageBoxButtons.OK, MessageBoxIcon.Information);
                this.DialogResult = DialogResult.OK;
                this.Close();
            }
        }

        private void llblReport_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Util.OpenURL(NetworkConstants.EVEMonBugReport);
        }

        private void llblKnownProblems_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Util.OpenURL(NetworkConstants.EVEMonKnownProblems);
        }

        private void llblLatestBinaries_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Util.OpenURL(NetworkConstants.EVEMonMainPage);
        }
    }
}