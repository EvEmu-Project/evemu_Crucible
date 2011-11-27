using System;
using System.Linq;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common;
using EVEMon.Common.Attributes;

using CommonProperties = EVEMon.Common.Properties;

namespace EVEMon.SettingsUI
{
    public partial class UpdateSettingsControl : UserControl
    {
        // Would have love to use tableLayoutPanel, unfortunately, they are just a piece of trash.
        public const int RowHeight = 28;

        private List<ComboBox> m_combos = new List<ComboBox>();
        private UpdateSettings m_settings;

        public UpdateSettingsControl()
        {
            InitializeComponent();

            // Add the controls for every member of the enumeration
            int height = RowHeight;
            var methods = Enum.GetValues(typeof(APIMethods)).Cast<APIMethods>();
            foreach (var method in methods)
            {
                // Skip if there is no header
                if (!method.HasHeader())
                    continue;

                // Add the icon
                var icon = CommonProperties.Resources.APIKeyLimited16;
                var iconToolTip = "This query requires a limited API key.";
                if (method.HasAttribute<FullKeyAttribute>())
                {
                    icon = CommonProperties.Resources.APIKeyFull16;
                    iconToolTip = "This query requires a full API key.";
                }

                var picture = new PictureBox();
                picture.Image = icon;
                picture.Size = icon.Size;
                picture.Location = new Point(0, height + (RowHeight - icon.Size.Height) / 2);
                toolTip.SetToolTip(picture, iconToolTip);
                Controls.Add(picture);

                // Add the label
                var label = new Label();
                label.AutoSize = false;
                label.Text = method.GetHeader();
                label.TextAlign = ContentAlignment.MiddleLeft;
                label.Location = new Point(labelMethod.Location.X, height);
                label.Width = labelMethod.Width;
                label.Height = RowHeight;
                toolTip.SetToolTip(label, method.GetDescription());
                Controls.Add(label);

                // Add the "system tray tooltip" combo box
                var combo = new ComboBox();
                combo.Tag = method;

                foreach (var period in GetUpdatePeriods(method))
                {
                    var header = period.GetHeader();
                    if (period == UpdatePeriod.Never && method.HasAttribute<ForcedOnStartupAttribute>())
                        header = "On Startup";

                    combo.Items.Add(header);
                }

                combo.SelectedIndex = 0;
                combo.Margin = new Padding(3);
                combo.Height = RowHeight - 4;
                combo.Width = labelPeriod.Width;
                combo.DropDownStyle = ComboBoxStyle.DropDownList;
                combo.Location = new Point(labelPeriod.Location.X, height + 2);
                combo.SelectedIndexChanged += combo_SelectedIndexChanged;
                Controls.Add(combo);
                m_combos.Add(combo);

                // Updates the row ordinate
                height += RowHeight;
            }

            this.Height = height;
        }


        /// <summary>
        /// Gets or sets the settings to edit (should be a copy of the actual settings).
        /// </summary>
        [Browsable(false)]
        public UpdateSettings Settings
        {
            get { return m_settings; }
            set
            {
                m_settings = value;
                if (value == null)
                    return;

                foreach (var combo in m_combos)
                {
                    var method = (APIMethods)combo.Tag;
                    var periods = GetUpdatePeriods(method);
                    combo.SelectedIndex = Math.Max(0, periods.IndexOf(m_settings.Periods[method]));
                }
            }
        }

        /// <summary>
        /// When the selected indew changes, we update the settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void combo_SelectedIndexChanged(object sender, EventArgs e)
        {
            var combo = (ComboBox)sender;
            var method = (APIMethods)combo.Tag;
            var periods = GetUpdatePeriods(method);

            if (combo.SelectedIndex < 0 || combo.SelectedIndex >= periods.Count)
                return;

            if (method == APIMethods.MarketOrders)
                m_settings.Periods[APIMethods.CorporationMarketOrders] = periods[combo.SelectedIndex];

            if (method == APIMethods.IndustryJobs)
                m_settings.Periods[APIMethods.CorporationIndustryJobs] = periods[combo.SelectedIndex];

            m_settings.Periods[method] = periods[combo.SelectedIndex];
        }

        /// <summary>
        /// Gets the available update periods for the given method.
        /// </summary>
        /// <param name="method"></param>
        /// <returns></returns>
        private List<UpdatePeriod> GetUpdatePeriods(APIMethods method)
        {
            List<UpdatePeriod> periods = new List<UpdatePeriod>();
            periods.Add(UpdatePeriod.Never);

            var updateAttribute = method.GetAttribute<UpdateAttribute>();
            int min = (int)updateAttribute.Minimum;
            int max = (int)updateAttribute.Maximum;

            foreach (UpdatePeriod period in Enum.GetValues(typeof(UpdatePeriod)))
            {
                if (period != UpdatePeriod.Never)
                {
                    int index = (int)period;
                    if (index >= min && index <= max)
                        periods.Add(period);
                }
            }

            return periods;
        }

    }
}
