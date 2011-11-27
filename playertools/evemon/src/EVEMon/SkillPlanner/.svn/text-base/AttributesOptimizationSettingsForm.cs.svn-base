using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.SkillPlanner
{
    public partial class AttributesOptimizationSettingsForm : EVEMonForm
    {
        private AttributesOptimizationForm m_optimizationForm;
        private readonly Character m_character;
        private readonly Plan m_plan;

        public AttributesOptimizationSettingsForm(Plan plan)
        {
            InitializeComponent();

            buttonWholePlan.Font = FontFactory.GetFont("Microsoft Sans Serif", 10F);
            buttonCharacter.Font = FontFactory.GetFont("Microsoft Sans Serif", 10F);
            buttonRemappingPoints.Font = FontFactory.GetFont("Microsoft Sans Serif", 10F);

            m_plan = plan;
            m_character = (Character)plan.Character;
        }

        public AttributesOptimizationForm OptimizationForm
        {
            get { return m_optimizationForm; }
        }

        private void buttonRemappingPoints_Click(object sender, EventArgs e)
        {
            string title = "Attributes optimization (" + m_plan.Name + ", remapping points)";
            string description = "Based on " + m_plan.Name + "; using the remapping points you defined.";
            m_optimizationForm = new AttributesOptimizationForm(m_character, m_plan, 
                AttributesOptimizationForm.Strategy.RemappingPoints, title, description);
        }

        private void buttonWholePlan_Click(object sender, EventArgs e)
        {
            string title = "Attributes optimization (" + m_plan.Name + ", first year)";
            string description = "Based on " + m_plan.Name + "; best attributes for the first year.";
            m_optimizationForm = new AttributesOptimizationForm(m_character, m_plan,
                AttributesOptimizationForm.Strategy.OneYearPlan, title, description);
        }

        private void buttonCharacter_Click(object sender, EventArgs e)
        {
            string title = "Attributes optimization (" + m_character.Name + ")";
            string description = "Based on " + m_character.Name;
            description += (description.EndsWith("s") ? "' skills" : "'s skills");
            m_optimizationForm = new AttributesOptimizationForm(m_character, m_plan,
                AttributesOptimizationForm.Strategy.Character, title, description);
        }
    }
}
