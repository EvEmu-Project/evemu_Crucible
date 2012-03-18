using System;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Collections.Generic;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.Serialization.BattleClinic;

namespace EVEMon
{
    public partial class DataUpdateNotifyForm : EVEMonForm
    {
        private DataUpdateAvailableEventArgs m_args;

        /// <summary>
        /// Default constructor.
        /// </summary>
        public DataUpdateNotifyForm()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        public DataUpdateNotifyForm(DataUpdateAvailableEventArgs args)
            : this()
        {
            m_args = args;
        }

        /// <summary>
        /// On load we update the informations.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void DataUpdateNotifyForm_Load(object sender, EventArgs e)
        {
            StringBuilder changedFiles = new StringBuilder();
            StringBuilder notes = new StringBuilder("UPDATE NOTES:\n");
            foreach (SerializableDatafile dfv in m_args.ChangedFiles)
            {
                changedFiles.AppendFormat(CultureConstants.DefaultCulture,
                                        "Filename: {0}\t\tDated: {1}{3}Url: {2}/{0}{3}{3}",
                                        dfv.Name, dfv.Date, dfv.Url, Environment.NewLine);
                notes.AppendLine(dfv.Message);
            }
            tbFiles.Lines = changedFiles.ToString().Split('\n');
            tbNotes.Lines = notes.ToString().Split('\n');
        }

        /// <summary>
        /// Occurs on "update" button click.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnUpdate_Click(object sender, EventArgs e)
        {
            DialogResult result = DialogResult.Yes;
            int changedFilesCount = m_args.ChangedFiles.Count;

            while (m_args.ChangedFiles.Count != 0 && result == DialogResult.Yes)
            {
                DownloadUpdates();

                if (m_args.ChangedFiles.Count == 0)
                    break;

                // one or more files failed
                string message = String.Format(
                    CultureConstants.DefaultCulture, 
                    "{0} file{1} failed to download, do you wish to try again?", 
                    m_args.ChangedFiles.Count, m_args.ChangedFiles.Count == 1 ? String.Empty : "s");

                result = MessageBox.Show(message, "Failed Download", MessageBoxButtons.YesNo);
            }

            // if no files were updated, abort the update process.
            if (m_args.ChangedFiles.Count == changedFilesCount)
            {
                DialogResult = DialogResult.Abort;
            }
            else
            {
                DialogResult = DialogResult.OK;
            }

            Close();
        }

        /// <summary>
        /// Downloads the updates.
        /// </summary>
        private void DownloadUpdates()
        {
            List<SerializableDatafile> datafiles = new List<SerializableDatafile>();

            // Copy the list of datafiles
            m_args.ChangedFiles.ForEach(x => datafiles.Add(x));

            foreach (var dfv in datafiles)
            {
                // Work out the new names of the files
                string urn = String.Format(CultureConstants.DefaultCulture, "{0}/{1}", dfv.Url, dfv.Name);
                string oldFilename = Path.Combine(EveClient.EVEMonDataDir, dfv.Name);
                string newFilename = String.Format(CultureConstants.DefaultCulture, "{0}.tmp", oldFilename);

                // If the file already exists delete it
                if (File.Exists(newFilename))
                    File.Delete(newFilename);

                // Show the download dialog, which will download the file
                using (UpdateDownloadForm f = new UpdateDownloadForm(urn, newFilename))
                {
                    if (f.ShowDialog() == DialogResult.OK)
                    {
                        string filename = Path.GetFileName(newFilename);
                        Datafile datafile = new Datafile(filename);

                        if (datafile.MD5Sum != dfv.MD5Sum)
                        {
                            File.Delete(newFilename);
                            continue;
                        }

                        ReplaceDatafile(oldFilename, newFilename);
                        m_args.ChangedFiles.Remove(dfv);
                    }
                }
            }
        }

        /// <summary>
        /// Replaces the datafile.
        /// </summary>
        /// <param name="oldFilename">The old filename.</param>
        /// <param name="newFilename">The new filename.</param>
        private static void ReplaceDatafile(string oldFilename, string newFilename)
        {
            try
            {
                File.Delete(String.Format("{0}.bak", oldFilename));
                File.Copy(oldFilename, String.Format("{0}.bak", oldFilename));
                File.Delete(oldFilename);
                File.Move(newFilename, oldFilename);
            }
            catch (IOException ex)
            {
                ExceptionHandler.LogException(ex, true);
            }
        }

        /// <summary>
        /// Occurs on "remind me later" button click.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnLater_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }
    }
}