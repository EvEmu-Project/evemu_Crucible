using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using EVEMon.Common;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Represents the method that will handle change of attributes in optimization control.
    /// </summary>
    /// <param name="control">Source attribute optimization control</param>
    /// <param name="remapping">Current remapping in control</param>
    public delegate void AttributeChangedHandler(AttributesOptimizationControl control, AttributesOptimizer.RemappingResult remapping);

    /// <summary>
    /// Control that shows attribute remapping and allows to adjust it.
    /// </summary>
    public partial class AttributesOptimizationControl : UserControl
    {
        private readonly Character m_character;
        private readonly BasePlan m_plan;
        private readonly AttributesOptimizer.RemappingResult m_remapping;
        private readonly string m_description;

        /// <summary>
        /// Occurs when attributes changes. 
        /// </summary>
        [Category("Behavior")]
        public event AttributeChangedHandler AttributeChanged;

        /// <summary>
        /// Initializes a new instance of <see cref="AttributesOptimizationControl"/>.
        /// </summary>
        /// <param name="character">Character information</param>
        /// <param name="plan">Skill plan</param>
        /// <param name="remapping">Optimized remapping</param>
        /// <param name="description"></param>
        public AttributesOptimizationControl(Character character, BasePlan plan, AttributesOptimizer.RemappingResult remapping, string description)
        {
            InitializeComponent();

            m_character = character;
            m_plan = plan;
            m_remapping = remapping;
            m_description = description;

            UpdateControls(m_character, m_plan, m_remapping, m_description);
        }

        /// <summary>
        /// On load, use the <see cref="FontFactory"/> to retrieve fonts.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            lblUnassignedAttributePoints.Font = FontFactory.GetFont("Tahoma", 8.25F);
            lbWarning.Font = FontFactory.GetFont("Microsoft Sans Serif", 8.25F);
            lblMemory.Font = FontFactory.GetFont("Microsoft Sans Serif", 8.25F);
            lblWillpower.Font = FontFactory.GetFont("Microsoft Sans Serif", 8.25F);
            lblCharisma.Font = FontFactory.GetFont("Microsoft Sans Serif", 8.25F);
            lblPerception.Font = FontFactory.GetFont("Microsoft Sans Serif", 8.25F);
            lblIntelligence.Font = FontFactory.GetFont("Microsoft Sans Serif", 8.25F);
        }

        /// <summary>
        /// Updates bars and labels with given attributes from remapping.
        /// </summary>
        /// <param name="character">Character information</param>
        /// <param name="plan">Skill plan</param>
        /// <param name="remapping">Remapping with attributes and training time</param>
        /// <param name="description"></param>
        private void UpdateControls(Character character, BasePlan plan, AttributesOptimizer.RemappingResult remapping, string description)
        {
            var baseCharacter = character.After(plan.ChosenImplantSet);
            UpdateAttributeControls(baseCharacter, remapping, EveAttribute.Perception, lbPER, pbPERRemappable, pbPERImplants);
            UpdateAttributeControls(baseCharacter, remapping, EveAttribute.Willpower, lbWIL, pbWILRemappable, pbWILImplants);
            UpdateAttributeControls(baseCharacter, remapping, EveAttribute.Memory, lbMEM, pbMEMRemappable, pbMEMImplants);
            UpdateAttributeControls(baseCharacter, remapping, EveAttribute.Intelligence, lbINT, pbINTRemappable, pbINTImplants);
            UpdateAttributeControls(baseCharacter, remapping, EveAttribute.Charisma, lbCHA, pbCHARemappable, pbCHAImplants);

            // Update the description label
            labelDescription.Text = description;

            // Update the current time control
            lbCurrentTime.Text = remapping.BaseDuration.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);

            // Update the optimized time control
            lbOptimizedTime.Text = remapping.BestDuration.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);

            // Update the time benefit control
            if (remapping.BestDuration < remapping.BaseDuration)
            {
                lbGain.ForeColor = Color.Black;
                lbGain.Text = String.Format("{0} better than current",
                    remapping.BaseDuration.Subtract(remapping.BestDuration).ToDescriptiveText(DescriptiveTextOptions.IncludeCommas));
            }
            else
                if (remapping.BaseDuration < remapping.BestDuration)
                {
                    lbGain.ForeColor = Color.DarkRed;
                    lbGain.Text = String.Format("{0} slower than current",
                        remapping.BestDuration.Subtract(remapping.BaseDuration).ToDescriptiveText(DescriptiveTextOptions.IncludeCommas));
                }
                else
                {
                    lbGain.ForeColor = Color.Black;
                    lbGain.Text = @"Same as current";
                }

            // A plan may not have a years worth of skills in it,
            // only fair to warn the user
            lbWarning.Visible = remapping.BestDuration < new TimeSpan(365, 0, 0, 0);

            // Spare points
            int sparePoints = EveConstants.SpareAttributePointsOnRemap;
            for (int i = 0; i < 5; i++)
            {
                sparePoints -= (remapping.BestScratchpad[(EveAttribute)i].Base)- EveConstants.CharacterBaseAttributePoints;
            }
            pbUnassigned.Value = sparePoints;

            // If the implant set isn't the active one we notify the user
            lblNotice.Visible = (plan.ChosenImplantSet != character.ImplantSets.Current);
        }

        /// <summary>
        /// Updates bars and labels for specified attribute.
        /// </summary>
        /// <param name="character">Character information</param>
        /// <param name="remapping">Attribute remapping</param>
        /// <param name="attrib">Attribute that will be used to update controls</param>
        /// <param name="lb">Label control</param>
        /// <param name="pbRemappable">Attribute bar for remappable value</param>
        /// <param name="pbImplants">Attribute bar for implants</param>
        private void UpdateAttributeControls(
            BaseCharacter character,
            AttributesOptimizer.RemappingResult remapping,
            EveAttribute attrib,
            Label lb,
            AttributeBarControl pbRemappable,
            AttributeBarControl pbImplants)
        {
            // Compute base and effective attributes
            int effectiveAttribute = remapping.BestScratchpad[attrib].EffectiveValue;
            int oldBaseAttribute = remapping.BaseScratchpad[attrib].Base;
            int remappableAttribute = remapping.BestScratchpad[attrib].Base;
            int implantsBonus = remapping.BestScratchpad[attrib].ImplantBonus;

            // Update the label
            lb.Text = String.Format("{0} (new : {1} ; old : {2})", effectiveAttribute, remappableAttribute, oldBaseAttribute);

            // Update the bars
            pbRemappable.Value = remappableAttribute - EveConstants.CharacterBaseAttributePoints;
            pbImplants.Value = implantsBonus;
        }

        /// <summary>
        /// Calculates new remapping from values of controls.
        /// </summary>
        public void Recalculate()
        {
            var scratchpad = m_remapping.BaseScratchpad.Clone();
            scratchpad.Memory.Base = pbMEMRemappable.Value + EveConstants.CharacterBaseAttributePoints;
            scratchpad.Charisma.Base = pbCHARemappable.Value + EveConstants.CharacterBaseAttributePoints;
            scratchpad.Willpower.Base = pbWILRemappable.Value + EveConstants.CharacterBaseAttributePoints;
            scratchpad.Perception.Base = pbPERRemappable.Value + EveConstants.CharacterBaseAttributePoints;
            scratchpad.Intelligence.Base = pbINTRemappable.Value + EveConstants.CharacterBaseAttributePoints;

            // Get remapping for provided attributes
            var manualRemapping = new AttributesOptimizer.RemappingResult(m_remapping, scratchpad);
            manualRemapping.Update();
            UpdateControls(m_character, m_plan, manualRemapping, m_description);

            // Notify the changes
            if (AttributeChanged != null)
                AttributeChanged(this, manualRemapping);
        }

        #region Events

        /// <summary>
        /// Change of any attribute must be adjusted if there is no enough free points in the pool.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="deltaValue"></param>
        private void pbRemappable_ValueChanging(AttributeBarControl sender, ref int deltaValue)
        {
            // Adjust delta if there is no enough free points
            if (pbUnassigned.Value < deltaValue)
                deltaValue = pbUnassigned.Value;

            // Add/remove points from pool
            pbUnassigned.Value -= deltaValue;
        }

        /// <summary>
        /// Recalculate the time after change of an attribute.
        /// </summary>
        /// <param name="sender"></param>
        private void pb_ValueChanged(AttributeBarControl sender)
        {
            Recalculate();
        }

        /// <summary>
        /// Correct highlight if selected cell is inaccessable.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="highlightValue"></param>
        private void pbRemappable_Highlighting(AttributeBarControl sender, ref int highlightValue)
        {
            // Adjust possible highlight using free points in pool
            if (highlightValue - sender.Value > pbUnassigned.Value)
                highlightValue = sender.Value + pbUnassigned.Value;
        }

        /// <summary>
        /// Reset to original optimized remapping.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void buttonOptimize_Click(object sender, EventArgs e)
        {
            // Updates the remapping point with the optimized remapping
            m_remapping.Update();

            // Set all labels and bars to calculated optimized remap
            UpdateControls(m_character, m_plan, m_remapping, m_description);

            // Fires the event
            if (AttributeChanged != null)
                AttributeChanged(this, m_remapping);
        }

        /// <summary>
        /// Reset to remapping with current attributes.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void buttonCurrent_Click(object sender, EventArgs e)
        {
            // Make unoptimized remap
            var zeroRemapping = new AttributesOptimizer.RemappingResult(m_remapping, m_remapping.BaseScratchpad.Clone());
            zeroRemapping.Update();

            // Update the controls
            UpdateControls(m_character, m_plan, zeroRemapping, m_description);

            // Fires the event
            if (AttributeChanged != null)
                AttributeChanged(this, zeroRemapping);
        }

        /// <summary>
        /// One of +/- buttons was pressed.
        /// Check is it possible to change requested attribute and do it if we can.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void attributeButton_Click(object sender, EventArgs e)
        {
            var button = (sender as AttributeButtonControl);
            if (button == null)
                return;
            if (button.AttributeBar == null)
                return;

            // Adjust delta
            int deltaValue = button.ValueChange;
            if (pbUnassigned.Value < deltaValue)
                deltaValue = pbUnassigned.Value;

            if (deltaValue < 0 && button.AttributeBar.Value <= button.AttributeBar.BaseValue)
                return;

            if (button.AttributeBar.Value + deltaValue < button.AttributeBar.BaseValue)
                deltaValue = Math.Max(button.AttributeBar.Value - button.AttributeBar.BaseValue, deltaValue);

            if (button.AttributeBar.Value + deltaValue > button.AttributeBar.MaxPoints)
                deltaValue = button.AttributeBar.MaxPoints - button.AttributeBar.Value;

            if (deltaValue == 0)
                return;

            button.AttributeBar.Value += deltaValue;
            pbUnassigned.Value -= deltaValue;
            Recalculate();
        }

        /// <summary>
        /// Updates the controls with the values from the current remapping point.
        /// </summary>
        /// <param name="point"></param>
        public void UpdateValuesFrom(RemappingPoint point)
        {
            // Creates a scratchpad with the base values from the provided point.
            var scratchpad = new CharacterScratchpad(m_character.After(m_plan.ChosenImplantSet));
            for (int i = 0; i < 5; i++)
            {
                scratchpad[(EveAttribute)i].Base = point[(EveAttribute)i];
            }

            var remapping = new AttributesOptimizer.RemappingResult(m_remapping, scratchpad);
            remapping.Update();

            // Update the controls
            UpdateControls(m_character, m_plan, remapping, m_description);

            // Fires the event
            if (AttributeChanged != null)
                AttributeChanged(this, remapping);
        }
        #endregion
    }
}
