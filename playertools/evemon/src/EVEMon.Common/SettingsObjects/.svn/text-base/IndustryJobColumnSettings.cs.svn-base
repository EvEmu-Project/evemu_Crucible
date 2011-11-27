using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

using EVEMon.Common;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class IndustryJobColumnSettings : IColumnSettings
    {
        public IndustryJobColumnSettings()
        {
            Width = -1;
        }

        [XmlAttribute("column")]
        public IndustryJobColumn Column
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

        /// <summary>
        /// ShallowClones this instance.
        /// </summary>
        /// <returns></returns>
        public IndustryJobColumnSettings Clone()
        {
            return new IndustryJobColumnSettings { Column = this.Column, Visible = this.Visible, Width = this.Width };
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString()
        {
            return Column.GetHeader();
        }
    }
}
