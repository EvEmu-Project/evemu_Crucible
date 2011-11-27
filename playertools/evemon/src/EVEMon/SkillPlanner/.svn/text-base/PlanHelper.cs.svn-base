using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common;
using System.Windows.Forms;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Helper for the "Plan To" and "Remove" menus.
    /// </summary>
    public static class PlanHelper
    {
        /// <summary>
        /// Updates a regular "Plan to X" menu : text, tag, enable/disable.
        /// </summary>
        /// <param name="menu"></param>
        /// <param name="plan"></param>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        public static bool UpdatesRegularPlanToMenu(ToolStripItem menu, Plan plan, Skill skill, int level)
        {
            if (level == 0)
            {
                menu.Text = "Remove";
            }
            else
            {
                menu.Text = "Level " + level.ToString();
            }

            menu.Enabled = EnablePlanTo(plan, skill, level);
            if (menu.Enabled)
            {
                var operation = plan.TryPlanTo(skill, level);
                menu.Tag = operation;
                if (RequiresWindow(operation))
                    menu.Text += "...";
            }

            var menuItem = menu as ToolStripMenuItem;
            if (menuItem != null)
            {
                menuItem.Checked = (plan.GetPlannedLevel(skill) == level);
            }
            return menu.Enabled;
        }

        /// <summary>
        /// Checks whether the given skill level can be planned. Used to enable or disable the "Plan To N" and "Remove" menu options.
        /// </summary>
        /// <param name="plan"></param>
        /// <param name="skill"></param>
        /// <param name="level">A integer between 0 (remove all entries for this skill) and 5.</param>
        /// <param name="operation"></param>
        /// <returns></returns>
        public static bool EnablePlanTo(Plan plan, Skill skill, int level)
        {
            // The entry actually wants to remove the item
            if (level == 0)
            {
                return plan.IsPlanned(skill);
            }
            // The entry is already known
            else if (skill.Level >= level)
            {
                return false;
            }
            // The entry is already planned at this very level
            else if (plan.GetPlannedLevel(skill) == level)
            {
                return false;
            }
            // Ok
            return true;
        }

        /// <summary>
        /// Checks whether the given operation absolutely requires a confirmation from the user.
        /// True when there are dependencies to remove.
        /// </summary>
        /// <param name="operation"></param>
        /// <returns></returns>
        public static bool RequiresWindow(IPlanOperation operation)
        {
            if (operation == null)
                return false;

            if (operation.Type != PlanOperations.Suppression)
                return false;

            return (operation.SkillsToRemove.Count != operation.AllEntriesToRemove.Count);
        }

        /// <summary>
        /// Performs the action for the "Plan To N" and "Remove" menu options, in a silent way whenever possible.
        /// </summary>
        /// <param name="parentForm"></param>
        /// <param name="operation"></param>
        /// <returns></returns>
        public static bool PerformSilently(IPlanOperation operation)
        {
            var window = WindowsFactory<PlanWindow>.GetByTag(operation.Plan);
            return PerformSilently(window, operation);
        }

        /// <summary>
        /// Performs the action for the "Plan To N" and "Remove" menu options, in a silent way whenever possible.
        /// </summary>
        /// <param name="parentForm"></param>
        /// <param name="operation"></param>
        /// <returns></returns>
        public static bool PerformSilently(Form parentForm, IPlanOperation operation)
        {
            if (operation == null)
                return false;

            // A window is required
            if (RequiresWindow(operation))
            {
                return Perform(parentForm, operation);
            }

            // Silent way
            operation.Perform();
            return (operation.Type == PlanOperations.None);
        }

        /// <summary>
        /// Performs the action for the "Plan To N" and "Remove" menu options.
        /// </summary>
        /// <param name="operation"></param>
        /// <returns></returns>
        public static bool Perform(IPlanOperation operation)
        {
            var window = WindowsFactory<PlanWindow>.GetByTag(operation.Plan);
            return Perform(window, operation);
        }

        /// <summary>
        /// Performs the action for the "Plan To N" and "Remove" menu options.
        /// </summary>
        /// <param name="operation"></param>
        /// <returns></returns>
        public static bool Perform(Form parentForm, IPlanOperation operation)
        {
            using (var window = new PlanToOperationForm(operation))
            {
                var result = window.ShowDialog(parentForm);
                return result == DialogResult.OK;
            }
        }

        /// <summary>
        /// Selects which type of Perform will be called according to user settings.
        /// </summary>
        /// <param name="operation"></param>
        /// <returns></returns>
        public static bool SelectPerform(IPlanOperation operation)
        {
            if (Settings.UI.PlanWindow.UseAdvanceEntryAddition && operation.Type == PlanOperations.Addition)
            {
                return Perform(operation);
            }
            else
            {
                return PerformSilently(operation);
            }
        }
    }
}
