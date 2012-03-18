using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Drawing;

namespace EVEMon.Common.Controls
{
    /// <summary>
    /// Code based on Stelios Alexandrakis's CheckedComboBox, see :
    /// http://www.codeproject.com/KB/combobox/checkedcombobox.aspx
    /// License : http://www.codeproject.com/info/cpol10.aspx
    /// </summary>
    public abstract class CustomComboBox : ComboBox
    {
        #region CustomComboBoxEventArgs
        /// <summary>
        /// Custom EventArgs encapsulating value as to whether the combo box value(s) should be assignd to or not.
        /// </summary>
        protected class CustomComboBoxEventArgs : EventArgs
        {
            private bool validate;
            public bool Validate
            {
                get { return validate; }
                set { validate = value; }
            }

            public CustomComboBoxEventArgs(bool validate)
                : base()
            {
                this.validate = validate;
            }
        }
        #endregion


        #region Dropdown
        /// <summary>
        /// Internal class to represent the dropdown list of the CheckedComboBox
        /// </summary>
        protected class Dropdown : Form
        {
            private CustomComboBox ccbParent;
            private Control content;

            // Keeps track of whether checked item(s) changed, hence the value of the CheckedComboBox as a whole changed.
            // This is simply done via maintaining the old string-representation of the value(s) and the new one and comparing them!
            private string oldStrValue = String.Empty;
            public bool ValueChanged
            {
                get
                {
                    string newStrValue = ccbParent.Text;
                    if ((oldStrValue.Length > 0) && (newStrValue.Length > 0))
                    {
                        return (oldStrValue.CompareTo(newStrValue) != 0);
                    }
                    else
                    {
                        return (oldStrValue.Length != newStrValue.Length);
                    }
                }
            }

            // Whether the dropdown is closed.
            private bool dropdownClosed = true;

            public Dropdown(CustomComboBox ccbParent, Control content)
            {
                this.ccbParent = ccbParent;
                this.content = content;

                InitializeComponent();

                this.ShowInTaskbar = false;
            }

            // Create a CustomCheckedListBox which fills up the entire form area.
            private void InitializeComponent()
            {
                this.SuspendLayout();
                // 
                // Dropdown
                // 
                this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                this.BackColor = System.Drawing.SystemColors.Menu;
                this.ControlBox = false;
                this.Controls.Add(this.content);
                this.ForeColor = System.Drawing.SystemColors.ControlText;
                this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
                this.MinimizeBox = false;
                this.Name = "dropdown";
                this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
                this.ResumeLayout(false);
            }



            /// <summary>
            /// Closes the dropdown portion and enacts any changes according to the specified boolean parameter.
            /// NOTE: even though the caller might ask for changes to be enacted, this doesn't necessarily mean
            ///       that any changes have occurred as such. Caller should check the ValueChanged property of the
            ///       CheckedComboBox (after the dropdown has closed) to determine any actual value changes.
            /// </summary>
            /// <param name="validate"></param>
            public void CloseDropdown(bool validate)
            {
                if (dropdownClosed)
                {
                    return;
                }
                //Debug.WriteLine("CloseDropdown");
                // Cancel parent's selection before we close
                if (validate) ccbParent.SelectedIndex = -1;
                ccbParent.OnDropDownDeactivated(validate);
                // From now on the dropdown is considered closed. We set the flag here to prevent OnDeactivate() calling
                // this method once again after hiding this window.
                dropdownClosed = true;
                // Set the focus to our parent CheckedComboBox and hide the dropdown check list.
                ccbParent.Focus();
                this.Hide();
                // Notify CheckedComboBox that its dropdown is closed. (NOTE: it does not matter which parameters we pass to
                // OnDropDownClosed() as long as the argument is CCBoxEventArgs so that the method knows the notification has
                // come from our code and not from the framework).
                ccbParent.OnDropDownClosed(new CustomComboBoxEventArgs(false));
            }

            protected override void OnActivated(EventArgs e)
            {
                //Debug.WriteLine("OnActivated");
                base.OnActivated(e);
                dropdownClosed = false;
                // Assign the old string value to compare with the new value for any changes.
                oldStrValue = ccbParent.Text;
                ccbParent.OnDropDownActivated();
            }

            protected override void OnDeactivate(EventArgs e)
            {
                //Debug.WriteLine("OnDeactivate");
                base.OnDeactivate(e);
                CustomComboBoxEventArgs ce = e as CustomComboBoxEventArgs;
                if (ce != null)
                {
                    CloseDropdown(ce.Validate);
                }
                else
                {
                    // If not custom event arguments passed, means that this method was called from the
                    // framework. We assume that the checked values should be registered regardless.
                    CloseDropdown(true);
                }
            }

            public void ForceDeactivate(CustomComboBoxEventArgs e)
            {
                OnDeactivate(e);
            }
        }
        #endregion


        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        // A form-derived object representing the drop-down list of the checked combo box.
        protected Dropdown dropdown;
        private Control content;

        public bool ValueChanged
        {
            get { return dropdown.ValueChanged; }
        }

        private ToolTip toolTip;
        public ToolTip ToolTip
        {
            get { return this.toolTip; }
            set
            {
                if (value != this.toolTip)
                {
                    this.toolTip = value;
                    UpdateToolTip();
                }
            }
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public CustomComboBox()
            : base()
        {
            this.content = this.CreateContent();

            // We want to do the drawing of the dropdown.
            this.DrawMode = DrawMode.OwnerDrawVariable;
            // This prevents the actual ComboBox dropdown to show, although it's not strickly-speaking necessary.
            // But including this remove a slight flickering just before our dropdown appears (which is caused by
            // the empty-dropdown list of the ComboBox which is displayed for fractions of a second).
            this.DropDownHeight = 1;
            // This is the default setting - text portion is editable and user must click the arrow button
            // to see the list portion. Although we don't want to allow the user to edit the text portion
            // the DropDownList style is not being used because for some reason it wouldn't allow the text
            // portion to be programmatically set. Hence we set it as editable but disable keyboard input (see below).
            this.DropDownStyle = ComboBoxStyle.DropDown;
            this.dropdown = new Dropdown(this, this.content);
        }

        public abstract string GetTextValue();
        protected abstract Control CreateContent();
        protected abstract void OnDropDownActivated();
        protected abstract void OnDropDownDeactivated(bool validate);


        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        protected override void OnTextChanged(EventArgs e)
        {
            UpdateToolTip();
            base.OnTextChanged(e);
        }

        private void UpdateToolTip()
        {
            if (this.toolTip != null)
            {
                this.toolTip.SetToolTip(this, this.Text + "\n(use del and shift + del to unselect/select all)");
            }
        }


        private void DoDropDown()
        {
            if (!dropdown.Visible)
            {
                Rectangle rect = RectangleToScreen(this.ClientRectangle);
                dropdown.Location = new Point(rect.X, rect.Y + this.Size.Height);
                this.content.Refresh();
                Size preferedSize = this.content.GetPreferredSize(new Size(this.Width, 600));
                dropdown.ClientSize = new Size(Math.Max(preferedSize.Width, this.Width), Math.Max(96, preferedSize.Height));
                dropdown.Show(this);
            }
        }

        protected override void OnDropDown(EventArgs e)
        {
            base.OnDropDown(e);
            DoDropDown();
        }

        protected override void OnDropDownClosed(EventArgs e)
        {
            // Call the handlers for this event only if the call comes from our code - NOT the framework's!
            // NOTE: that is because the events were being fired in a wrong order, due to the actual dropdown list
            //       of the ComboBox which lies underneath our dropdown and gets involved every time.
            if (e is CustomComboBoxEventArgs)
            {
                base.OnDropDownClosed(e);
            }
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Down)
            {
                // Signal that the dropdown is "down". This is required so that the behaviour of the dropdown is the same
                // when it is a result of user pressing the Down_Arrow (which we handle and the framework wouldn't know that
                // the list portion is down unless we tell it so).
                // NOTE: all that so the DropDownClosed event fires correctly!                
                OnDropDown(null);
            }
            // Make sure that certain keys or combinations are not blocked.
            e.Handled = !e.Alt && !(e.KeyCode == Keys.Tab) &&
                !((e.KeyCode == Keys.Left) || (e.KeyCode == Keys.Right) || (e.KeyCode == Keys.Home) || (e.KeyCode == Keys.End));

            base.OnKeyDown(e);
        }

        protected override void OnKeyPress(KeyPressEventArgs e)
        {
            e.Handled = true;
            base.OnKeyPress(e);
        }

        protected override void OnSizeChanged(EventArgs e)
        {
            this.dropdown.MaximumSize = new Size(this.Width, 6 * this.ItemHeight);
            base.OnSizeChanged(e);
        }
    }
}
