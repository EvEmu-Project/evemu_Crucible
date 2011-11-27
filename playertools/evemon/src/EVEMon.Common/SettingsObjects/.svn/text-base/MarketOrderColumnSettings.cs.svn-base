using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

using EVEMon.Common;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class MarketOrderColumnSettings : IColumnSettings
    {
        public MarketOrderColumnSettings()
        {
            Width = -1;
        }

        [XmlAttribute("column")]
        public MarketOrderColumn Column
        {
            get;
            set;
        }

        [XmlAttribute("visible")]
        public bool Visible
        {
            get;
            set;
        }

        [XmlAttribute("width")]
        public int Width
        {
            get;
            set;
        }

        int IColumnSettings.Key
        {
            get { return (int)Column; }
        }

        public MarketOrderColumnSettings Clone()
        {
            return new MarketOrderColumnSettings { Column = this.Column, Visible = this.Visible, Width = this.Width };
        }

        public override string ToString()
        {
            return Column.GetHeader();
        }
    }
}
