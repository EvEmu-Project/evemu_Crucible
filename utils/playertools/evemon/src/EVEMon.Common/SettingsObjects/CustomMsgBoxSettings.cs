using System;
using System.Text;
using System.Windows.Forms;
using System.Xml.Serialization;
using System.Collections.Generic;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class CustomMsgBoxSettings
    {
        public CustomMsgBoxSettings()
        {
            ShowDialogBox = true;
        }

        [XmlAttribute("showDialogBox")]
        public bool ShowDialogBox
        {
            get;
            set;
        }

        [XmlAttribute("dialogResult")]
        public DialogResult DialogResult
        {
            get;
            set;
        }

        internal CustomMsgBoxSettings Clone()
        {
            return (CustomMsgBoxSettings)MemberwiseClone();
        }
    }
}
