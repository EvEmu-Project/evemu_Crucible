using System;
using System.IO;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Net;
using EVEMon.Common.Controls;

namespace EVEMon
{
    public partial class UpdateDownloadForm : EVEMonForm
    {
        public UpdateDownloadForm()
        {
            InitializeComponent();
        }

        public UpdateDownloadForm(string url, string filename)
            : this()
        {
            m_url = url;
            m_fileName = filename;
        }

        private readonly string m_url;
        private readonly string m_fileName;
        private object m_request = null;

        private void UpdateDownloadForm_Shown(object sender, EventArgs e)
        {
            try
            {
                m_request = EveClient.HttpWebService.DownloadFileAsync(m_url, m_fileName, DownloadCompletedCallback, ProgressChangedCallback);
            }
            catch (Exception ex)
            {
                ExceptionHandler.LogException(ex, true);
                DialogResult = DialogResult.Cancel;
                Close();
            }
        }

        private void ProgressChangedCallback(DownloadProgressChangedArgs e)
        {
            Invoke((MethodInvoker) delegate { ProgressChanged(e); });
        }

        private void ProgressChanged(DownloadProgressChangedArgs e)
        {
            if (e.TotalBytesToReceive > 0)
            {
                label1.Text = String.Format(CultureConstants.TidyInteger, "Downloading update ({0}%, {1:n} of {2:n} bytes received)...",
                                            e.ProgressPercentage, e.BytesReceived, e.TotalBytesToReceive);
                pbProgress.Style = ProgressBarStyle.Blocks;
                pbProgress.Minimum = 0;
                pbProgress.Maximum = 100;
                
                // Under Vista and Windows 7 there is a lag when progress bar updates too quick.
                // This hackish way though solves this issue (in a way) as explained in
                // http://stackoverflow.com/questions/977278/how-can-i-make-the-progress-bar-update-fast-enough/1214147#1214147.
                pbProgress.Value = e.ProgressPercentage;
                pbProgress.Value = (e.ProgressPercentage == 0 ? e.ProgressPercentage : e.ProgressPercentage - 1);
            }
            else
            {
                label1.Text = String.Format(CultureConstants.TidyInteger, "Downloading update ({0:n} bytes received)...",
                                            e.BytesReceived);
                if (pbProgress.Style != ProgressBarStyle.Marquee)
                {
                    pbProgress.Style = ProgressBarStyle.Marquee;
                }
            }
        }

        private void DownloadCompletedCallback(DownloadFileAsyncResult e)
        {
            Invoke((MethodInvoker) delegate { DownloadCompleted(e); });
        }

        private void DownloadCompleted(DownloadFileAsyncResult e)
        {
            if (e.Error != null)
            {
                ExceptionHandler.LogException(e.Error, true);
                DialogResult = DialogResult.Cancel;
            }
            else if (e.Cancelled)
            {
                DialogResult = DialogResult.Cancel;
                if (File.Exists(m_fileName))
                {
                    try
                    {
                        File.Delete(m_fileName);
                    }
                    catch (Exception ex)
                    {
                        ExceptionHandler.LogException(ex, false);
                    }
                }
            }
            else
            {
                DialogResult = DialogResult.OK;
            }
            Close();
        }

        private void btCancel_Click(object sender, EventArgs e)
        {
            if (m_request != null)
                EveClient.HttpWebService.CancelRequest(m_request);
        }
    }
}