using System;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.ComponentModel;
using System.Drawing.Design;

namespace EVEMon.Common.Controls
{
    /// <summary>
    /// Code based on Stelios Alexandrakis's CheckedComboBox, see :
    /// http://www.codeproject.com/KB/combobox/checkedcombobox.aspx
    /// License : http://www.codeproject.com/info/cpol10.aspx
    /// </summary>
    public class CheckedComboBox : CustomComboBox
    {
        #region CustomCheckedListBox
        /// <summary>
        /// A custom CheckedListBox being shown within the dropdown form representing the dropdown list of the CheckedComboBox.
        /// </summary>
        internal class CustomCheckedListBox : CheckedListBox
        {
            private int m_curSelIndex = -1;

            public CustomCheckedListBox()
            {
                SelectionMode = SelectionMode.One;
                HorizontalScrollbar = true;
            }

            /// <summary>
            /// Intercepts the keyboard input, [Enter] confirms a selection and [Esc] cancels it.
            /// </summary>
            /// <param name="e">The Key event arguments</param>
            protected override void OnKeyDown(KeyEventArgs e)
            {
                if (e.KeyCode == Keys.Enter || e.KeyCode == Keys.Escape)
                {
                    // Enact selection.
                    ((Dropdown)Parent).ForceDeactivate(new CustomComboBoxEventArgs(true));
                    e.Handled = true;

                }
                else if (e.KeyCode == Keys.Delete)
                {
                    // Delete unckecks all, [Shift + Delete] checks all.
                    for (int i = 0; i < Items.Count; i++)
                    {
                        SetItemChecked(i, e.Shift);
                    }
                    e.Handled = true;
                }
                // If no Enter or Esc keys presses, let the base class handle it.
                base.OnKeyDown(e);
            }

            /// <summary>
            /// When the mouse moves, we update the selection.
            /// </summary>
            /// <param name="e"></param>
            protected override void OnMouseMove(MouseEventArgs e)
            {
                base.OnMouseMove(e);
                int index = IndexFromPoint(e.Location);
                //Debug.WriteLine("Mouse over item: " + (index >= 0 ? GetItemText(Items[index]) : "None"));
                if ((index >= 0) && (index != m_curSelIndex))
                {
                    m_curSelIndex = index;
                    SetSelected(index, true);
                }
            }

        }
        #endregion



        // The content of the popup
        private CustomCheckedListBox listBox;

        /// <summary>
        /// The valueSeparator character(s) between the ticked elements as they appear in the text portion of the CheckedComboBox.
        /// </summary> 
        public string ValueSeparator { get; set; }

        public bool CheckOnClick
        {
            get { return listBox.CheckOnClick; }
            set { listBox.CheckOnClick = value; }
        }

        public new string DisplayMember
        {
            get { return listBox.DisplayMember; }
            set { listBox.DisplayMember = value; }
        }

        public delegate string CheckedComboBoxTextBuilderDelegate(CheckedComboBox box);
        public CheckedComboBoxTextBuilderDelegate CustomTextBuilder { get; set; }

        [DesignerSerializationVisibility(DesignerSerializationVisibility.Visible)]
        public string TextForAll { get; set; }

        [DesignerSerializationVisibility(DesignerSerializationVisibility.Visible)]
        public string TextForNone { get; set; }


        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        public new CheckedListBox.ObjectCollection Items
        {
            get { return listBox.Items; }
        }

        public CheckedListBox.CheckedItemCollection CheckedItems
        {
            get { return listBox.CheckedItems; }
        }

        public CheckedListBox.CheckedIndexCollection CheckedIndices
        {
            get { return listBox.CheckedIndices; }
        }

        // Array holding the checked states of the items. This will be used to reverse any changes if user cancels selection.
        private bool[] oldStates;
        private string displayText;

        // Event handler for when an item check state changes.
        public event ItemCheckEventHandler ItemCheck;

        /// <summary>
        /// Constructor
        /// </summary>
        public CheckedComboBox()
        {
            // Default value separator.
            TextForAll = "All";
            TextForNone = "None";
            ValueSeparator = ", ";
            Cursor = Cursors.Default;
            DropDownStyle = ComboBoxStyle.DropDownList;
            DrawMode = DrawMode.OwnerDrawFixed;
            DrawItem += CheckedComboBox_DrawItem;

            // CheckOnClick style for the dropdown (NOTE: must be set after dropdown is created).
            CheckOnClick = true;
            listBox.ItemCheck += listBox_ItemCheck;
            listBox.IntegralHeight = false;
        }

        /// <summary>
        /// Create the popup's content
        /// </summary>
        /// <returns>The control to add to the popup</returns>
        protected override Control CreateContent()
        {
            listBox = new CustomCheckedListBox();
            listBox.BorderStyle = BorderStyle.None;
            listBox.Dock = DockStyle.Fill;
            listBox.FormattingEnabled = true;
            listBox.Location = new Point(0, 0);
            listBox.Name = "listBox";
            listBox.Size = new Size(47, 15);
            listBox.TabIndex = 0;
            return listBox;
        }

        public override string GetTextValue()
        {
            if (listBox.CheckedItems.Count == 0) return TextForNone;
            if (listBox.CheckedItems.Count == listBox.Items.Count) return TextForAll;

            if (CustomTextBuilder != null)
            {
                return CustomTextBuilder(this);
            }
            else
            {
                StringBuilder sb = new StringBuilder("");
                for (int i = 0; i < listBox.CheckedItems.Count; i++)
                {
                    if (i != 0) sb.Append(ValueSeparator);
                    sb.Append(listBox.GetItemText(listBox.CheckedItems[i]));
                }
                return sb.ToString();
            }
        }

        protected override void OnDropDownDeactivated(bool validate)
        {
            // Perform the actual selection and display of checked items.
            if (!validate)
            {
                // Caller cancelled selection - need to restore the checked items to their original state.
                for (int i = 0; i < listBox.Items.Count; i++)
                {
                    SetItemChecked(i, oldStates[i]);
                }
            }
            // Set the text portion equal to the string comprising all checked items (if any, otherwise empty!).
            Text = GetTextValue();
        }

        protected override void OnDropDownActivated()
        {
            // Make a copy of the checked state of each item, in cace caller cancels selection.
            oldStates = new bool[Items.Count];
            for (int i = 0; i < Items.Count; i++)
            {
                oldStates[i] = GetItemChecked(i);
            }

            FitDropDownToContent();
        }

        private void FitDropDownToContent()
        {
            Size preferedSize = listBox.GetPreferredSize(new Size(Width, 600));
            int width = Math.Max(preferedSize.Width, Width);
            int height = Math.Max(96, preferedSize.Height);
            dropdown.MaximumSize = new Size(Width, height + 20);
            dropdown.ClientSize = new Size(width, height);
            listBox.Refresh();
        }

        public bool GetItemChecked(int index)
        {
            if (index < 0 || index > Items.Count)
            {
                throw new ArgumentOutOfRangeException("index", "value out of range");
            }
            else
            {
                return listBox.GetItemChecked(index);
            }
        }

        public void SetItemChecked(int index, bool isChecked)
        {
            if (index < 0 || index > Items.Count)
            {
                throw new ArgumentOutOfRangeException("index", "value out of range");
            }
            else
            {
                listBox.SetItemChecked(index, isChecked);
                // Need to update the Text.
                Text = GetTextValue();
            }
        }

        public CheckState GetItemCheckState(int index)
        {
            if (index < 0 || index > Items.Count)
            {
                throw new ArgumentOutOfRangeException("index", "value out of range");
            }
            else
            {
                return listBox.GetItemCheckState(index);
            }
        }

        public void SetItemCheckState(int index, CheckState state)
        {
            if (index < 0 || index > Items.Count)
            {
                throw new ArgumentOutOfRangeException("index", "value out of range");
            }
            else
            {
                listBox.SetItemCheckState(index, state);
                // Need to update the Text.
                Text = GetTextValue();
            }
        }

        private bool manuallyFired;
        private void listBox_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            if (!manuallyFired)
            {
                // Force the update of the checkeditems
                try
                {
                    manuallyFired = true;
                    SetItemCheckState(e.Index, e.NewValue);
                }
                finally
                {
                    manuallyFired = false;
                }

                // Update the combobox's text
                displayText = GetTextValue();
                Invalidate();

                if (ItemCheck != null)
                {
                    ItemCheck(sender, e);
                }
            }
        }

        /// <summary>
        /// Draws the item that appears on the textbox
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void CheckedComboBox_DrawItem(object sender, DrawItemEventArgs e)
        {
            using (Brush backBrush = new SolidBrush(BackColor))
            {
                e.Graphics.FillRectangle(backBrush, e.Bounds);
            }

            if (displayText != null)
            {
                using (Brush foreBrush = new SolidBrush(ForeColor))
                {
                    const float offset = 3.0f;
                    var size = e.Graphics.MeasureString(displayText, Font);
                    var rect = new RectangleF(offset, (Bounds.Height - size.Height) * 0.5f, e.Bounds.Width - offset, size.Height);
                    e.Graphics.DrawString(displayText, Font, foreBrush, rect, StringFormat.GenericTypographic);
                }
            }

            e.DrawFocusRectangle();
        }
    }

}
