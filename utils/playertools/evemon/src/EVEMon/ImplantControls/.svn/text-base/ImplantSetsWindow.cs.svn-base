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
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.Data;
using EVEMon.Controls;
using System.Diagnostics;

namespace EVEMon.ImplantControls
{
    /// <summary>
    /// This is the implant groups UI
    /// </summary>
    public partial class ImplantSetsWindow : EVEMonForm
    {
        private const string PhantomSetName = "<New set>";

        private int m_maxJumpClones;
        private Character m_character;
        private SerializableImplantSetCollection m_sets = null;

        /// <summary>
        /// Labels are substituted to comboboxes for read-only sets. It's because comboboxes cannot be readonly, only disabled. 
        /// But, then, they don't fire mouse events.
        /// </summary>
        private Label[] m_labels = new Label[10];

        /// <summary>
        /// Default constructor for designer
        /// </summary>
        public ImplantSetsWindow()
        {
            InitializeComponent();
            this.RememberPositionKey = "ImplantSetsWindow";
        }

        /// <summary>
        /// Constructor used in code.
        /// </summary>
        /// <param name="character"></param>
        public ImplantSetsWindow(Character character)
            : this()
        {
            m_character = character;
            m_sets = character.ImplantSets.Export();
            m_maxJumpClones = character.Skills["Infomorph Psychology"].Level;
        }

        /// <summary>
        /// On load, fill up the controls.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ImpGroups_Load(object sender, EventArgs e)
        {
            // Header
            headerLabel.Text = String.Format(CultureConstants.DefaultCulture, "{0} has the skill for {1:D} Jump Clones\n(plus 1 for the implants in your active body)",
                          m_character, m_maxJumpClones);

            // Populate implants combo boxes
            foreach (var control in this.Controls)
            {
                var combo = control as DropDownMouseMoveComboBox;
                if (combo == null) continue;

                int slotIndex = Int32.Parse(combo.Name.Replace("cbSlot", String.Empty)) - 1;
                var slot = (ImplantSlots)slotIndex;

                combo.Tag = (object)slot;
                combo.MouseMove += new MouseEventHandler(combo_MouseMove);
                combo.DropDownClosed += new EventHandler(combo_DropDownClosed);
                combo.DropDownMouseMove += new DropDownMouseMoveHandler(combo_DropDownMouseMove);
                foreach (var implant in StaticItems.GetImplants(slot))
                {
                    combo.Items.Add(implant);
                }

                var label = new Label();
                label.AutoSize = false;
                label.Anchor = combo.Anchor;
                label.Bounds = combo.Bounds;
                label.TextAlign = ContentAlignment.MiddleLeft;
                label.MouseMove += new MouseEventHandler(label_MouseMove);
                m_labels[(int)slot] = label;
            }

            // Adds the labels
            for (int i = 0; i < 10; i++)
            {
                this.Controls.Add(m_labels[i]);
            }

            // Sets the grid rows
            setsGrid.Rows.Clear();
            AddRow(m_sets.API);
            AddRow(m_sets.OldAPI);
            foreach (var set in m_sets.CustomSets) AddRow(set);
            setsGrid.Rows[0].ReadOnly = true;
            setsGrid.Rows[1].ReadOnly = true;
            setsGrid.Rows[0].Cells[0].Style.ForeColor = SystemColors.GrayText;
            setsGrid.Rows[1].Cells[0].Style.ForeColor = SystemColors.GrayText;
            setsGrid.Rows[0].Selected = true;

            // Update the phantom row, for insertion by the user
            var phantomRow = setsGrid.Rows[setsGrid.Rows.Count - 1];

            // Update the texts
            UpdateSlots();

            // Subscribe events
            this.setsGrid.AllowUserToDeleteRows = false;
            this.setsGrid.CellValidating += new DataGridViewCellValidatingEventHandler(this.setsGrid_CellValidating);
            this.setsGrid.RowsRemoved += new DataGridViewRowsRemovedEventHandler(this.setsGrid_RowsRemoved);
            this.setsGrid.SelectionChanged += new EventHandler(this.setsGrid_SelectionChanged);
            this.setsGrid.CellBeginEdit += new DataGridViewCellCancelEventHandler(setsGrid_CellBeginEdit);
            this.Disposed += new EventHandler(OnDisposed);
        }

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void OnDisposed(Object sender, EventArgs e)
        {
            this.setsGrid.CellValidating -= new DataGridViewCellValidatingEventHandler(this.setsGrid_CellValidating);
            this.setsGrid.RowsRemoved -= new DataGridViewRowsRemovedEventHandler(this.setsGrid_RowsRemoved);
            this.setsGrid.SelectionChanged -= new EventHandler(this.setsGrid_SelectionChanged);
            this.setsGrid.CellBeginEdit -= new DataGridViewCellCancelEventHandler(setsGrid_CellBeginEdit);
            this.Disposed -= new EventHandler(OnDisposed);
        }

        /// <summary>
        /// On close, closes the tooltip.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosed(EventArgs e)
        {
            m_fakeToolTip.Close();
            m_fakeToolTip = null;
            base.OnClosed(e);
        }

        /// <summary>
        /// Adds the row for the given set.
        /// </summary>
        /// <param name="set"></param>
        private void AddRow(SerializableSettingsImplantSet set)
        {
            var row = new DataGridViewRow();
            row.CreateCells(setsGrid, set.Name);
            row.Tag = set;
            setsGrid.Rows.Add(row);
        }

        /// <summary>
        /// Update the comboboxes' selections.
        /// </summary>
        private void UpdateSlots()
        {
            var set = GetSelectedSet();

            // No set selected or row name empty?
            if (set == null || String.IsNullOrEmpty(set.Name.ToString()))
            {
                foreach (var control in this.Controls)
                {
                    DropDownMouseMoveComboBox combo = control as DropDownMouseMoveComboBox;
                    if (combo == null) continue;

                    // Disable the combo with the <None> implant
                    combo.SelectedIndex = 0;
                    combo.Visible = true;
                    combo.Enabled = false;

                    // Hide the label used for read-only sets
                    var slot = (ImplantSlots)combo.Tag;
                    var label = m_labels[(int)slot];
                    label.Visible = false;
                }
                return;
            }

            // Scroll through comboboxes
            bool isReadOnly = (set == m_sets.API || set == m_sets.OldAPI);
            foreach (var control in this.Controls)
            {
                // Only look for combo boxes
                DropDownMouseMoveComboBox combo = control as DropDownMouseMoveComboBox;
                if (combo == null) continue;

                // Enable the combo with the <None> implant
                combo.SelectedIndex = 0;
                combo.Visible = !isReadOnly;
                combo.Enabled = true;

                var slot = (ImplantSlots)combo.Tag;
                var selectedImplant = GetImplant(set, slot);

                // Scroll through every implant and check whether it is the selected one.
                int index = 0;
                foreach (Implant implant in combo.Items)
                {
                    if (implant == selectedImplant)
                    {
                        combo.SelectedIndex = index;
                        break;
                    }
                    index++;
                }

                // Set "none" when the implant was not found.
                if (index == combo.Items.Count) combo.SelectedIndex = 0;

                // Updates the label displayed for read-only sets.
                var label = m_labels[(int)slot];
                label.Visible = isReadOnly;
                label.Text = selectedImplant.Name;
                label.Tag = selectedImplant;

            }
        }

        #region Helper getters and setters
        /// <summary>
        /// Gets the selected implant slot
        /// </summary>
        private SerializableSettingsImplantSet GetSelectedSet()
        {
            if (setsGrid.SelectedRows.Count == 0) return null;
            return (SerializableSettingsImplantSet)setsGrid.SelectedRows[0].Tag;
        }

        /// <summary>
        /// Gets the implant name for the given slot and the provided set.
        /// </summary>
        /// <param name="set"></param>
        /// <param name="slot"></param>
        /// <returns></returns>
        private Implant GetImplant(SerializableSettingsImplantSet set, ImplantSlots slot)
        {
            // Invoke the property getter with the matching name through reflection
            var implantName = typeof(SerializableSettingsImplantSet).GetProperty(slot.ToString()).GetValue(set, null);

            return StaticItems.GetImplants(slot)[(string)implantName] as Implant;
        }

        /// <summary>
        /// Sets the implant name for the given slot and the provided set.
        /// </summary>
        /// <param name="set"></param>
        /// <param name="slot"></param>
        /// <param name="implantName"></param>
        /// <returns></returns>
        private void SetImplant(SerializableSettingsImplantSet set, ImplantSlots slot, Implant implant)
        {
            // Set may be null when the user is editing the phantom line
            if (set == null) return;

            // Invoke the property setter with the matching name through reflection
            typeof(SerializableSettingsImplantSet).GetProperty(slot.ToString()).SetValue(set, implant.Name, null);
        }
        #endregion


        #region Reaction to controls events
        /// <summary>
        /// Ensures we display &lt;New set&gt; when we begin a new row.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void setsGrid_CellBeginEdit(object sender, DataGridViewCellCancelEventArgs e)
        {
            // If cell is empty, replaces the content by <New set>
            var row = setsGrid.Rows[e.RowIndex];
            if (row.Tag == null && String.IsNullOrEmpty(row.Cells[0].FormattedValue.ToString()))
            {
                row.Cells[0].Value = PhantomSetName;
            }
        }

        /// <summary>
        /// When the selection changes, we update the controls.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void setsGrid_SelectionChanged(object sender, EventArgs e)
        {
            // Enable/disable the top buttons
            if (setsGrid.SelectedRows.Count == 0)
            {
                importButton.Enabled = false;
            }
            else
            {
                importButton.Enabled = true;
                setsGrid.AllowUserToDeleteRows = (setsGrid.SelectedRows[0].Index >= 2);
            }
            UpdateSlots();
        }

        /// <summary>
        /// When a cell value changes, we replace the new name.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void setsGrid_CellValidating(object sender, DataGridViewCellValidatingEventArgs e)
        {
            var row = setsGrid.Rows[e.RowIndex];
            var text = (e.FormattedValue == null ? String.Empty : e.FormattedValue.ToString());

            // If the user forgets the edition and there is no bound set, we replace <New set> by an empty value
            if (row.Tag == null && text == PhantomSetName)
            {
                row.Cells[0].Value = String.Empty;
                return;
            }

            // Updates the set's name
            EnsureRowSetInitialized(row);
            if (row.Tag != null)
            {
                var set = (SerializableSettingsImplantSet)row.Tag;
                set.Name = e.FormattedValue.ToString();
            }
        }

        /// <summary>
        /// When a row is removed, remove the matching set.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void setsGrid_RowsRemoved(object sender, DataGridViewRowsRemovedEventArgs e)
        {
            int index = e.RowIndex - 2;
            if (index >= 0) m_sets.CustomSets.RemoveAt(index);
        }

        /// <summary>
        /// When the selection changes, we change the implant
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbSlotN_DropDownClosed(object sender, EventArgs e)
        {
            if (setsGrid.SelectedRows.Count != 0)
            {
                EnsureRowSetInitialized(setsGrid.SelectedRows[0]);
            }

            var combo = (DropDownMouseMoveComboBox)sender;
            var slot = (ImplantSlots)combo.Tag;
            SetImplant(GetSelectedSet(), slot, (Implant)combo.SelectedItem);
        }

        /// <summary>
        /// On cancel, nothing special.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnCancel_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            this.Close();
        }

        /// <summary>
        /// On OK, let's fetch the serialization object to the real implant sets.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnOK_Click(object sender, EventArgs e)
        {
            var set = GetSelectedSet();
            if (set != null && !String.IsNullOrEmpty(set.Name.ToString()))
                m_character.ImplantSets.Import(m_sets);
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        /// <summary>
        /// On import button, we replace the implants of the set with the ones from the implants preivously queried from the API.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void importButton_Click(object sender, EventArgs e)
        {
            var set = GetSelectedSet();
            if (set == null) return;
            set.Intelligence = m_sets.OldAPI.Intelligence;
            set.Perception = m_sets.OldAPI.Perception;
            set.Willpower = m_sets.OldAPI.Willpower;
            set.Charisma = m_sets.OldAPI.Charisma;
            set.Memory = m_sets.OldAPI.Memory;
            set.Slot6 = m_sets.OldAPI.Slot6;
            set.Slot7 = m_sets.OldAPI.Slot7;
            set.Slot8 = m_sets.OldAPI.Slot8;
            set.Slot9 = m_sets.OldAPI.Slot9;
            set.Slot10 = m_sets.OldAPI.Slot10;
            UpdateSlots();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="row"></param>
        private void EnsureRowSetInitialized(DataGridViewRow row)
        {
            if (row.Tag == null)
            {
                row.Tag = new SerializableSettingsImplantSet { Name = row.Cells[0].FormattedValue.ToString() };
                m_sets.CustomSets.Add((SerializableSettingsImplantSet)row.Tag);
            }
        }
        #endregion


        #region Tooltips management
        private Implant m_lastImplant;
        private ImplantTooltip m_fakeToolTip = new ImplantTooltip();

        /// <summary>
        /// When the combo box's dropdown is closed, we hide the implant.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void combo_DropDownClosed(object sender, EventArgs e)
        {
            m_fakeToolTip.Hide();
            m_lastImplant = null;
        }

        /// <summary>
        /// When the mouse moves over the implants labels (used for read-only sets), we display a tooltip.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void label_MouseMove(object sender, MouseEventArgs e)
        {
            var label = (Label)sender;
            var implant = label.Tag as Implant;
            if (implant == null) return;

            Point point = e.Location;
            point.Y += 10;

            m_fakeToolTip.Location = label.PointToScreen(point);
            m_fakeToolTip.Implant = implant;
            m_fakeToolTip.ShowInactiveTopmost();
        }

        /// <summary>
        /// When the mouse moves over the implants combos (used for writable sets), we display a tooltip on the right of the combo.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void combo_MouseMove(object sender, MouseEventArgs e)
        {
            var combo = (DropDownMouseMoveComboBox)sender;
            var implant = combo.SelectedItem as Implant;
            if (implant == null) return;

            if (m_fakeToolTip.Visible && m_fakeToolTip.Implant == implant) return;

            Point point = new Point();
            point.X = combo.Width + 5;
            point.Y = 1;

            m_fakeToolTip.Location = combo.PointToScreen(point);
            m_fakeToolTip.Implant = implant;
            m_fakeToolTip.ShowInactiveTopmost();
            m_lastImplant = implant;
        }

        /// <summary>
        /// When the mouse moves over one of the items of the combobox dropdown, we display a tooltip on the right of the dropdown.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="item"></param>
        /// <param name="point"></param>
        void combo_DropDownMouseMove(object sender, object item, Point point)
        {
            var implant = (Implant)item;
            //if (toolTip.Active && m_lastImplant == implant) return;

            var control = (Control)sender;
            point.X = control.ClientRectangle.Right + 20;
            point.Y = control.ClientRectangle.Top;

            m_fakeToolTip.Location = control.PointToScreen(point);
            m_fakeToolTip.Implant = implant;
            m_fakeToolTip.ShowInactiveTopmost();
            m_lastImplant = implant;
        }

        /// <summary>
        /// When the mouse moves over this window, then it left the comboboxes and labels, so we hide the tooltip.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseMove(MouseEventArgs e)
        {
            m_fakeToolTip.Hide();
            base.OnMouseMove(e);
        }
        #endregion
    }
}