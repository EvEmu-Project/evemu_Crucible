using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text.RegularExpressions;
using EVEMon.Common;
using EVEMon.Common.Net;

namespace EVEMon.Sales
{
    [DefaultMineralParser("cxi")]
    public class CXIParser : IMineralParser
    {
        private static Regex mineralTokenizer =
            new Regex(@"<name>(?<name>.+?)</name>.+?<price>(?<price>.+?)</price>",
              RegexOptions.Compiled | RegexOptions.IgnorePatternWhitespace | RegexOptions.Singleline
                | RegexOptions.Multiline
                | RegexOptions.IgnoreCase);

        #region IMineralParser Members
        public string Title
        {
            get { return "CompleXion Industries"; }
        }

        public string CourtesyUrl
        {
            get { return "http://cxa.supreme-eve.com/wiki/"; }
        }

        public string CourtesyText
        {
            get { return "CompleXion Industries"; }
        }

        public IEnumerable<Pair<string, decimal>> GetPrices()
        {
            string content;
            try
            {
                content = EveClient.HttpWebService.DownloadString(
                    "http://www.c-l-o-t.com/cxi/orecalc/evemon.php");
            }
            catch (HttpWebServiceException ex)
            {
                ExceptionHandler.LogException(ex, true);
                throw new MineralParserException(ex.Message);
            }

            //scan for prices
            
            MatchCollection mc = mineralTokenizer.Matches(content);

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
