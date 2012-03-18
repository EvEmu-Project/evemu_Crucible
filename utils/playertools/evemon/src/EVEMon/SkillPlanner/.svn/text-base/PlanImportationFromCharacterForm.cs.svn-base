using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Linq;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;


namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// This window allow the exportation of plans between characters.
    /// </summary>
    public partial class PlanImportationFromCharacterForm : EVEMonForm
    {
        private Character m_targetCharacter;
        private Character m_selectedChar;
        private Plan m_selectedPlan;
        private Plan m_targetPlan;


        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="toCharacter"></param>
        public PlanImportationFromCharacterForm(Character targetCharacter)
        {
            InitializeComponent();
            m_targetCharacter = targetCharacter;
        }

        /// <summary>
        /// Gets the selected source plan.
        /// </summary>
        public Plan SourcePlan
        {
            get { return m_selectedPlan; }
        }

        /// <summary>
        /// Gets the selected source character.
        /// </summary>
        public Character SourceCharacter
        {
            get { return m_selectedChar; }
        }

        /// <summary>
        /// Gets the target character.
        /// </summary>
        public Character TargetCharacter
        {
            get { return m_targetCharacter; }
        }

        /// <summary>
        /// Gets the exported plan. 
        /// </summary>
        /// <remarks>This plan has not been added to the character's list and still has the same name than the source plan.</remarks>
        public Plan TargetPlan
        {
            get { return m_targetPlan; }
        }

        /// <summary>
        /// Populate the plans list from the given character
        /// </summary>
        /// <param name="characterName"></param>
        private void PopulatePlans(Character character)
        {
            btnLoad.Enabled = false;
            lbPlan.Items.Clear();
            foreach (var plan in character.Plans)
            {
                lbPlan.Items.Add(plan);
            }
        }
        #region Event handlers
        /// <summary>
        /// Populate the character list with all characters except the target
        /// </summary>
        private void CrossPlanSelect_Load(object sender, EventArgs e)
        {
            cbCharacter.Items.Clear();
            foreach (var character in EveClient.Characters.Where(x => x.CharacterID != m_targetCharacter.CharacterID))
            {
                cbCharacter.Items.Add(character);
            }
            cbCharacter.SelectedIndex = 0;
            PopulatePlans(cbCharacter.SelectedItem as Character);
        }

        /// <summary>
        /// When the selected character changed, we update the plans list.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbCharacter_SelectedIndexChanged(object sender, EventArgs e)
        {
            PopulatePlans(cbCharacter.Items[cbCharacter.SelectedIndex] as Character);
        }

        /// <summary>
        /// When the selected plan changed, we enable/disable the "load" button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lbPlan_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnLoad.Enabled = (lbPlan.SelectedItems.Count == 1);
        }

        /// <summary>
        /// When the user clicks "load", import the plan.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnLoad_Click(object sender, EventArgs e)
        {
            m_selectedPlan = lbPlan.SelectedItem as Plan;
            m_selectedChar = cbCharacter.SelectedItem as Character;
            m_targetPlan = m_selectedPlan.Clone(m_targetCharacter);
            DialogResult = DialogResult.OK;
            this.Close();
        }
        #endregion
    }
}