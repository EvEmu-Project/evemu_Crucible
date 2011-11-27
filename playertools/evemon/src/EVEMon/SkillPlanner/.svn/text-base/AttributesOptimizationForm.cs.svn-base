using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Controls;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Allows to wiew and change attribute remappings.
    /// </summary>
    public partial class AttributesOptimizationForm : EVEMonForm, IPlanOrderPluggable
    {
        /// <summary>
        /// Remapping strategy.
        /// </summary>
        public enum Strategy
        {
            /// <summary>
            /// Stratagy based on remapping points.
            /// </summary>
            RemappingPoints,
            /// <summary>
            /// Strategy based on the first year from a plan.
            /// </summary>
            OneYearPlan,
            /// <summary>
            /// Strategy based on already trained skills.
            /// </summary>
            Character,
            /// <summary>
            /// Used when the user double-click a remapping point to manually edit it.
            /// </summary>
            ManualRemappingPointEdition
        }

        private readonly Dictionary<AttributesOptimizationControl, AttributesOptimizer.RemappingResult> m_remappingDictionary;
        private PlanEditorControl m_planEditor;
        private BaseCharacter m_baseCharacter;
        private Character m_character;
        private Strategy m_strategy;
        private BasePlan m_plan;

        private Thread m_thread;

        private bool m_areRemappingPointsActive;
        private bool m_update = false;
        private string m_description;
        private CharacterScratchpad m_statisticsScratchpad;

        // Variables for manual edition of a plan
        private RemappingPoint m_manuallyEditedRemappingPoint;
        private AttributesOptimizer.RemappingResult m_remapping;


        /// <summary>
        /// Constructor for designer
        /// </summary>
        private AttributesOptimizationForm()
        {
            InitializeComponent();
            m_remappingDictionary = new Dictionary<AttributesOptimizationControl, AttributesOptimizer.RemappingResult>();
        }

        /// <summary>
        /// Constructor for use in code when optimizing remapping.
        /// </summary>
        /// <param name="character">Character information</param>
        /// <param name="plan">Plan to optimize for</param>
        /// <param name="strategy">Optimization strategy</param>
        /// <param name="name">Title of this form</param>
        /// <param name="description">Description of the optimization operation</param>
        public AttributesOptimizationForm(Character character, BasePlan plan, Strategy strategy, string name, string description)
            : this()
        {
            m_character = character;
            m_baseCharacter = character.After(plan.ChosenImplantSet);
            m_strategy = strategy;
            m_plan = plan;
            m_description = description;
            Text = name;
        }

        /// <summary>
        /// Constructor for use in code when the user wants to manually edit a remapping point.
        /// </summary>
        /// <param name="character">Character information</param>
        /// <param name="plan">Plan to optimize for</param>
        /// <param name="strategy">Optimization strategy</param>
        /// <param name="name">Title of this form</param>
        /// <param name="description">Description of the optimization operation</param>
        public AttributesOptimizationForm(Character character, Plan plan, RemappingPoint point)
            : this()
        {
            m_plan = plan;
            m_character = character;
            m_baseCharacter = character.After(plan.ChosenImplantSet);
            m_manuallyEditedRemappingPoint = point;
            m_strategy = Strategy.ManualRemappingPointEdition;
            m_description = "Manual editing of a remapping point";
            Text = "Remapping point manual editing (" + plan.Name + ")";
        }

        /// <summary>
        /// Gets or sets a <see cref="PlanEditorControl"/>.
        /// </summary>
        public PlanEditorControl PlanEditor
        {
            get { return m_planEditor; }
            set { m_planEditor = value; }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            this.lvPoints.Font = FontFactory.GetFont("Arial", 9F);
            this.throbber.State = ThrobberState.Rotating;

            m_thread = new Thread(new ThreadStart(Run));
            m_thread.Start();

            lvPoints.Font = FontFactory.GetDefaultFont(9F, FontStyle.Regular);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosed(EventArgs e)
        {
            // Stop the thread
            if (m_thread != null)
            {
                m_thread.Abort();
                m_thread = null;
            }

            this.m_planEditor = null;

            // Base call
            base.OnClosed(e);
        }

        /// <summary>
        /// Starts optimization.
        /// </summary>
        private void Run()
        {
            // Compute best scratchpad
            var bestDuration = TimeSpan.Zero;
            AttributesOptimizer.RemappingResult remapping = null;
            List<AttributesOptimizer.RemappingResult> remappingList = null;

            switch (m_strategy)
            {
                case Strategy.ManualRemappingPointEdition:
                    m_areRemappingPointsActive = true;
                    if (m_update)
                    {
                        remapping = m_remapping;
                        m_manuallyEditedRemappingPoint = remapping.Point.Clone();
                    }
                    else
                    {
                        remapping = AttributesOptimizer.GetResultsFromRemappingPoints(m_plan).Single(x => x.Point == m_manuallyEditedRemappingPoint);
                        m_manuallyEditedRemappingPoint = m_manuallyEditedRemappingPoint.Clone();
                        m_remapping = remapping;
                    }
                    remapping.Optimize(TimeSpan.MaxValue);
                    break;
                case Strategy.Character:
                    m_areRemappingPointsActive = false;
                    remapping = AttributesOptimizer.OptimizeFromCharacter(m_character, m_plan);
                    break;
                case Strategy.OneYearPlan:
                    m_areRemappingPointsActive = false;
                    remapping = AttributesOptimizer.OptimizeFromFirstYearOfPlan(m_plan);
                    break;
                case Strategy.RemappingPoints:
                    m_areRemappingPointsActive = true;
                    remappingList = AttributesOptimizer.OptimizeFromPlanAndRemappingPoints(m_plan);
                    break;
                default:
                    throw new NotImplementedException();
            }

            if (m_update)
            {
                // Update the controls for every attribute on the already shown form
                UpdateForm(remapping, remappingList);
            }
            else
            {
                // Update the controls for every attribute
                this.Invoke((MethodInvoker)(() => UpdateForm(remapping, remappingList)));
            }
        }

        /// <summary>
        /// Updates controls on the form.
        /// </summary>
        /// <param name="remapping">An <see cref="AttributesOptimizer.Remapping"/> object</param>
        /// <param name="remappingList">List of remappings</param>
        private void UpdateForm(AttributesOptimizer.RemappingResult remapping, List<AttributesOptimizer.RemappingResult> remappingList)
        {
            // If the thread has been canceled, we stop right now to prevent an exception
            if (m_thread == null)
                return;

            // Hide the throbber and the waiting message
            this.throbber.State = ThrobberState.Stopped;
            this.panelWait.Visible = false;

            this.tabControl.Controls.Clear();

            // Update the attributes
            if (remapping != null)
            {
                m_statisticsScratchpad = remapping.BestScratchpad.Clone();
                UpdateForRemapping(remapping);
            }
            else
            {
                UpdateForRemappingList(remappingList);
            }

            // Update the plan order's column
            if (m_planEditor != null && (remapping != null || remappingList.Count != 0))
                this.m_planEditor.ShowWithPluggable(this);
        }

        /// <summary>
        /// Updates the UI once the computation has been done (for whole plan or character from birth)
        /// </summary>
        /// <param name="remapping"></param>
        private void UpdateForRemapping(AttributesOptimizer.RemappingResult remapping)
        {
            // Create control
            var ctl = CreateAttributesOptimizationControl(remapping, m_description);
            this.Controls.Add(ctl);
            ctl.BringToFront();
        }

        /// <summary>
        /// Updates the UI once the computations with remapping points strategy have been done.
        /// </summary>
        /// <param name="bestDuration"></param>
        /// <param name="remappingList"></param>
        /// <returns></returns>
        private void UpdateForRemappingList(List<AttributesOptimizer.RemappingResult> remappingList)
        {
            // Display "no result" or "summary"
            if (remappingList.Count == 0)
            {
                panelNoResult.Visible = true;
            }
            else
            {
                // Adds a tab page for the summary
                this.tabControl.Controls.Add(tabSummary);

                // Updates the summary informations
                UpdateSummaryInformation(remappingList);

                // Adds a tab page for every remapping
                int index = 1;
                foreach (var remap in remappingList)
                {
                    AddTabPage(remap, "#" + index.ToString(), m_description);
                    index++;
                }

                this.tabControl.Visible = true;
                tabSummary.Focus();
            }
        }

        /// <summary>
        /// Updates information in summary page.
        /// </summary>
        /// <param name="remappingList">List of remappings</param>
        private void UpdateSummaryInformation(IEnumerable<AttributesOptimizer.RemappingResult> remappingList)
        {
            var baseDuration = m_plan.GetTotalTime(m_character.After(m_plan.ChosenImplantSet), false);
            this.lvPoints.Items.Clear();

            // Add global informations
            ListViewGroup globalGroup = new ListViewGroup("Global informations");
            this.lvPoints.Groups.Add(globalGroup);

            TimeSpan savedTime = TimeSpan.Zero;
            foreach (var remap in remappingList)
            {
                savedTime += (remap.BaseDuration - remap.BestDuration);
            }

            this.lvPoints.Items.Add(new ListViewItem(String.Format(CultureConstants.DefaultCulture, "Current time : {0}",
                baseDuration.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas), globalGroup)));

            if (savedTime != TimeSpan.Zero)
            {
                this.lvPoints.Items.Add(new ListViewItem(String.Format(CultureConstants.DefaultCulture, "Optimized time : {0}",
                    baseDuration.Subtract(savedTime).ToDescriptiveText(DescriptiveTextOptions.IncludeCommas), globalGroup)));

                if (savedTime < TimeSpan.Zero)
                {
                    ListViewItem savedTimeItem = this.lvPoints.Items.Add(new ListViewItem(
                        String.Format(CultureConstants.DefaultCulture, "{0} slower than current",
                        (-savedTime).ToDescriptiveText(DescriptiveTextOptions.IncludeCommas), globalGroup)));
                    savedTimeItem.ForeColor = Color.DarkRed;
                }
                else
                {
                    this.lvPoints.Items.Add(new ListViewItem(String.Format(CultureConstants.DefaultCulture, "{0} better than current",
                        savedTime.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas), globalGroup)));
                }
            }
            else
            {
                this.lvPoints.Items.Add(new ListViewItem("Your attributes are already optimal", globalGroup));
            }

            // Notify plan updated
            var lvi = new ListViewItem("Your plan has been updated.", globalGroup);
            lvi.Font = FontFactory.GetFont(lvPoints.Font, FontStyle.Bold);
            this.lvPoints.Items.Add(lvi);

            // Add pages and summary informations
            TimeSpan lastRemap = TimeSpan.Zero;
            foreach (var remap in remappingList)
            {
                AddSummaryForRemapping(remap, ref lastRemap);
            }

            this.columnHeader.Width = this.lvPoints.ClientSize.Width;
        }

        /// <summary>
        /// Adds summury information for given remapping.
        /// </summary>
        /// <param name="remap">Remapping object</param>
        /// <param name="lastRemap">Time of previous remapping</param>
        private void AddSummaryForRemapping(AttributesOptimizer.RemappingResult remap, ref TimeSpan lastRemap)
        {
            // Create the group
            string text = String.Format(CultureConstants.DefaultCulture, "{0} at {1}",
                remap.ToString(m_character), remap.StartTime.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas));
            ListViewGroup group = new ListViewGroup(text);
            this.lvPoints.Groups.Add(group);

            // Add five items, one for each attribute
            AddItemForAttribute(remap, group, EveAttribute.Intelligence);
            AddItemForAttribute(remap, group, EveAttribute.Perception);
            AddItemForAttribute(remap, group, EveAttribute.Charisma);
            AddItemForAttribute(remap, group, EveAttribute.Willpower);
            AddItemForAttribute(remap, group, EveAttribute.Memory);

            // Check there are at least one year between each remap
            TimeSpan timeSinceLastRemap = remap.StartTime - lastRemap;
            if (timeSinceLastRemap < TimeSpan.FromDays(365.0) && lastRemap != TimeSpan.Zero)
            {
                var item = new ListViewItem(String.Format(CultureConstants.DefaultCulture, "The previous remap was only {0} ago.",
                    timeSinceLastRemap.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas), group));
                item.ForeColor = Color.DarkRed;
                lvPoints.Items.Add(item);
            }

            lastRemap = remap.StartTime;
        }

        /// <summary>
        /// Adds the list item for the given attribute.
        /// </summary>
        /// <param name="remap"></param>
        /// <param name="group"></param>
        /// <param name="attrib"></param>
        private void AddItemForAttribute(AttributesOptimizer.RemappingResult remap, ListViewGroup group, EveAttribute attrib)
        {
            StringBuilder builder = new StringBuilder(attrib.ToString());
            int difference = remap.BestScratchpad[attrib].Base - remap.BaseScratchpad[attrib].Base;

            // Add the list view item for this attribute
            string itemText = RemappingPoint.GetStringForAttribute(attrib, remap.BaseScratchpad, remap.BestScratchpad);
            lvPoints.Items.Add(new ListViewItem(itemText, group));
        }

        /// <summary>
        /// Adds the tab page for the given remapping
        /// </summary>
        /// <param name="remapping"></param>
        /// <param name="tabName"></param>
        private void AddTabPage(AttributesOptimizer.RemappingResult remapping, string tabName, string description)
        {
            var ctl = CreateAttributesOptimizationControl(remapping, description);
            m_remappingDictionary[ctl] = remapping;

            TabPage page = new TabPage(tabName);
            page.Controls.Add(ctl);

            this.tabControl.TabPages.Add(page);
        }

        /// <summary>
        /// Creates a <see cref="AttributesOptimizationControl"/> for a given remapping.
        /// </summary>
        /// <param name="remapping">The remapping object to represents.</param>
        /// <returns>The created control.</returns>
        private AttributesOptimizationControl CreateAttributesOptimizationControl(AttributesOptimizer.RemappingResult remapping, string description)
        {
            var ctl = new AttributesOptimizationControl(m_character, m_plan, remapping, description);
            ctl.AttributeChanged += new AttributeChangedHandler(AttributesOptimizationControl_AttributeChanged);

            // For a manually edited point, we initialize the control with the attributes from the current remapping point
            if (m_strategy == Strategy.ManualRemappingPointEdition && m_manuallyEditedRemappingPoint.Status == RemappingPoint.PointStatus.UpToDate)
            {
                ctl.UpdateValuesFrom(m_manuallyEditedRemappingPoint);
            }

            return ctl;
        }

        /// <summary>
        /// Racalculating plan and summary page after change of a <see cref="AttributesOptimizationControl"/>.
        /// </summary>
        /// <param name="control"></param>
        /// <param name="remapping"></param>
        void AttributesOptimizationControl_AttributeChanged(AttributesOptimizationControl control, AttributesOptimizer.RemappingResult remapping)
        {
            // Update the plan order's column
            if (m_planEditor != null)
            {
                if (m_strategy == Strategy.RemappingPoints)
                {
                    m_remappingDictionary[control] = remapping;
                    UpdateSummaryInformation(m_remappingDictionary.Values);
                }

                m_statisticsScratchpad = remapping.BestScratchpad.Clone();
                this.m_planEditor.ShowWithPluggable(this);
                m_remapping = remapping;
            }
        }

        #region IPlanOrderPluggable Members
        /// <summary>
        /// Updates the statistics for the plan editor.
        /// </summary>
        /// <param name="plan"></param>
        /// <param name="areRemappingPointsActive"></param>
        public void UpdateStatistics(BasePlan plan, out bool areRemappingPointsActive)
        {
            areRemappingPointsActive = m_areRemappingPointsActive;

            if (m_areRemappingPointsActive)
            {
                plan.UpdateStatistics(new CharacterScratchpad(m_baseCharacter.After(plan.ChosenImplantSet)), true, true);
            }
            else
            {
                plan.UpdateStatistics(m_statisticsScratchpad.Clone(), false, true);
            }

            plan.UpdateOldTrainingTimes(new CharacterScratchpad(m_baseCharacter.After(plan.ChosenImplantSet)), false, true);
        }

        /// <summary>
        /// Updates the times when "choose implant set" changes.
        /// </summary>
        /// <param name="plan"></param>
        /// <param name="areRemappingPointsActive"></param>
        public void UpdateOnImplantSetChange()
        {
            m_update = true;
            Run();
        }
        #endregion
    }
}
