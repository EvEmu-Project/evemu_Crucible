using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common.Controls;
using EVEMon.Common;
using EVEMon.Controls;

namespace EVEMon.Accounting
{
    /// <summary>
    /// A windows to import characters from files or urls.
    /// </summary>
    public partial class CharacterImportationWindow : EVEMonForm
    {
        // When multiple downloads are fired, we only want to react to the latest one, this is done through versioning.
        private int m_version;
        private UriCharacterEventArgs m_args;
        private UriCharacter m_character;

        /// <summary>
        /// Constructor for a new character creation.
        /// </summary>
        public CharacterImportationWindow()
        {
            InitializeComponent();
            errorPanel.Visible = false;
            namePanel.Visible = false;
            okButton.Enabled = false;
        }

        /// <summary>
        /// Constructor to update a character.
        /// </summary>
        public CharacterImportationWindow(UriCharacter character)
        {
            InitializeComponent();
            errorPanel.Visible = false;
            namePanel.Visible = false;
            okButton.Enabled = false;

            m_character = character;
            if (m_character.Uri.IsFile)
            {
                fileTextBox.Text = m_character.Uri.ToString();
                fileRadio.Checked = true;
            }
            else
            {
                urlTextBox.Text = m_character.Uri.ToString();
                urlRadio.Checked = true;
            }
        }

        /// <summary>
        /// Gets the generated character after the form closing, or null if the operation was cancel or couldn't success.
        /// </summary>
        public UriCharacter Character
        {
            get { return m_character; }
        }

        /// <summary>
        /// On load, subscribe events.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            urlTextBox.TextChanged += urlTextBox_TextChanged;
        }

        /// <summary>
        /// Enable or disable the proper controls when the user changes the radio buttons.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void radio_CheckedChanged(object sender, EventArgs e)
        {
            urlTextBox.Enabled = urlRadio.Checked;
            urlThrobber.Enabled = urlRadio.Checked;

            fileButton.Enabled = fileRadio.Checked;
            fileTextBox.Enabled = fileRadio.Checked;
        }

        /// <summary>
        /// When the users changes the url, we query from the web.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void urlTextBox_TextChanged(object sender, EventArgs e)
        {
            delayQueryTimer.Stop();
            delayQueryTimer.Start();
        }

        /// <summary>
        /// We would like to check the provided url anytime the url textbox changes but it would be stressful, so we only do it when the user didn't type 
        /// anything in the last 400ms. Once this timer expires, we do the check.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void delayQueryTimer_Tick(object sender, EventArgs e)
        {
            TryUri(urlTextBox.Text);
        }

        /// <summary>
        /// When the users clicks the "..." button, we open a file selection dialog, then try to read the specified XML.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void fileButton_Click(object sender, EventArgs e)
        {
            using (var form = new OpenFileDialog())
            {
                form.Title = "Import character file";
                form.Filter = "CCP XML Character (*.xml)|*.xml";
                form.FilterIndex = 0;

                DialogResult dr = form.ShowDialog();
                if (dr == DialogResult.Cancel)
                    return;

                fileTextBox.Text = form.FileName;
                fileRadio.Checked = true;
                TryUri(form.FileName);
            }
        }

        /// <summary>
        /// Try to download a character XML from the given URI.
        /// </summary>
        /// <param name="uri"></param>
        private void TryUri(string uri)
        {
            m_version++;
            m_args = null;

            // Update controls
            okButton.Enabled = false;
            namePanel.Visible = false;
            errorPanel.Visible = false;
            urlThrobber.State = ThrobberState.Rotating;

            // Starts querying the web or the hard drive, and invokes the given callback on result.
            int version = m_version;
            EveClient.Characters.TryAddOrUpdateFromUriAsync(new Uri(uri), (sender, args) =>
                {
                    if (version != m_version)
                        return;

                    urlThrobber.State = ThrobberState.Stopped;

                    // Was there an error ?
                    if (args.HasError)
                    {
                        okButton.Enabled = false;
                        errorPanel.Visible = true;
                        labelError.Text = args.Error;
                        return;
                    }

                    nameTextBox.Text = args.CharacterName;
                    namePanel.Visible = true;
                    okButton.Enabled = true;
                    m_args = args;
                });
        }

        /// <summary>
        /// When the user clicks the OK button, we may have to generate the character
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void okButton_Click(object sender, EventArgs e)
        {
            // Shouldn't occur but, well...
            if (m_args == null || m_args.HasError)
            {
                DialogResult = DialogResult.Cancel;
                return;
            }

            // Generates or updates the character
            if (m_character == null)
            {
                m_character = m_args.CreateCharacter();
            }
            else
            {
                m_args.UpdateCharacter(m_character);
            }
            
            m_character.Name = nameTextBox.Text;

            DialogResult = DialogResult.OK;
        }
    }
}
