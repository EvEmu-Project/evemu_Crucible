using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text.RegularExpressions;
using EVEMon.Common;
using EVEMon.Common.Net;

namespace EVEMon.Sales
{
    [DefaultMineralParser("battleclinic")]
    public class BattleclinicParser : IMineralParser
    {

        private static Regex mineralTokenizer =
            new Regex(@"<name>(?<name>.+?)</name>.+?<price>(?<price>.+?)</price>",
                      RegexOptions.Compiled | RegexOptions.IgnorePatternWhitespace | RegexOptions.Singleline 
                        | RegexOptions.Multiline
                        | RegexOptions.IgnoreCase);

        #region BattleclinicParser Members
        public string Title
        {
            get { return "BattleClinic.com EVE Averages"; }
        }

        public string CourtesyUrl
        {
            get { return "http://eve.battleclinic.com/"; }
        }

        public string CourtesyText
        {
            get { return "BattleClinic.com"; }
        }

        public IEnumerable<Pair<string, decimal>> GetPrices()
        {
            string content;
            try
            {
                content = EveClient.HttpWebService.DownloadString(
                    "http://eve.battleclinic.com/market_xml.php");
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
