using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.Xml.Serialization;
using EVEMon.Common.Attributes;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class MarketOrderSettings
    {
        public MarketOrderSettings()
        {
            // Add default columns
            var defaultColumns = new MarketOrderColumn[]
            { 
                MarketOrderColumn.Item, 
                MarketOrderColumn.SolarSystem, 
                MarketOrderColumn.UnitaryPrice, 
                MarketOrderColumn.Volume
            };

            Columns = EnumExtensions.GetValues<MarketOrderColumn>().Where(x => x != MarketOrderColumn.None).Select(x =>
                new MarketOrderColumnSettings { Column = x, Visible = defaultColumns.Contains(x), Width = -1 }).ToArray();

            HideInactiveOrders = true;
        }

        [XmlArray("columns")]
        [XmlArrayItem("column")]
        public MarketOrderColumnSettings[] Columns
        {
            get;
            set;
        }

        [XmlElement("hideInactiveOrders")]
        public bool HideInactiveOrders
        {
            get;
            set;
        }

        [XmlElement("numberAbsFormat")]
        public bool NumberAbsFormat
        {
            get;
            set;
        }

        public MarketOrderSettings Clone()
        {
            MarketOrderSettings clone = (MarketOrderSettings)MemberwiseClone();
            clone.Columns = Columns.Select(x => x.Clone()).ToArray();
            clone.HideInactiveOrders = HideInactiveOrders;
            clone.NumberAbsFormat = NumberAbsFormat;
            return clone;
        }
    }
}
