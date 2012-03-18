using System.Collections.Generic;
using System.Linq;

using EVEMon.Common;
using EVEMon.Common.SettingsObjects;
using EVEMon.Controls;

namespace EVEMon
{
    public sealed class IndustryJobsColumnsSelectWindow : ColumnSelectWindow
    {
        public IndustryJobsColumnsSelectWindow(IEnumerable<IndustryJobColumnSettings> settings)
            : base(settings.Select(x => x.Clone()).Cast<IColumnSettings>())
        {
        }

        protected override string GetHeader(int key)
        {
            return ((IndustryJobColumn)key).GetDescription();
        }

        protected override IEnumerable<int> GetAllKeys()
        {
            return EnumExtensions.GetValues<IndustryJobColumn>().Where(x => x != IndustryJobColumn.None).Select(x => (int)x);
        }

        protected override IEnumerable<IColumnSettings> GetDefaultColumns()
        {
            var settings = new IndustryJobSettings();
            return settings.Columns.Cast<IColumnSettings>();
        }
    }
}
