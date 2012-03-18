using System;
using System.Drawing;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// This controls allows the user to see how its plan's comutation times would change wth different implants.
    /// </summary>
    public partial class ImplantCalculator : EVEMonForm, IPlanOrderPluggable
    {
        private bool m_isUpdating = false;
        private Character m_character;
        private BaseCharacter m_characterScratchpad;
        private PlanEditorControl m_planEditor;
        private Plan m_plan;
        private ImplantSet m_set;

        /// <summary>
        /// Default constructor for designer.
        /// </summary>
        public ImplantCalculator()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Constructor for the given plan.
        /// </summary>
        /// <param name="plan"></param>
        public ImplantCalculator(Plan plan)
            : this()
        {
            m_plan = plan;
        }

        /// <summary>
        /// Sets the owner control
        /// </summary>
        public PlanEditorControl PlanEditor
        {
            set { m_planEditor = value; }
        }

        /// <summary>
        /// On load, update the controls states.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            m_character = (Character)m_plan.Character;
            m_characterScratchpad = m_plan.Character.After(m_plan.ChosenImplantSet);
            m_set = m_plan.ChosenImplantSet;

            // Set the min and max values of the NumericUpDown controls
            // based on character attributes value
            foreach (var control in AtrributesPanel.Controls)
            {
                NumericUpDown nud = control as NumericUpDown;
                 
                if (nud == null)
                    continue;

                var attrib = (EveAttribute)(int.Parse((string)nud.Tag));
                nud.Minimum = m_character[attrib].EffectiveValue - m_character[attrib].ImplantBonus;
                nud.Maximum = (m_plan.ChosenImplantSet[attrib].Bonus > EveConstants.MaxImplantPoints ?
                                nud.Minimum + m_plan.ChosenImplantSet[attrib].Bonus :
                                nud.Minimum + EveConstants.MaxImplantPoints);
            }

            UpdateContent();
            base.OnLoad(e);
        }

        /// <summary>
        /// Update all the numeric boxes
        /// </summary>
        private void UpdateContent()
        {
            gbAttributes.Text = String.Format(CultureConstants.DefaultCulture, "Attributes of \"{0}\"", m_set.Name);

            m_isUpdating = true;

            nudCharisma.Value = m_characterScratchpad.Charisma.EffectiveValue;
            nudWillpower.Value = m_characterScratchpad.Willpower.EffectiveValue;
            nudIntelligence.Value = m_characterScratchpad.Intelligence.EffectiveValue;
            nudPerception.Value = m_characterScratchpad.Perception.EffectiveValue;
            nudMemory.Value = m_characterScratchpad.Memory.EffectiveValue;
            m_isUpdating = false;

            // If the implant set isn't the active one we notify the user
            this.lblNotice.Visible = (m_set != m_character.ImplantSets.Current);

            //  Update all the times on the right pane
            UpdateTimes();
        }

        /// <summary>
        /// Updates the labels on the right of the numeric box.
        /// </summary>
        /// <param name="attrib"></param>
        /// <param name="myValue"></param>
        /// <param name="lblAdjust"></param>
        /// <param name="lblEffective"></param>
        private void UpdateAttributeLabels(EveAttribute attrib, int myValue, Label lblAdjust, Label lblEffective)
        {
            int baseAttr = m_characterScratchpad[attrib].EffectiveValue - m_characterScratchpad[attrib].ImplantBonus;
            int adjust = myValue - baseAttr;

            if (adjust >= 0)
            {
                lblAdjust.ForeColor = SystemColors.ControlText;
                lblAdjust.Text = "+" + adjust.ToString();
            }
            else
            {
                lblAdjust.ForeColor = Color.Red;
                lblAdjust.Text = adjust.ToString();
            }

            lblEffective.Text = m_characterScratchpad[attrib].EffectiveValue.ToString("0");
        }
        
        /// <summary>
        /// Update all the times on the right pane (base time, best time, etc).
        /// </summary>
        private void UpdateTimes()
        {
            if (m_isUpdating)
                return;
            

            if (m_planEditor != null)
            {
                m_characterScratchpad = m_character.After(m_set);
                m_planEditor.ShowWithPluggable(this);
            }

            // Current (with implants)
            TimeSpan currentSpan = UpdateTimesForCharacter(m_character.After(m_plan.ChosenImplantSet), lblCurrentSpan, lblCurrentDate);

            // Current (without implants)
            var noneImplantSet = m_character.ImplantSets.None;
            TimeSpan baseSpan = UpdateTimesForCharacter(m_character.After(noneImplantSet), lblBaseSpan, lblBaseDate);

            // This
            var scratchpad = CreateModifiedScratchpad();
            TimeSpan thisSpan = UpdateTimesForCharacter(scratchpad, lblThisSpan, lblThisDate);

            // Are the new attributes better than current (without implants) ?
            if (thisSpan > baseSpan)
            {
                lblComparedToBase.ForeColor = Color.Red;
                lblComparedToBase.Text = String.Format("{0} slower than current base",
                    thisSpan.Subtract(baseSpan).ToDescriptiveText(DescriptiveTextOptions.IncludeCommas));
            }
            else if (thisSpan < baseSpan)
            {
                lblComparedToBase.ForeColor = Color.Green;
                lblComparedToBase.Text = String.Format("{0} faster than current base",
                    baseSpan.Subtract(thisSpan).ToDescriptiveText(DescriptiveTextOptions.IncludeCommas));
            }
            else
            {
                lblComparedToBase.ForeColor = SystemColors.ControlText;
                lblComparedToBase.Text = "No time difference than current base";
            }

            // Are the new attributes better than current (with implants) ?
            if (thisSpan > currentSpan)
            {
                lblComparedToCurrent.ForeColor = Color.DarkRed;
                lblComparedToCurrent.Text = String.Format("{0} slower than current",
                    thisSpan.Subtract(currentSpan).ToDescriptiveText(DescriptiveTextOptions.IncludeCommas));
            }
            else if (thisSpan < currentSpan)
            {
                lblComparedToCurrent.ForeColor = Color.DarkGreen;
                lblComparedToCurrent.Text = String.Format("{0} faster than current",
                    currentSpan.Subtract(thisSpan).ToDescriptiveText(DescriptiveTextOptions.IncludeCommas));
            }
            else
            { 
                lblComparedToCurrent.ForeColor = SystemColors.ControlText;
                lblComparedToCurrent.Text = "No time difference than current";
            }
        }

        /// <summary>
        /// Update the times labels from the given character
        /// </summary>
        /// <param name="character"></param>
        /// <param name="lblSpan"></param>
        /// <param name="lblDate"></param>
        /// <returns></returns>
        private TimeSpan UpdateTimesForCharacter(BaseCharacter character, Label lblSpan, Label lblDate)
        {
            
            TimeSpan ts = character.GetTrainingTimeToMultipleSkills(m_plan);
            DateTime dt = DateTime.Now + ts;

            lblSpan.Text = ts.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);
            lblDate.Text = dt.ToString();

            return ts;
        }

        #region Controls events
        /// <summary>
        /// When the intelligence numeric box changed, we update the attributes labels and the times on the right pane.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nudIntelligence_ValueChanged(object sender, EventArgs e)
        {
            UpdateAttributeLabels(EveAttribute.Intelligence, (int)(nudIntelligence.Value),
                            lblAdjustIntelligence, lblEffectiveIntelligence);
            UpdateTimes();
        }

        /// <summary>
        /// When the charisma numeric box changed, we update the attributes labels and the times on the right pane.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nudCharisma_ValueChanged(object sender, EventArgs e)
        {
            UpdateAttributeLabels(EveAttribute.Charisma, (int)(nudCharisma.Value),
                            lblAdjustCharisma, lblEffectiveCharisma);
            UpdateTimes();
        }

        /// <summary>
        /// When the perception numeric box changed, we update the attributes labels and the times on the right pane.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nudPerception_ValueChanged(object sender, EventArgs e)
        {
            UpdateAttributeLabels(EveAttribute.Perception, (int)(nudPerception.Value),
                            lblAdjustPerception, lblEffectivePerception);
            UpdateTimes();
            if (m_planEditor != null)
                m_planEditor.ShowWithPluggable(this);
        }

        /// <summary>
        /// When the memory numeric box changed, we update the attributes labels and the times on the right pane.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nudMemory_ValueChanged(object sender, EventArgs e)
        {
            UpdateAttributeLabels(EveAttribute.Memory, (int)(nudMemory.Value),
                            lblAdjustMemory, lblEffectiveMemory);
            UpdateTimes();
        }

        /// <summary>
        /// When the willpower numeric box changed, we update the attributes labels and the times on the right pane.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nudWillpower_ValueChanged(object sender, EventArgs e)
        {
            UpdateAttributeLabels(EveAttribute.Willpower, (int)(nudWillpower.Value),
                            lblAdjustWillpower, lblEffectiveWillpower);
            UpdateTimes();
        }

        /// <summary>
        /// When the "load attributes" menu is opening, we add the items.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mnLoadAtts_DropDownOpening(object sender, EventArgs e)
        {
            // Add the menus for the sets
            mnLoadAtts.DropDownItems.Clear();
            foreach (var set in m_character.ImplantSets)
            {
                var item = mnLoadAtts.DropDownItems.Add(set.Name);
                item.Click += new EventHandler(implantSetMenuitem_Click);
                item.Tag = set;
            }
        }

        /// <summary>
        /// When an implant set menu item is clicked, we update the member for the current character.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void implantSetMenuitem_Click(object sender, EventArgs e)
        {
            // Update the character to a scratchpad using the implant set attacthed to the sender menu as its tag.
            var menu = (ToolStripItem)sender;
            m_set = menu.Tag as ImplantSet;
            m_characterScratchpad = m_character.After(m_set);
            
            UpdateContent();
        }

        /// <summary>
        /// Creates a scratchpad with the new implants values.
        /// </summary>
        /// <returns></returns>
        private CharacterScratchpad CreateModifiedScratchpad()
        {
            // Creates a scratchpad with new implants
            m_characterScratchpad = m_character.After(m_set);
            CharacterScratchpad scratchpad = new CharacterScratchpad(m_characterScratchpad);

            scratchpad.Memory.ImplantBonus += (int)nudMemory.Value - m_characterScratchpad.Memory.EffectiveValue;
            scratchpad.Charisma.ImplantBonus += (int)nudCharisma.Value - m_characterScratchpad.Charisma.EffectiveValue;
            scratchpad.Intelligence.ImplantBonus += (int)nudIntelligence.Value - m_characterScratchpad.Intelligence.EffectiveValue;
            scratchpad.Perception.ImplantBonus += (int)nudPerception.Value - m_characterScratchpad.Perception.EffectiveValue;
            scratchpad.Willpower.ImplantBonus += (int)nudWillpower.Value - m_characterScratchpad.Willpower.EffectiveValue;

            return scratchpad;
        }
        #endregion


        #region IPlanOrderPluggable Members
        /// <summary>
        /// Updates the statistics for the plan editor.
        /// </summary>
        /// <param name="plan"></param>
        /// <param name="areRemappingPointsActive"></param>
        public void UpdateStatistics(BasePlan plan, out bool areRemappingPointsActive)
        {
            areRemappingPointsActive = true;

            var scratchpad = CreateModifiedScratchpad();
            plan.UpdateStatistics(scratchpad, true, true);
            plan.UpdateOldTrainingTimes();
        }

        /// <summary>
        /// Updates the times when "choose implant set" changes.
        /// </summary>
        public void UpdateOnImplantSetChange()
        {
            UpdateTimes();
        }
        #endregion
    }
}