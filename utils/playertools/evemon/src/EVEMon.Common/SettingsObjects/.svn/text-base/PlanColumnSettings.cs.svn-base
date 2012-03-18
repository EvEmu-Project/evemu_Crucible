using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class PlanColumnSettings : IColumnSettings
    {
        public PlanColumnSettings()
        {
            this.Width = -1;
        }

        [XmlAttribute("column")]
        public PlanColumn Column
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

        public PlanColumnSettings Clone()
        {
            return new PlanColumnSettings { Column = this.Column, Visible = this.Visible, Width = this.Width };
        }

        public override string ToString()
        {
            return Column.GetHeader();
        }
    }


    public interface IColumnSettings
    {
        bool Visible { get; set; }
        int Key { get; }
    }
}
