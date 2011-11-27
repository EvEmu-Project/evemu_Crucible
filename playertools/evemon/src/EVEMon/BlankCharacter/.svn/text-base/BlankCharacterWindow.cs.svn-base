using System;
using System.IO;
using System.Text;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.BlankCharacter
{
    public partial class BlankCharacterWindow : EVEMonForm
    {
        private string m_filename;
        private bool m_fileSaved;

        /// <summary>
        /// Initializes a new instance of the <see cref="BlankCharacterWindow"/> class.
        /// </summary>
        public BlankCharacterWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Handles the Load event of the BlankCharacterWindow control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void BlankCharacterWindow_Load(object sender, EventArgs e)
        {
            EveClient.TimerTick += EveClient_TimerTick;
            Disposed += OnDisposed;

            buttonOK.Enabled = false;
        }
        
        
        #region Event Handlers

        /// <summary>
        /// Handles the TimerTick event of the EveClient control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void EveClient_TimerTick(object sender, EventArgs e)
        {
            buttonOK.Enabled = !String.IsNullOrEmpty(blankCharacterControl.tbCharacterName.Text);
            buttonOK.Text = (!buttonOK.Enabled || m_fileSaved ? "Import" : "Save");
        }

        /// <summary>
        /// Called when [disposed].
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.TimerTick -= EveClient_TimerTick;
            Disposed -= OnDisposed;
        }
        
        #endregion


        #region Control Handlers

        /// <summary>
        /// Handles the Click event of the buttonOK control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void buttonOK_Click(object sender, EventArgs e)
        {
            // Create the blank character from selections
            var serial = blankCharacterControl.CreateCharacter();

            // Two choices for one button
            // Save blank character ?
            if (!m_fileSaved)
            {
                Save(serial);
            }
            // Add blank character ?
            else
            {
                AddBlankCharacter();
            }

            buttonOK.DialogResult = (m_fileSaved ? DialogResult.OK : DialogResult.None);
        }

        /// <summary>
        /// Handles the Click event of the buttonCancel control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void buttonCancel_Click(object sender, EventArgs e)
        {
            Close();
        }

        #endregion


        #region Helper Methods

        /// <summary>
        /// Saves the blank character.
        /// </summary>
        /// <param name="serial">The serial.</param>
        private void Save(SerializableCCPCharacter serial)
        {
            using (SaveFileDialog fileDialog = new SaveFileDialog())
            {
                fileDialog.Title = "Save Blank Character";
                fileDialog.Filter = "Blank Character CCPXML (*.xml) | *.xml";
                fileDialog.FileName = String.Format(CultureConstants.DefaultCulture, "{0}.xml", serial.Name);
                fileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
                DialogResult saveFile = fileDialog.ShowDialog();

                if (saveFile == DialogResult.OK)
                {
                    // Disabling control edit ability
                    blankCharacterControl.Enabled = false;
                    
                    var xmlDoc = Util.SerializeToXmlDocument(serial.GetType(), serial);
                    string content = Util.GetXMLStringRepresentation(xmlDoc);
                    FileHelper.OverwriteOrWarnTheUser(fileDialog.FileName, fs =>
                    {
                        using (var writer = new StreamWriter(fs, Encoding.UTF8))
                        {
                            writer.Write(content);
                            writer.Flush();
                            writer.Close();
                        }
                        return true;
                    });

                    m_filename = fileDialog.FileName;
                    m_fileSaved = true;
                }
                else
                {
                    m_fileSaved = false;
                }
            }
        }

        /// <summary>
        /// Adds the blank character.
        /// </summary>
        private void AddBlankCharacter()
        {
            // Add blank character
            EveClient.Characters.TryAddOrUpdateFromUriAsync(new Uri(m_filename), (send, args) =>
            {
                if (args == null || args.HasError)
                    return;

                var character = args.CreateCharacter();
                character.Monitored = true;
            });
        }

        #endregion
    }
}
