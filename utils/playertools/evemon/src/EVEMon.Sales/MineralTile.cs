using System;
using System.Drawing;
using System.IO;
using System.Reflection;
using System.Windows.Forms;
using EVEMon.Common;

namespace EVEMon.Sales
{
    [Serializable]
    public partial class MineralTile : UserControl
    {
        public event EventHandler<EventArgs> SubtotalChanged;
        public event EventHandler<EventArgs> MineralPriceChanged;

        public MineralTile()
        {
            InitializeComponent();
        }

        private string m_mineralName;

        public String MineralName
        {
            get { return m_mineralName; }
            set
            {
                m_mineralName = value;
                groupBox1.Text = value;
                Stream s = null;
                Image i = null;
                try
                {
                    Assembly asm = Assembly.GetExecutingAssembly();
                    s = asm.GetManifestResourceStream("EVEMon.Sales.icons." + value + ".png");
                    i = Image.FromStream(s, true, true);
                    icon.Image = i;
                }
                catch (Exception e)
                {
                    ExceptionHandler.LogException(e, true);
                    if (i != null)
                        i.Dispose();

                    if (s != null)
                        s.Dispose();

                    icon.Image = null;
                }
            }
        }

        public int Quantity
        {
            get { return Int32.Parse(txtStock.Text); }
            set { txtStock.Text = value.ToString(); }
        }

        public Decimal PricePerUnit
        {
            get { return Decimal.Parse(txtLastSell.Text); }
            set { txtLastSell.Text = value.ToString("N"); }
        }

        public bool PriceLocked
        {
            get { return txtLastSell.ReadOnly; }
            set
            {
                this.txtLastSell.TabStop = !value;
                txtLastSell.ReadOnly = value;
            }
        }

        private Decimal m_subtotal = 0;

        public Decimal Subtotal
        {
            get { return m_subtotal; }
        }

        private void UpdateSubtotal()
        {
            try
            {
                Decimal pricePerUnit = Decimal.Parse(txtLastSell.Text);
                int quantity = Int32.Parse(txtStock.Text);

                m_subtotal = pricePerUnit*quantity;
            }
            catch (Exception e)
            {
                ExceptionHandler.LogException(e, true);
                m_subtotal = 0;
            }
            tbSubtotal.Text = m_subtotal.ToString("N");

            if (SubtotalChanged != null)
                SubtotalChanged(this, new EventArgs());
        }

        private void txtLastSell_TextChanged(object sender, EventArgs e)
        {
            UpdateSubtotal();
            if (MineralPriceChanged != null)
                MineralPriceChanged(this, new EventArgs());
        }

        private void txtStock_TextChanged(object sender, EventArgs e)
        {
            UpdateSubtotal();
        }
    }
}