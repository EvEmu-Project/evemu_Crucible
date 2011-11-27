using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class TrayTooltipSettings
    {
        public TrayTooltipSettings()
        {
            Format = "%n - %s %tr - %r";
        }

        [XmlElement("format")]
        public string Format
        {
            get;
            set;
        }

        [XmlElement("DisplayOrder")]
        public bool DisplayOrder
        {
            get;
            set;
        }

        public TrayTooltipSettings Clone()
        {
            return this.MemberwiseClone() as TrayTooltipSettings;
        }
    }
}
