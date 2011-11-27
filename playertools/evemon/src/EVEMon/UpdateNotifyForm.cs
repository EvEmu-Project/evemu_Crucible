using System;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;
using System.Text;

namespace EVEMon
{
    public partial class UpdateNotifyForm : EVEMonForm
    {
        private UpdateAvailableEventArgs m_args;

        /// <summary>
        /// Default constructor.
        /// </summary>
        public UpdateNotifyForm()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        public UpdateNotifyForm(UpdateAvailableEventArgs args)
            : this()
        {
            m_args = args;
        }

        /// <summary>
        /// Occurs on "ignore" button click.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnIgnore_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show(
                "Are you sure you want to ignore this update? You will not " +
                "be prompted again until a newer version is released.",
                "Ignore Update?",
                MessageBoxButtons.YesNo,
                MessageBoxIcon.Question,
                MessageBoxDefaultButton.Button2);
            if (dr == DialogResult.No)
            {
                return;
            }
            Settings.Updates.MostRecentDeniedUpgrade = m_args.NewestVersion.ToString();
            Settings.Save();
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        /// <summary>
        /// Occurs on "update" button click.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnUpdate_Click(object sender, EventArgs e)
        {
            if (cbAutoInstall.Enabled && cbAutoInstall.Checked)
            {
                Uri updateURI = new Uri(m_args.InstallerUrl);
                string localFilename = Path.Combine(EveClient.EVEMonDataDir, Path.GetFileName(updateURI.AbsolutePath));
                using (UpdateDownloadForm f = new UpdateDownloadForm(m_args.InstallerUrl, localFilename))
                {
                    f.ShowDialog();
                    if (f.DialogResult == DialogResult.OK)
                    {
                        ExecPatcher(localFilename, m_args.AutoInstallArguments);
                    }
                }
            }
            else
            {
                Process.Start(m_args.ForumUrl);
                this.DialogResult = DialogResult.OK;
                this.Close();
            }
        }

        /// <summary>
        /// Initiates the auto installer.
        /// </summary>
        /// <param name="fn"></param>
        /// <param name="args"></param>
        private void ExecPatcher(string fn, string args)
        {
            try
            {
                Process.Start(fn, args);
            }
            catch (Exception e)
            {
                ExceptionHandler.LogRethrowException(e);
                if (File.Exists(fn))
                {
                    try
                    {
                        File.Delete(fn);
                    }
                    catch
                    {
                        ExceptionHandler.LogException(e, false);
                    }
                }
                throw;
            }
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        /// <summary>
        /// Occurs on "remind me later" button click.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnLater_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        /// <summary>
        /// On form shown we subcribe the event handler.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UpdateNotifyForm_Shown(object sender, EventArgs e)
        {
            UpdateInformation();
            cbAutoInstall.Checked = m_args.CanAutoInstall;
            EveClient.UpdateAvailable += UpdateNotifyForm_UpdateAvailable;
        }

        /// <summary>
        /// When an update is available, we update the informations.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UpdateNotifyForm_UpdateAvailable(object sender, UpdateAvailableEventArgs e)
        {
            m_args = e;
            UpdateInformation();
        }

        /// <summary>
        /// Initilizes the contents and state of the controls.
        /// </summary>
        private void UpdateInformation()
        {
            // Set the basic update information
            StringBuilder labelText = new StringBuilder();
            labelText.AppendLine("An EVEMon update is available.");
            labelText.AppendLine();
            labelText.AppendFormat(CultureConstants.DefaultCulture, "Current version: {0}{1}", m_args.CurrentVersion, Environment.NewLine);
            labelText.AppendFormat(CultureConstants.DefaultCulture, "Newest version: {0}{1}", m_args.NewestVersion, Environment.NewLine);
            labelText.AppendLine("The newest version has the following updates:");
            label1.Text = labelText.ToString();

            // Set the detailed update information (from the XML)
            string updMessage = m_args.UpdateMessage;
            updMessage.Replace("\r", String.Empty);
            textBox1.Lines = updMessage.Split('\n');

            cbAutoInstall.Enabled = m_args.CanAutoInstall;
        }

        /// <summary>
        /// On form closed we unsuscribe the event handler.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UpdateNotifyForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            EveClient.UpdateAvailable -= UpdateNotifyForm_UpdateAvailable;
        }
    }
}