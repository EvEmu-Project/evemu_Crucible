using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using EVEMon.Common;

namespace EVEMon.Common.Controls
{
    public partial class EveFolderWindow : EVEMonForm
    {
        private readonly string[] m_defaultFolderLocation = EveClient.DefaultEvePortraitCacheFolders;
        private string[] m_specifiedPortraitFolder = new string[] { String.Empty };

        public EveFolderWindow()
        {
            InitializeComponent();
        }

        private void EVEFolderWindow_Load(object sender, EventArgs e)
        {
            if (m_defaultFolderLocation == null)
            {
                SpecifyFolderRadioButton.Checked = true;
                DefaultFolderRadioButton.Enabled = false;
            }
            
            if (m_specifiedPortraitFolder == m_defaultFolderLocation)
            {
                DefaultFolderRadioButton.Checked = true;
                BrowseButton.Enabled = false;
            }
            else
            {
                SpecifyFolderRadioButton.Checked = true;
                BrowseButton.Enabled = true;
            }
        }

        public string[] EVEPortraitCacheFolder
        {
            get { return m_specifiedPortraitFolder; }
        }

        private void BrowseButton_Click(object sender, EventArgs e)
        {
            DialogResult dr = OpenDirFolderBrowserDialog.ShowDialog();
            if (dr == DialogResult.OK)
            {
                FilenameTextBox.Text = OpenDirFolderBrowserDialog.SelectedPath;
                m_specifiedPortraitFolder[0] = FilenameTextBox.Text;
                OKButton.Enabled = true;
            }
        }

        private void OKButton_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            Close();
        }

        private void DefaultFolderRadioButton_CheckedChanged(object sender, EventArgs e)
        {
            if (DefaultFolderRadioButton.Checked)
            {
                m_specifiedPortraitFolder = m_defaultFolderLocation;
                OKButton.Enabled = true;
            }
        }

        private void SpecifyFolderRadioButton_CheckedChanged(object sender, EventArgs e)
        {
            BrowseButton.Enabled = SpecifyFolderRadioButton.Checked;
            OKButton.Enabled = (SpecifyFolderRadioButton.Checked && FilenameTextBox.Text != String.Empty);
        }
    }
}
