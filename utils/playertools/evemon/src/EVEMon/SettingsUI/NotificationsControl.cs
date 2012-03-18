using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common.Notifications;

namespace EVEMon.SettingsUI
{
    public partial class NotificationsControl : UserControl
    {
        // Would have love to use tableLayoutPanel, unfortunately, they are just a piece of trash.
        public const int RowHeight = 28;


        private List<ComboBox> m_combos = new List<ComboBox>();
        private List<CheckBox> m_checkboxes = new List<CheckBox>();
        private NotificationSettings m_settings;

        /// <summary>
        /// Constructor
        /// </summary>
        public NotificationsControl()
        {
            InitializeComponent();

            // Add the controls for every member of the enumeration
            int height = RowHeight;
            var categories = Enum.GetValues(typeof(NotificationCategory))
                                            .Cast<NotificationCategory>()
                                            .Where(x=> x.HasHeader());

            foreach (var cat in categories)
            {
                // Add the label
                var label = new Label();
                label.AutoSize = false;
                label.Text = cat.GetHeader();
                label.TextAlign = ContentAlignment.MiddleLeft;
                label.Location = new Point(labelNotification.Location.X, height);
                label.Width = labelBehaviour.Location.X - 3;
                label.Height = RowHeight;
                Controls.Add(label);

                // Add the "system tray tooltip" combo box
                var combo = new ComboBox();
                combo.Tag = cat;
                combo.Items.Add("Never");
                combo.Items.Add("Once");
                combo.Items.Add("Repeat until clicked");
                combo.SelectedIndex = 0;
                combo.Margin = new Padding(3);
                combo.Height = RowHeight - 4;
                combo.Width = labelBehaviour.Width;
                combo.DropDownStyle = ComboBoxStyle.DropDownList;
                combo.Location = new Point(labelBehaviour.Location.X, height + 2);
                combo.SelectedIndexChanged += combo_SelectedIndexChanged;
                Controls.Add(combo);
                m_combos.Add(combo);

                // Add the "main window" checkbox
                var checkbox = new CheckBox();
                checkbox.Tag = cat;
                checkbox.Text = "Show";
                checkbox.Margin = new Padding(3);
                checkbox.Height = RowHeight - 4;
                checkbox.Width = labelMainWindow.Width;
                checkbox.Location = new Point(labelMainWindow.Location.X + 15, height + 2);
                checkbox.CheckedChanged += checkbox_CheckedChanged;
                Controls.Add(checkbox);
                m_checkboxes.Add(checkbox);

                // Updates the row ordinate
                height += RowHeight;
            }

            Height = height;
        }

        /// <summary>
        /// Gets or sets the settings to edit (should be a copy of the actual settings).
        /// </summary>
        [Browsable(false)]
        public NotificationSettings Settings
        {
            get { return m_settings; }
            set
            {
                m_settings = value;
                if (value == null)
                    return;

                foreach (var combo in m_combos)
                {
                    var cat = (NotificationCategory)combo.Tag;
                    int index = (int)m_settings.Categories[cat].ToolTipBehaviour;
                    
                    // TODO: Remove the following code line after deprecating ToolTipNotificationBehaviour.RepeatUntiClicked
                    if (index > 2)
                        index = 2;
                    
                    combo.SelectedIndex = index;
                }

                foreach (var checkbox in m_checkboxes)
                {
                    var cat = (NotificationCategory)checkbox.Tag;
                    checkbox.Checked = m_settings.Categories[cat].ShowOnMainWindow;
                }
            }
        }

        /// <summary>
        /// When the selected index changes, we update the settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void combo_SelectedIndexChanged(object sender, EventArgs e)
        {
            var combo = (ComboBox)sender;
            var cat = (NotificationCategory)combo.Tag;
            m_settings.Categories[cat].ToolTipBehaviour = (ToolTipNotificationBehaviour)combo.SelectedIndex;
        }

        /// <summary>
        /// When the selected checkbox check state changes, we update the settings.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        void checkbox_CheckedChanged(object sender, EventArgs e)
        {
            var checkbox = (CheckBox)sender;
            var cat = (NotificationCategory)checkbox.Tag;
            m_settings.Categories[cat].ShowOnMainWindow = checkbox.Checked;
        }
    }
}
