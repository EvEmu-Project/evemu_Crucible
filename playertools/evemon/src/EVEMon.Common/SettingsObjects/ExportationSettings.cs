using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class ExportationSettings
    {
        public ExportationSettings()
        {
            PlanToText = new PlanExportSettings();
        }

        public PlanExportSettings PlanToText
        {
            get;
            set;
        }

        internal ExportationSettings Clone()
        {
            return new ExportationSettings { PlanToText = this.PlanToText.Clone() };
        }
    }
}
