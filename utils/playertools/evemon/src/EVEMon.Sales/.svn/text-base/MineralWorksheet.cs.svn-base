using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using EVEMon.Common;
using EVEMon.Common.Controls;

namespace EVEMon.Sales
{
	internal delegate void TileUpdate(MineralTile mt, Single s);

	public partial class MineralWorksheet : EVEMonForm
	{
		private Decimal m_total;
		private string m_courtesyUrl;
		private string m_source;

		private EventHandler<EventArgs> tileChangeHandler;

		public MineralWorksheet()
		{
			InitializeComponent();
		}
        
		private IEnumerable<MineralTile> Tiles
		{
			get
			{
				foreach (Control c in MineralTileTableLayout.Controls)
				{
					if (c is MineralTile)
					{
						yield return c as MineralTile;
					}
				}
			}
		}

		private void TileSubtotalChanged(object sender, EventArgs e)
		{
			m_total = 0;
			foreach (MineralTile mt in Tiles)
			{
				m_total += mt.Subtotal;
			}

			tslTotal.Text = String.Format(CultureConstants.DefaultCulture, "{0:N} ISK", m_total);
		}

		private void MineralWorksheet_Load(object sender, EventArgs e)
		{
			tileChangeHandler = TileSubtotalChanged;
			foreach (MineralTile mt in Tiles)
			{
				mt.SubtotalChanged += tileChangeHandler;
			}

			SortedList<string, Pair<string, string>> parsersSorted = new SortedList<string, Pair<string, string>>();

			foreach (Pair<string, IMineralParser> p in MineralDataRequest.Parsers)
			{
				parsersSorted.Add(p.B.Title, new Pair<string, string>(p.A, p.B.Title));
			}

			foreach (Pair<string, string> p in parsersSorted.Values)
			{
				ToolStripMenuItem mi = new ToolStripMenuItem();
				mi.Text = p.B;
				mi.Tag = p.A;
				mi.Click += mi_Click;
				tsddFetch.DropDownItems.Add(mi);
			}
		}

		private void mi_Click(object sender, EventArgs e)
		{
			if (!bckgrndWrkrGetPrices.IsBusy)
			{
				ToolStripMenuItem mi = sender as ToolStripMenuItem;
				if (mi != null && mi.Tag is string)
				{
					m_source = mi.Tag as string;
					bckgrndWrkrGetPrices.RunWorkerAsync(m_source);
				}
			}
		}

		private void bckgrndWrkrGetPrices_DoWork(object sender, DoWorkEventArgs e)
		{
			IDictionary<string, Decimal> prices = new Dictionary<string, decimal>();
			string source = e.Argument as string;
			if (source != null)
			{
				foreach (Pair<string, Decimal> p in MineralDataRequest.GetPrices(source))
				{
					prices[p.A] = p.B;
				}
			}

			e.Result = prices;
		}

		private void bckgrndWrkrGetPrices_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
		{
			if (e.Error != null)
			{
				ExceptionHandler.LogException(e.Error, true);
                MessageBox.Show(String.Format("Failed to retrieve mineral pricing data:\n{0}", e.Error.Message),
                                "Failed to Retrieve Data",
								MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
			else
			{
                IDictionary<string, Decimal> prices = e.Result as IDictionary<string, Decimal>;
				if (prices != null)
				{
					foreach (MineralTile mt in Tiles)
					{
						if (prices.ContainsKey(mt.MineralName))
						{
							mt.PricePerUnit = prices[mt.MineralName];
						}
					}

                    tslCourtesy.Text = String.Format("Mineral Prices Courtesy of {0}", MineralDataRequest.GetCourtesyText(m_source));
					m_courtesyUrl = MineralDataRequest.GetCourtesyUrl(m_source);
					tslCourtesy.Visible = true;
				}
			}
		}

		private bool m_pricesLocked;

		private bool PricesLocked
		{
			set
			{
				m_pricesLocked = value;
				foreach (MineralTile mt in Tiles)
				{
					mt.PriceLocked = value;
				}
			}
		}

		private void btnLockPrices_Click(object sender, EventArgs e)
		{
			if (m_pricesLocked)
			{
				PricesLocked = false;
				btnLockPrices.Text = "Lock Prices";
			}
			else
			{
				PricesLocked = true;
				btnLockPrices.Text = "Unlock Prices";
			}
		}

		private void tslCourtesy_Click(object sender, EventArgs e)
		{
			try
			{
				using (Process.Start(m_courtesyUrl))
				{
				}
			}
			catch (Exception ex)
			{
				ExceptionHandler.LogException(ex, false);
			}
		}

		private void mt_MineralPriceChanged(object sender, EventArgs e)
		{
			tslCourtesy.Visible = false;
		}

		private void btnReset_Click(object sender, EventArgs e)
		{
			foreach (MineralTile mt in Tiles)
			{
				mt.Quantity = 0;
			}
		}

		private void copyTotalDropDownButton_DropDownOpening(object sender, EventArgs e)
		{
			copyFormattedTotalToolStripMenuItem.Text = String.Format(CultureConstants.DefaultCulture, "Formatted ({0:N} ISK)", m_total);
			copyUnformattedTotalToolStripMenuItem.Text = String.Format(CultureConstants.DefaultCulture, "Unformatted ({0})", m_total);
		}

		private void copyFormattedTotalToolStripMenuItem_Click(object sender, EventArgs e)
		{
            try
            {
                Clipboard.Clear();
                Clipboard.SetText(tslTotal.Text);
            }
            catch (ExternalException ex)
            {
                // there is a bug that results in an exception being
                // thrown when the clipboard is in use by another
                // thread.
                ExceptionHandler.LogException(ex, true);
            }
		}

		private void copyUnformattedTotalToolStripMenuItem_Click(object sender, EventArgs e)
		{
            try
            {
                Clipboard.Clear();
                Clipboard.SetText(m_total.ToString());
            }
            catch (ExternalException ex)
            {
                // there is a bug that results in an exception being
                // thrown when the clipboard is in use by another
                // thread.
                ExceptionHandler.LogException(ex, true);
            }
		}
	}
}
