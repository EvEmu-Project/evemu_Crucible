using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Linq;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.SettingsObjects;

namespace EVEMon
{
    public partial class MarketOrdersWindow : EVEMonForm
    {
        private bool m_init;

        public MarketOrdersWindow()
        {
            InitializeComponent();
            this.RememberPositionKey = "MarketOrdersWindow";
            m_init = true;
        }

        /// <summary>
        /// Gets or sets the grouping mode.
        /// </summary>
        public Enum Grouping
        {
            get { return ordersList.Grouping; }
            set
            {
                ordersList.Grouping = value;

                if (m_init)
                {
                    ordersList.UpdateColumns();
                    ordersList.lvOrders.Visible = !ordersList.Orders.IsEmpty();
                }
            }
        }

        /// <summary>
        /// Gets or sets the showIssuedFor mode.
        /// </summary>
        public IssuedFor ShowIssuedFor
        {
            get { return ordersList.ShowIssuedFor; }
            set
            {
                ordersList.ShowIssuedFor = value;

                if (m_init)
                {
                    ordersList.UpdateColumns();
                    ordersList.lvOrders.Visible = !ordersList.Orders.IsEmpty();
                }
            }
        }

        /// <summary>
        /// Gets or sets the enumeration of orders to display.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public IEnumerable<MarketOrder> Orders
        {
            get { return ordersList.Orders; }
            set { ordersList.Orders = value; }
        }

        /// <summary>
        /// Gets or sets the enumeration of displayed columns.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public IEnumerable<MarketOrderColumnSettings> Columns
        {
            get { return ordersList.Columns; }
            set
            {
                ordersList.Columns = value;

                if (m_init)
                {
                    ordersList.UpdateColumns();
                    ordersList.lvOrders.Visible = !ordersList.Orders.IsEmpty();
                }
            }
        }
    }
}
