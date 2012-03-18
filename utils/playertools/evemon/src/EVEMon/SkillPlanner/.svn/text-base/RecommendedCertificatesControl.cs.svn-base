using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Globalization;
using System.Linq;

using EVEMon.Common;
using EVEMon.Common.Data;


namespace EVEMon.SkillPlanner
{
    public partial class RecommendedCertificatesControl : UserControl
    {
        private const int GrantedIcon = 0;
        private const int ClaimableIcon = 1;
        private const int UnknownButTrainableIcon = 2;
        private const int UnknownIcon = 3;
        private const int CertificateIcon = 4;
        private const int SkillIcon = 5;
        private const int Planned = 6;
        
        private Item m_object;
        private Plan m_plan;
        TimeSpan trainTime;

        #region Constructors
        /// <summary>
        /// User control to display required certificates for a given eveobject and update a plan object for requirements not met.
        /// </summary>
        public RecommendedCertificatesControl()
        {
            this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            this.SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
            
            InitializeComponent();

            tvCertList.DrawNode += new DrawTreeNodeEventHandler(tvCertList_DrawNode);
            tvCertList.MouseDown += new MouseEventHandler(tvCertList_MouseDown);

            this.Disposed += new EventHandler(OnDisposed);
            EveClient.PlanChanged += new EventHandler<PlanChangedEventArgs>(EveClient_PlanChanged);
        }

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            this.Disposed -= new EventHandler(OnDisposed);
            EveClient.PlanChanged -= new EventHandler<PlanChangedEventArgs>(EveClient_PlanChanged);
        }

        /// <summary>
        /// Occurs when the plan changes, when update the display.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_PlanChanged(object sender, PlanChangedEventArgs e)
        {
            UpdateDisplay();
        }
        #endregion

        #region Public Properties
        /// <summary>
        /// An EveObject for which we want to show required skills.
        /// </summary>
        public Item Object
        {
            get { return m_object; }
            set
            {
                m_object = value;
                UpdateDisplay();
            }
        }
        /// <summary>
        /// The target Plan object to add any required skills.
        /// </summary>
        public Plan Plan
        {
            get { return m_plan; }
            set
            {
                m_plan = value;
                UpdateDisplay();
            }
        }
        #endregion

        #region Content creation
        /// <summary>
        /// Updates control contents.
        /// </summary>
        private void UpdateDisplay()
        {
            trainTime = TimeSpan.Zero;

            // Default all known flag to true. Will be set to false in UpdateNode() if a requirement is not met
            bool allCertsKnown = true;

            // Default unplanned certificates flag to false. Will be set to true in UpdateNode() if a requirement is neither met nor planned
            bool certsUnplanned = false;

            // Treeview update
            tvCertList.BeginUpdate();
            try
            {
                tvCertList.Nodes.Clear();
                if (m_object != null && m_plan != null)
                {
                    // Recursively create nodes
                    foreach (var cert in StaticCertificates.AllCertificates.Where(x=> x.Recommendations.Contains(m_object)))
                    {
                        tvCertList.Nodes.Add(GetCertNode(cert));
                    }
                }

                // Update the nodes
                foreach (TreeNode node in tvCertList.Nodes)
                {
                    UpdateNode(node, ref allCertsKnown, ref certsUnplanned);
                }
            }
            finally
            {
                tvCertList.EndUpdate();
            }

            // Set training time required label
            if (allCertsKnown)
            {
                lblTimeRequired.Text = "No training required";
            }
            else
            {
                lblTimeRequired.Text = trainTime.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);
            }

            // Set minimun control size
            Size timeRequiredTextSize = TextRenderer.MeasureText(lblTimeRequired.Text, Font);
            Size newMinimumSize = new Size(timeRequiredTextSize.Width + btnAddCerts.Width, 0);
            if (this.MinimumSize.Width < newMinimumSize.Width)
                this.MinimumSize = newMinimumSize;

            // Enable / disable button
            btnAddCerts.Enabled = certsUnplanned;
        }
        
        /// <summary>
        /// Recursive method to generate treenodes for tvCertList.
        /// </summary>
        /// <param name="certificate">An EntityRecommendedCertificate object</param>
        /// <returns></returns>
        private TreeNode GetCertNode(StaticCertificate certificate)
        {
            var character = (Character)m_plan.Character;
            Certificate cert = character.Certificates[certificate.ID];

            TreeNode node = new TreeNode(cert.ToString());
            node.Tag = cert;

            // Generate child certificate nodes if required
            foreach (StaticCertificate childCert in cert.StaticData.PrerequisiteCertificates)
            {
                node.Nodes.Add(GetCertNode(childCert));

            }

            // Generate prerequisites skill nodes if required
            foreach (StaticSkillLevel prereq in cert.StaticData.PrerequisiteSkills)
            {
                node.Nodes.Add(GetSkillNode(prereq));
            }

            return node;
        }

        /// <summary>
        /// Recursive method to generate treenodes for tvCertList.
        /// </summary>
        /// <param name="requiredSkill">An EntityRequiredSkill object</param>
        /// <returns></returns>
        private TreeNode GetSkillNode(StaticSkillLevel prereq)
        {
            var character = (Character)m_plan.Character;
            Skill skill = character.Skills[prereq.Skill];

            TreeNode node = new TreeNode(prereq.ToString());
            node.Tag = new SkillLevel(skill, prereq.Level);

            // Generate child prerequisite skill nodes if required
            foreach (StaticSkillLevel childPrereq in skill.StaticData.Prerequisites)
            {
                node.Nodes.Add(GetSkillNode(childPrereq));
            }

            return node;
        }

        /// <summary>
        /// Updates the index image of the specified node and its children.
        /// </summary>
        /// <param name="node"></param>
        private void UpdateNode(TreeNode node, ref bool allCertsKnown, ref bool certsUnplanned)
        {
            Certificate cert = node.Tag as Certificate;

            // The node represents a certificate
            if (cert != null)
            {
                switch (cert.Status)
                {
                    case CertificateStatus.Granted:
                        node.ImageIndex = GrantedIcon;
                        break;
                    case CertificateStatus.Claimable:
                        node.ImageIndex = ClaimableIcon;
                        break;
                    case CertificateStatus.PartiallyTrained:
                        node.ImageIndex = UnknownButTrainableIcon;
                        break;
                    case CertificateStatus.Untrained:
                        node.ImageIndex = UnknownIcon;
                        break;
                    default:
                        throw new NotImplementedException();
                }
            }
            // The node represents a skill prerequisite
            else
            {
                SkillLevel skillPrereq = (SkillLevel)node.Tag;
                var character = (Character)m_plan.Character;
                var skill = character.Skills[skillPrereq.Skill];

                // Skill requirement met
                if (skill.Level >= skillPrereq.Level)
                {
                    node.ImageIndex = GrantedIcon;
                    node.SelectedImageIndex = GrantedIcon;
                }
                // Requirement not met, but planned
                else if (m_plan.IsPlanned(skill, skillPrereq.Level))
                {
                    node.ImageIndex = Planned;
                    node.SelectedImageIndex = Planned;
                    allCertsKnown = false;
                }
                // Requirement not met, but not planned
                else if (skill.IsKnown)
                { 
                    node.ImageIndex = UnknownButTrainableIcon;
                    node.SelectedImageIndex = UnknownButTrainableIcon;
                    allCertsKnown = false;
                    certsUnplanned = true;
                }
                // Requirement not met
                else
                {
                    node.ImageIndex = UnknownIcon;
                    node.SelectedImageIndex = UnknownIcon;
                    allCertsKnown = false;
                    certsUnplanned = true;
                }
            }

            // When selected, the image remains the same
            node.SelectedImageIndex = node.ImageIndex;

            // Update the children
            foreach (TreeNode child in node.Nodes)
            {
                UpdateNode(child, ref allCertsKnown, ref certsUnplanned);
            }
        }

        /// <summary>
        /// Custom draw for the label.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvCertList_DrawNode(object sender, DrawTreeNodeEventArgs e)
        {
            // Prevents a bug that causes every item to be redrawn at the top left corner
            if (e.Bounds.Left <= 10)
            {
                e.DrawDefault = true;
                return;
            }

            string line = String.Empty;
            int supIcon = -1;

            // Is it a certificate ?
            Certificate cert = e.Node.Tag as Certificate;
            if (cert != null)
            {
                var status = cert.Status;
                line = cert.ToString();
                supIcon = CertificateIcon;

                // When not granted or claimable, let's calculate the training time
                if (status != CertificateStatus.Claimable && status != CertificateStatus.Granted)
                    trainTime += cert.GetTrainingTime();
            }
            // Or a skill prerequisite ?
            else
            {
                var skillPrereq = (SkillLevel)e.Node.Tag;
                line = skillPrereq.ToString();
                supIcon = SkillIcon;

                // When not known to the require level, let's calculate the training time
                var skill = skillPrereq.Skill;
                if (!skillPrereq.IsKnown)
                    trainTime += skill.GetLeftTrainingTimeToLevel(skillPrereq.Level);
            }

            // Choose colors according to selection
            bool isSelected = ((e.State & TreeNodeStates.Selected) == TreeNodeStates.Selected);
            var backColor = (isSelected ? SystemColors.Highlight : tvCertList.BackColor);
            var foreColor = (isSelected ? SystemColors.HighlightText : tvCertList.ForeColor);
            var lightForeColor = (isSelected ? SystemColors.HighlightText : SystemColors.GrayText);

            // Draws the background
            using (var background = new SolidBrush(backColor))
            {
                var width = tvCertList.ClientSize.Width - e.Bounds.Left;
                e.Graphics.FillRectangle(background, new Rectangle(e.Bounds.Left, e.Bounds.Top, width, e.Bounds.Height));
            }

            // Performs the drawing
            using (var foreground = new SolidBrush(foreColor))
            {
                var left = e.Bounds.Left + this.imageList.ImageSize.Width + 2;
                e.Graphics.DrawString(line, Font, foreground, new PointF(left, e.Bounds.Top));
            }

            // Draws the icon for skill/cert on the far right
            if (supIcon != -1)
            {
                var imgOfssetX = e.Bounds.Left;
                var imgOffsetY = Math.Max(0.0f, (e.Bounds.Height - imageList.ImageSize.Height) * 0.5f);
                e.Graphics.DrawImageUnscaled(imageList.Images[supIcon],
                    (int)(imgOfssetX),
                    (int)(e.Bounds.Top + imgOffsetY));
            }
        }

        /// <summary>
        /// When the treeview is clicked, we manually select nodes since the bounding boxes are incorrect due to custom draw.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvCertList_MouseDown(object sender, MouseEventArgs e)
        {
            // Perform the selection manually since the bound's width and x are incorrect in owndraw
            TreeNode selection = null;
            for (TreeNode node = tvCertList.TopNode; node != null; node = node.NextVisibleNode)
            {
                if (node.Bounds.Top <= e.Y && node.Bounds.Bottom >= e.Y)
                {
                    // If the user clicked the "arrow zone", we do not change the selection and just return
                    if (e.X < (node.Bounds.Left - 32))
                        return;

                    selection = node;
                    break;
                }
            }

            // Updates the selection
            if (selection != tvCertList.SelectedNode)
                tvCertList.SelectedNode = selection;
        }
        #endregion

        #region Controls' handlers
        /// <summary>
        /// Event handler method for Add Certs button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnAddCerts_Click(object sender, EventArgs e)
        {
            // Add Certificates to plan
            List<StaticSkillLevel> skillsToAdd = new List<StaticSkillLevel>();
            foreach (TreeNode certificate in tvCertList.Nodes)
            {
                var cert = certificate.Tag as Certificate;
                var skills = cert.StaticData.AllTopPrerequisiteSkills;
                skillsToAdd.AddRange(skills);
            }

            var operation = m_plan.TryAddSet(skillsToAdd, m_object.Name);
            PlanHelper.Perform(operation);

            // Refresh display to reflect plan changes
            UpdateDisplay();
        }

        /// <summary>
        /// Forces the selection update when a node is right-clicked.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvCertList_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
                tvCertList.SelectedNode = e.Node;
        }

        /// <summary>
        /// Event handler for treenode double click event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tvCertList_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            // Get selected node
            TreeNode selectedNode = e.Node as TreeNode;

            // Make sure we have a skill to use
            if (selectedNode.Tag == null)
                return;

            if (selectedNode.Tag is Certificate)
            {
                PlanWindow pw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
                Certificate cert = ((Certificate)selectedNode.Tag);
                pw.ShowCertInBrowser(cert);
            }
            else
            {
                // Open skill browser tab for this skill
                PlanWindow pw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
                Skill skill = ((SkillLevel)selectedNode.Tag).Skill;
                pw.ShowSkillInBrowser(skill);
            }
        }
        #endregion

        #region Context menu
        /// <summary>
        /// Context menu opening, we update the menus' statuses.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void contextMenu_Opening(object sender, CancelEventArgs e)
        {
            if (tvCertList.SelectedNode == null)
            {
                // Update "add to" menu
                tsmAddToPlan.Enabled = false;
                tsmAddToPlan.Text = "Plan...";

                // Update "show in skill browser" menu
                showInMenuSeparator.Visible = false;
                showInExplorerMenu.Visible = false;
                showInBrowserMenu.Visible = false;
            }
            else
            {
                var cert = tvCertList.SelectedNode.Tag as Certificate;
                showInMenuSeparator.Visible = true;
                showInBrowserMenu.Visible = true;

                // When a certificate is selected
                if (cert != null)
                {
                    // Update "add to" menu
                    tsmAddToPlan.Enabled = !m_plan.WillGrantEligibilityFor(cert);
                    tsmAddToPlan.Text = String.Format(CultureConstants.DefaultCulture, "Plan \"{0}\"", cert.ToString());

                    // Update "show in..." menu
                    showInBrowserMenu.Text = "Show in Certificates";

                    // Update "show in skill explorer"
                    showInExplorerMenu.Visible = false;
                }
                // When a skill is selected
                else
                {
                    // Update "add to" menu
                    var prereq = (SkillLevel)tvCertList.SelectedNode.Tag;
                    var skill = prereq.Skill;
                    tsmAddToPlan.Enabled = skill.Level < prereq.Level && !m_plan.IsPlanned(skill, prereq.Level);
                    tsmAddToPlan.Text = String.Format(CultureConstants.DefaultCulture, "Plan \"{0} {1}\"", skill.ToString(), Skill.GetRomanForInt(prereq.Level));

                    // Update "show in..." menu
                    showInBrowserMenu.Enabled = true;
                    showInBrowserMenu.Text = "Show in Skill &Browser...";

                    // Update "show in skill explorer"
                    showInExplorerMenu.Visible = true;
                }
            }
        }

        /// <summary>
        /// Context menu > Show "skill" in browser | Show "certificate class" certificates.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void showInSkillBrowserMenu_Click(object sender, EventArgs e)
        {
            // Retrieve the owner window
            PlanWindow npw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
            if (npw == null || npw.IsDisposed)
                return;

            Certificate cert = tvCertList.SelectedNode.Tag as Certificate;
            // When a certificate is selected
            if (cert != null)
            {
                npw.ShowCertInBrowser(cert);
            }
            // When a skill is selected
            else
            {
                SkillLevel prereq = (SkillLevel)tvCertList.SelectedNode.Tag;
                npw.ShowSkillInBrowser(prereq.Skill);
            }
        }

        /// <summary>
        /// Context > Show in Skill Explorer.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void showInSkillExplorerMenu_Click(object sender, EventArgs e)
        {
            // Retrieve the owner window
            PlanWindow npw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
            if (npw == null || npw.IsDisposed)
                return;

            // Open the skill explorer
            SkillLevel prereq = (SkillLevel)tvCertList.SelectedNode.Tag;
            npw.ShowSkillInExplorer(prereq.Skill);
        }

        /// <summary>
        /// Treeview's context menu > Expand All.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmExpandAll_Click(object sender, EventArgs e)
        {
            tvCertList.ExpandAll();
        }

        /// <summary>
        /// Treeview's context menu > Collapse All.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmCollapseAll_Click(object sender, EventArgs e)
        {
            tvCertList.CollapseAll();
        }

        /// <summary>
        /// Context > Plan "(selection)"
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmAddToPlan_Click(object sender, EventArgs e)
        {
            var cert = tvCertList.SelectedNode.Tag as Certificate;
            if (cert != null)
            {
                var operation = m_plan.TryPlanTo(cert);
                PlanHelper.SelectPerform(operation);
            }
            else
            {
                var prereq = (SkillLevel)tvCertList.SelectedNode.Tag;
                var operation = m_plan.TryPlanTo(prereq.Skill, prereq.Level);
                PlanHelper.SelectPerform(operation);
            }
        }

        #endregion
    }

    #region ReqCertificatesTreeView
    /// <summary>
    /// Derived from TreeView class.
    /// <para>Overrides standard node double click behaviour to prevent node expand / collapse actions</para>
    /// </summary>
    class ReqCertificatesTreeView : System.Windows.Forms.TreeView
    {
        private const int WM_LBUTTONDBLCLK = 0x203;

        protected override void WndProc(ref Message m)
        {
            if (m.Msg == WM_LBUTTONDBLCLK)
            {
                handleDoubleClick(ref m);
            }
            else
            { 
                base.WndProc(ref m);
            }
        }

        private void handleDoubleClick(ref Message m)
        {
            // Get mouse location from message.lparam
            // x is low order word, y is high order word
            string lparam = m.LParam.ToString("X08");
            int x = int.Parse(lparam.Substring(4, 4), NumberStyles.HexNumber);
            int y = int.Parse(lparam.Substring(0, 4), NumberStyles.HexNumber);
            // Test for a treenode at this location
            TreeViewHitTestInfo info = this.HitTest(x, y);
            if (info.Node != null)
            {
                // Raise NodeMouseDoubleClick event
                TreeNodeMouseClickEventArgs e = new TreeNodeMouseClickEventArgs(info.Node, MouseButtons.Left, 2, x, y);
                this.OnNodeMouseDoubleClick(e);
            }
        }

        protected override void CreateHandle()
        {
            if (!this.IsDisposed)
                base.CreateHandle();
        }
    }
    #endregion
}