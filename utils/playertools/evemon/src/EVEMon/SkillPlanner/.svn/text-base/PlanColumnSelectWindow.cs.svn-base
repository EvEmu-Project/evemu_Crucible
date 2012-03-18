using System;
using System.Linq;
using System.Collections.Generic;
using System.Text;
using EVEMon.Controls;
using EVEMon.Common;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.SkillPlanner
{
    public sealed class PlanColumnSelectWindow : ColumnSelectWindow
    {
        public PlanColumnSelectWindow(IEnumerable<PlanColumnSettings> settings)
            : base(settings.Select(x => x.Clone()).Cast<IColumnSettings>())
        {
        }

        protected override string GetHeader(int key)
        {
            return ((PlanColumn)key).GetDescription();
        }

        protected override IEnumerable<int> GetAllKeys()
        {
            return EnumExtensions.GetValues<PlanColumn>().Where(x => x != PlanColumn.None).Select(x => (int)x);
        }

        protected override IEnumerable<IColumnSettings> GetDefaultColumns()
        {
            var settings = new PlanWindowSettings();
            return settings.Columns.Cast<IColumnSettings>();
        }
    }
}
