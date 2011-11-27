using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// UserControl to display a tree of certificates
    /// </summary>
    public partial class CertificateTreeDisplayControl : UserControl
    {
        private const int GrantedIcon = 0;
        private const int ClaimableIcon = 1;
        private const int UnknownButTrainableIcon = 2;
        private const int UnknownIcon = 3;
        private const int CertificateIcon = 4;
        private const int SkillIcon = 5;

        private Plan m_plan;
        private Character m_character;
        private CertificateClass m_class;
        private Font m_boldFont;

        private bool m_allExpanded;

        public event EventHandler SelectionChanged;

        #region Constructors

        /// <summary>
        /// Constructor
        /// </summary>
        public CertificateTreeDisplayControl()
        {
            this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            this.SetStyle(ControlStyles.OptimizedDoubleBuffer, true);

            InitializeComponent();

            treeView.DrawNode += new DrawTreeNodeEventHandler(treeView_DrawNode);
            treeView.MouseDown += new MouseEventHandler(treeView_MouseDown);

            cmListSkills.Opening += new CancelEventHandler(cmListSkills_Opening);
            m_boldFont = FontFactory.GetFont(this.Font, FontStyle.Bold);
            this.treeView.Font = FontFactory.GetFont("Microsoft Sans Serif", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
            this.treeView.ItemHeight = (treeView.Font.Height * 2) + 6;

            EveClient.CharacterChanged += new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterChanged);
            this.Disposed += new EventHandler(OnDisposed);
        }

        #endregion


        #region Events

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.CharacterChanged -= new EventHandler<CharacterChangedEventArgs>(EveClient_CharacterChanged);
            this.Disposed -= new EventHandler(OnDisposed);
        }

        /// <summary>
        /// Gets or sets the current plan.
        /// </summary>
        public Plan Plan
        {
            get { return m_plan; }
            set 
            { 
                m_plan = value;
                if (m_plan == null)
                    return;

                var character = (Character)m_plan.Character;
                if (m_character == character)
                    return;
                m_character = character;

                this.treeView.Nodes.Clear();
                UpdateTree();

            }
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets or sets the certificate class (i.e. "Core competency").
        /// </summary>
        public CertificateClass CertificateClass
        {
            get { return this.m_class; }
            set 
            {
                if (value != m_class)
                {
                    this.m_class = value;
                    UpdateTree();
                }
            }
        }

        /// <summary>
        /// Gets the selected certificate, null if none is selected. 
        /// Note that the selected certificate may be a prerequisite and therefore
        /// not have the same class than the one represented by this control.
        /// </summary>
        public Certificate SelectedCertificate
        {
            get
            {
                if (this.treeView.SelectedNode == null)
                    return null;

                return this.treeView.SelectedNode.Tag as Certificate;
            }
        }

        /// <summary>
        /// Gets cert of the displayed class which contains the current selection.
        /// </summary>
        public Certificate SelectedCertificateLevel
        {
            get
            {
                TreeNode curr = treeView.SelectedNode;
                while (curr != null)
                {
                    Certificate c = curr.Tag as Certificate;
                    if (c != null && c.Class.Equals(this.CertificateClass))
                    {
                        return c;
                    }
                    curr = curr.Parent;
                }
                return null;
            }
        }
                
        /// <summary>
        /// Expands the node representing this certificate.
        /// </summary>
        /// <param name="cert"></param>
        public void ExpandCert(Certificate cert)
        {
            foreach (TreeNode n in this.treeView.Nodes)
            {
                if (n.Tag.Equals(cert))
                {
                    this.treeView.SelectedNode = n;
                    n.Expand();
                    if (this.SelectionChanged != null)
                    {
                        this.SelectionChanged(this, new EventArgs());
                    }
                    break;
                }
            }
        }

        #endregion


        #region Event Handlers
        /// <summary>
        /// Fired when one of the character changed (skill completion, update from CCP, etc).
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void EveClient_CharacterChanged(object sender, CharacterChangedEventArgs e)
        {
            UpdateTree();
        }

        /// <summary>
        /// When the treeview is clicked, we manually select nodes since the bounding boxes are incorrect due to custom draw.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void treeView_MouseDown(object sender, MouseEventArgs e)
        {
            // Perform the selection manually since the bound's width and x are incorrect in owndraw
            TreeNode selection = null;
            for (TreeNode node = treeView.TopNode; node != null; node = node.NextVisibleNode)
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

            // Updates the selection and fire the appropriate event
            if (selection != treeView.SelectedNode)
            {
                treeView.SelectedNode = selection;
                if (this.SelectionChanged != null)
                {
                    this.SelectionChanged(this, new EventArgs());
                }
            }
        }

        /// <summary>
        /// Forces the selection update when a node is right-clicked.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void treeView_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
                treeView.SelectedNode = e.Node;
        }

        /// <summary>
        /// Event handler for treenode double click event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void treeView_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            showInBrowserMenu_Click(sender, e);
        }
        #endregion


        #region Tree building
        /// <summary>
        /// Update the whole tree
        /// </summary>
        private void UpdateTree()
        {
            Certificate oldSelection = this.SelectedCertificate;
            TreeNode newSelection = null;

            this.treeView.BeginUpdate();
            try
            {
                // Clear the old items
                this.treeView.Nodes.Clear();

                // No update when not fully initialized
                if (m_character == null || m_class == null)
                    return;

                // Create the nodes when not done, yet
                if (this.treeView.Nodes.Count == 0)
                {
                    foreach (var cert in this.m_class) 
                    {
                        var node = CreateNode(cert);
                        this.treeView.Nodes.Add(node);

                        // Does the old selection still exists ?
                        if (cert == oldSelection)
                            newSelection = node;

                    }
                }
                
                // Update the nodes
                foreach (TreeNode node in this.treeView.Nodes)
                {
                    UpdateNode(node);
                }

                // Is the old selection kept ? Then we select the matching node
                if (newSelection != null)
                {
                    this.treeView.SelectedNode = newSelection;
                }
            }
            finally
            {
                this.treeView.EndUpdate();
            }

            // Fire the SelectionChanged event if the old selection doesn't exist anymore
            if (newSelection == null)
            {
                if (this.SelectionChanged != null)
                {
                    this.SelectionChanged(this, new EventArgs());
                }
            }
        }

        /// <summary>
        /// Create a node from a prerequisite certificate
        /// </summary>
        /// <param name="cert"></param>
        /// <returns></returns>
        private TreeNode CreateNode(Certificate cert)
        {
            TreeNode node = new TreeNode();
            node.Text = cert.ToString();
            node.Tag = cert;

            foreach (var prereqCert in cert.PrerequisiteCertificates)
            {
                node.Nodes.Add(CreateNode(prereqCert));
            }

            foreach (var prereqSkill in cert.PrerequisiteSkills)
            {
                node.Nodes.Add(CreateNode(prereqSkill));
            }

            return node;
        }

        /// <summary>
        /// Create a node from a prerequisite skill
        /// </summary>
        /// <param name="skillPrereq"></param>
        /// <returns></returns>
        private TreeNode CreateNode(SkillLevel skillPrereq)
        {
            TreeNode node = new TreeNode();
            node.Text = skillPrereq.ToString();
            node.Tag = skillPrereq;

            // Add this skill's prerequisites
            foreach (var prereqSkill in skillPrereq.Skill.Prerequisites)
            {
                if (prereqSkill.Skill != skillPrereq.Skill)
                {
                    node.Nodes.Add(CreateNode(prereqSkill));
                }
            }

            return node;
        }

        /// <summary>
        /// Updates the specified node and its children
        /// </summary>
        /// <param name="node"></param>
        private void UpdateNode(TreeNode node)
        {
            Certificate cert = node.Tag as Certificate;
            ImageList noImageList = new ImageList() { ImageSize = new Size(24, 24) };

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
                var skill = m_character.Skills[skillPrereq.Skill];

                if (skillPrereq.IsKnown)
                {
                    node.ImageIndex = GrantedIcon;
                }
                else if (skill.IsKnown)
                {
                    node.ImageIndex = UnknownButTrainableIcon;
                }
                else
                {
                    node.ImageIndex = UnknownIcon;
                }
            }

            // When selected, the image remains the same
            node.SelectedImageIndex = node.ImageIndex;

            // Update the children
            foreach (TreeNode child in node.Nodes)
            {
                UpdateNode(child);
            }
        }

        /// <summary>
        /// Custom draw for the label
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void treeView_DrawNode(object sender, DrawTreeNodeEventArgs e)
        {
            // Prevents a bug that causes every item to be redrawn at the top left corner
            if (e.Bounds.Left <= 10)
            {
                e.DrawDefault = true;
                return;
            }

            string line1 = String.Empty;
            string line2 = "-";
            int supIcon = -1;

            // Is it a certificate ?
            Certificate cert = e.Node.Tag as Certificate;
            if (cert != null)
            {
                line1 = cert.ToString();
                if (!Settings.UI.SafeForWork)
                    supIcon = CertificateIcon;
                var status = cert.Status;

                // When not granted or claimable, let's display the training time
                if (status != CertificateStatus.Claimable && status != CertificateStatus.Granted)
                {
                    var time = cert.GetTrainingTime();
                    line2 = time.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);
                }
            }
            // Or a skill prerequisite ?
            else
            {
                if (!Settings.UI.SafeForWork)
                    supIcon = SkillIcon;

                var skillPrereq = (SkillLevel)e.Node.Tag;
                line1 = skillPrereq.ToString();

                // When not known to the require level, let's display the training time
                var skill = skillPrereq.Skill;
                if (!skillPrereq.IsKnown)
                {
                    var time = skill.GetLeftTrainingTimeToLevel(skillPrereq.Level);
                    line2 = time.ToDescriptiveText(DescriptiveTextOptions.IncludeCommas);
                }
            }

            // Choose colors according to selection
            bool isSelected = ((e.State & TreeNodeStates.Selected) == TreeNodeStates.Selected);
            var backColor = (isSelected ? SystemColors.Highlight : this.treeView.BackColor);
            var foreColor = (isSelected ? SystemColors.HighlightText : this.treeView.ForeColor);
            var lightForeColor = (isSelected ? SystemColors.HighlightText : SystemColors.GrayText);

            // Draws the background
            using (var background = new SolidBrush(backColor))
            {
                var width = this.treeView.ClientSize.Width - e.Bounds.Left;
                e.Graphics.FillRectangle(background, new Rectangle(e.Bounds.Left, e.Bounds.Top, width, e.Bounds.Height));
            }

            // Performs the drawing
            using (var foreground = new SolidBrush(foreColor))
            {
                var left = e.Bounds.Left + this.imageList.ImageSize.Width + 2;
                Size line1Size = TextRenderer.MeasureText(line1, m_boldFont);

                if (!String.IsNullOrEmpty(line2))
                {
                    e.Graphics.DrawString(line1, m_boldFont, foreground, new PointF(left, e.Bounds.Top));

                    using (var lightForeground = new SolidBrush(lightForeColor))
                    {
                        e.Graphics.DrawString(line2, this.Font, lightForeground, new PointF(left, e.Bounds.Top + line1Size.Height));
                    }
                }
                else
                {
                    var height = e.Graphics.MeasureString(line1, m_boldFont).Height;
                    var yOffset = 0;
                    e.Graphics.DrawString(line1, m_boldFont, foreground, new PointF(left, e.Bounds.Top + yOffset));
                }
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
        #endregion


        #region Context menus
        /// <summary>
        /// Context menu opening, we update the menus' statuses.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void cmListSkills_Opening(object sender, CancelEventArgs e)
        {
            var node = this.treeView.SelectedNode;

            if (node == null)
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
                var cert = node.Tag as Certificate;
                showInMenuSeparator.Visible = true;
                showInBrowserMenu.Visible = true;

                // When a certificate is selected
                if (cert != null)
                {
                    // Update "add to" menu
                    tsmAddToPlan.Enabled = !m_plan.WillGrantEligibilityFor(cert);
                    tsmAddToPlan.Text = "Plan \"" + cert.ToString() + "\"";

                    // Update "browser" menu
                    showInBrowserMenu.Enabled = m_class != cert.Class;
                    showInBrowserMenu.Text = "Show Certificates";

                    // Update "show in skill explorer" menu
                    showInExplorerMenu.Visible = false;
                }
                // When a skill is selected
                else
                {
                    // Update "add to" menu
                    var prereq = (SkillLevel)node.Tag;
                    var skill = prereq.Skill;
                    tsmAddToPlan.Enabled = skill.Level < prereq.Level && !m_plan.IsPlanned(skill, prereq.Level);
                    tsmAddToPlan.Text = "Plan \"" + skill.ToString() + " " + Skill.GetRomanForInt(prereq.Level) + "\"";

                    // Update "show in skill browser" menu
                    showInBrowserMenu.Enabled = true;
                    showInBrowserMenu.Text = "Show in Skill Browser...";

                    // Update "show in skill explorer" menu
                    showInExplorerMenu.Visible = true;
                }
            }

            tsSeparatorToggle.Visible = (node != null && node.GetNodeCount(true) > 0);

            // "Collapse" and "Expand" menus
            tsmCollapseSelected.Visible = (node != null && node.GetNodeCount(true) > 0 && node.IsExpanded);
            tsmExpandSelected.Visible = (node != null && node.GetNodeCount(true) > 0 && !node.IsExpanded);

            tsmExpandSelected.Text = (node != null && node.GetNodeCount(true) > 0 && !node.IsExpanded ?
                String.Format("Expand {0}", node.Text) : String.Empty);
            tsmCollapseSelected.Text = (node != null && node.GetNodeCount(true) > 0 && node.IsExpanded ?
                String.Format("Collapse {0}", node.Text) : String.Empty);

            // "Expand All" and "Collapse All" menus
            tsmCollapseAll.Enabled = tsmCollapseAll.Visible = m_allExpanded;
            tsmExpandAll.Enabled = tsmExpandAll.Visible = !tsmCollapseAll.Enabled;

        }

        /// <summary>
        /// Treeview's context menu > Plan "(selection)"
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmAddToPlan_Click(object sender, EventArgs e)
        {
            var cert = this.treeView.SelectedNode.Tag as Certificate;
            if (cert != null)
            {
                var operation = m_plan.TryPlanTo(cert);
                PlanHelper.SelectPerform(operation);
            }
            else
            {
                var prereq = (SkillLevel)this.treeView.SelectedNode.Tag;
                var operation = m_plan.TryPlanTo(prereq.Skill, prereq.Level);
                PlanHelper.SelectPerform(operation);
            }
        }

        /// <summary>
        /// Treeview's context menu > Expand
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmExpandSelected_Click(object sender, EventArgs e)
        {
            treeView.SelectedNode.Expand();
        }

        /// <summary>
        /// Treeview's context menu > Collapse
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmCollapseSelected_Click(object sender, EventArgs e)
        {
            treeView.SelectedNode.Collapse();
        }

        /// <summary>
        /// Treeview's context menu > Expand All
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmExpandAll_Click(object sender, EventArgs e)
        {
            this.treeView.ExpandAll();
            m_allExpanded = true;
        }

        /// <summary>
        /// Treeview's context menu > Collapse All
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tsmCollapseAll_Click(object sender, EventArgs e)
        {
            this.treeView.CollapseAll();
            m_allExpanded = false;
        }

        /// <summary>
        /// Context menu > Show "skill" in browser | Show "certificate class" certificates.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void showInBrowserMenu_Click(object sender, EventArgs e)
        {
            // Retrieve the owner window
            PlanWindow npw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
            if (npw == null || npw.IsDisposed)
                return;

            // Return when nothing is selected
            if (this.treeView.SelectedNode == null)
                return;

            Certificate cert = this.treeView.SelectedNode.Tag as Certificate;
            // When a certificate is selected, we select its class in the left tree
            if (cert != null)
            {
                npw.ShowCertInBrowser(cert);
            }
            // When a skill is selected, we select it in the skill browser
            else
            {
                SkillLevel prereq = (SkillLevel)this.treeView.SelectedNode.Tag;
                npw.ShowSkillInBrowser(prereq.Skill);
            }
        }

        /// <summary>
        /// Context menu > Show "skill" in explorer.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void showInExplorerMenu_Click(object sender, EventArgs e)
        {
            // Retrieve the owner window
            PlanWindow npw = WindowsFactory<PlanWindow>.GetByTag(m_plan);
            if (npw == null || npw.IsDisposed)
                return;

            // Return when nothing is selected
            if (this.treeView.SelectedNode == null)
                return;

            // Open the skill explorer
            SkillLevel prereq = (SkillLevel)this.treeView.SelectedNode.Tag;
            npw.ShowSkillInExplorer(prereq.Skill);
        }
        #endregion
    }
}