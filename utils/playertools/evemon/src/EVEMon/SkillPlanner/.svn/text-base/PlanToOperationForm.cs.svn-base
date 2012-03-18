using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common.Controls;
using EVEMon.Common;
using EVEMon.Controls;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// This window allows the user to deal with addition and suppresion of entries of plans.
    /// </summary>
    public partial class PlanToOperationForm : EVEMonForm
    {
        private IPlanOperation m_operation;

        /// <summary>
        /// Designer constructor
        /// </summary>
        public PlanToOperationForm()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Constructor for use in-code.
        /// </summary>
        /// <param name="operation"></param>
        public PlanToOperationForm(IPlanOperation operation)
            :this()
        {
            if (operation.Type == PlanOperations.None)
            {
                throw new ArgumentException("This window doesn't support empty operations.", "operation");
            }

            m_operation = operation;
            rootMultiPanel.SelectedPage = additionPage;
        }

        /// <summary>
        /// On load, selects the proper page.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            // Are we performing an addition ?
            if (m_operation.Type == PlanOperations.Addition)
            {
                this.Text = "Add entries to plan";

                // Updates the other buttons
                previousButton.Visible = false;
                nextButton.DialogResult = DialogResult.OK;
                nextButton.Text = "OK";
                nextButton.Focus();

                // Loads the listbox
                FillListBox(m_operation.AllEntriesToAdd, additionsListBox);

                // Priority controls
                if (m_operation.HighestPriorityForAddition < 1)
                {
                    priorityNumericBox.Minimum = 1;
                    priorityGroup.Visible = false;
                }
                else
                {
                    priorityNumericBox.Minimum = m_operation.HighestPriorityForAddition;
                    priorityLabel.Text = "The highest priority you can set is " + m_operation.HighestPriorityForAddition.ToString();
                }
            }
            // Suppression 
            else
            {
                this.Text = "Remove entries from plan";
                previousButton.Visible = (m_operation.RemovablePrerequisites.Count != 0);
                cancelButton.Focus();

                // Updates the selected page
                this.rootMultiPanel.SelectionChange += new MultiPanelSelectionChangeHandler(rootMultiPanel_SelectionChange);
                rootMultiPanel.SelectedPage = dependenciesSuppressionPage;

                // Loads the entries to remove listbox
                FillListBox(m_operation.AllEntriesToRemove, suppressionListBox);

                // Loads the useless prerequisites listbox
                FillListBox(m_operation.RemovablePrerequisites, uselessPrereqsListBox);
            }
        }

        /// <summary>
        /// Fetches the items to the given list box.
        /// </summary>
        private void FillListBox<T>(IEnumerable<T> items, ListBox listBox)
            where T : ISkillLevel
        {
            var plan = new PlanScratchpad(m_operation.Plan.Character);
            plan.RebuildPlanFrom(items.Select(x => new PlanEntry(x.Skill, x.Level)));
            plan.Fix();

            listBox.Items.Clear();
            foreach (var entry in plan)
            {
                string name = entry.ToString();

                if (m_operation.Type == PlanOperations.Addition)
                { 
                    // Skip if the entry is already in the plan
                    if (m_operation.Plan.IsPlanned(entry.Skill, entry.Level))
                        continue;
                }
                else
                {
                    // On creation of "entries to remove" listbox (first pass),
                    // skip if entry type is of prerequisite.
                    // "Useless prerequisites" listbox is created on second pass
                    // and in that case entry type is of type planned.
                    if (entry.Type == PlanEntryType.Prerequisite)
                        continue;

                    if (entry.Type == PlanEntryType.Planned)
                        name += " (planned)";
                }

                listBox.Items.Add(name);
            }
        }

        /// <summary>
        /// When the selected page changes, we update the buttons. Only occurs for suppression.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void rootMultiPanel_SelectionChange(object sender, MultiPanelSelectionChangeEventArgs args)
        {

            // When there are useless prereqs, we offer the user to remove them on a second page.
            // When there aren't any, we just have one page.

            // Guess whether we're on the final page (2/2 or 1/1)
            bool isFinal = false;
            if (rootMultiPanel.SelectedPage == uselessPrereqsSuppressionPage)
            {
                isFinal = true;
            }
            else
            {
                isFinal = (m_operation.RemovablePrerequisites.Count == 0);
            }

            // Final page ? 
            if (isFinal)
            {
                nextButton.Text = "OK";
                nextButton.DialogResult = DialogResult.OK;
                previousButton.Enabled = true;
            }
            else
            {
                nextButton.Text = "&Next >";
                nextButton.DialogResult = DialogResult.None;
                previousButton.Enabled = false;
            }
        }

        /// <summary>
        /// Click on the previous button
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void previousButton_Click(object sender, EventArgs e)
        {
            rootMultiPanel.SelectedPage = dependenciesSuppressionPage;
        }

        /// <summary>
        /// Click on the Next/OK button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nextButton_Click(object sender, EventArgs e)
        {
            // Are we on the first page ? Then it's "Next >".
            if (nextButton.DialogResult == DialogResult.None)
            {
                rootMultiPanel.SelectedPage = uselessPrereqsSuppressionPage;
                return;
            }

            // Then we're on the last page and the button is now "OK"
            if (m_operation.Type == PlanOperations.Addition)
            {
                m_operation.PerformAddition((int)priorityNumericBox.Value);
            }
            else
            {
                m_operation.PerformSuppression(uselessPrereqsCheckbox.Checked);
            }

            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        /// <summary>
        /// Click on the cancel button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cancelButton_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }
    }
}
