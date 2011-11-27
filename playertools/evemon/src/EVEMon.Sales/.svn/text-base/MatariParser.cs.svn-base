using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text.RegularExpressions;
using EVEMon.Common;
using EVEMon.Common.Net;

namespace EVEMon.Sales
{
    [DefaultMineralParser("matari")]
    public class MatariParser : IMineralParser
    {
        private static Regex mineralTokenizer =
            new Regex(@"\<td.*?\>(?<name>\w*)\</td\>\<td.*?align.*?\>(?<price>(\d|\.|,)*)\</td\>\<td",
                      RegexOptions.Compiled);

        #region IMineralParser Members
        public string Title
        {
            get { return "Matari Mineral Index"; }
        }

        public string CourtesyUrl
        {
            get { return "http://www.evegeek.com/"; }
        }

        public string CourtesyText
        {
            get { return "EVE[geek]"; }
        }

        public IEnumerable<Pair<string, decimal>> GetPrices()
        {
            string phoenixContent;
            try
            {
                phoenixContent = EveClient.HttpWebService.DownloadString(
                    "http://www.evegeek.com/mineralindex.php");
            }
            catch (HttpWebServiceException ex)
            {
                ExceptionHandler.LogException(ex, true);
                throw new MineralParserException(ex.Message);
            }

            //scan for prices
            MatchCollection mc = mineralTokenizer.Matches(phoenixContent);

            foreach (Match mineral in mc)
            {
                string name = mineral.Groups["name"].Value;

                Decimal price = Decimal.Parse(mineral.Groups["price"].Value, NumberStyles.Currency, CultureInfo.InvariantCulture);
                yield return new Pair<string, Decimal>(name, price);
            }
        }
        #endregion
    }
}
