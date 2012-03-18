using System;
using System.Linq;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.Accounting
{
    public partial class CharacterDeletionWindow : EVEMonForm
    {
        private readonly Account m_account;
        private readonly Character m_character;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="character"></param>
        public CharacterDeletionWindow(Character character)
        {
            InitializeComponent();
            m_character = character;
            m_account = character.Identity.Account;

            // Replaces end of text with character's name
            string newText = label1.Text.Replace("a character", m_character.Name);
            label1.Text = newText;

            // Checks whether there will be no characters left after this deletion and hide/display the relevant labels.
            int charactersLeft = EveClient.Characters.Count(x => x.Identity.Account == m_account);
            bool noCharactersLeft = (m_account != null && m_character is CCPCharacter && charactersLeft == 1);
            noCharactersCheckBox.Visible = noCharactersLeft;
            noCharactersLabel.Visible = noCharactersLeft;
        }

        /// <summary>
        /// Delete button click.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void deleteButton_Click(object sender, EventArgs e)
        {
            // Either delete this character only or the whole account.
            if (noCharactersCheckBox.Checked)
            {
                EveClient.Characters.Remove(m_character);
                EveClient.Accounts.Remove(m_account);
            }
            else
            {
                EveClient.Characters.Remove(m_character);
            }

            Close();
        }
    }
}