using System.Collections.Generic;
using System.Linq;

using EVEMon.Common;
using EVEMon.Common.SettingsObjects;
using EVEMon.Controls;

namespace EVEMon
{
    public sealed class ResearchColumnsSelectWindow : ColumnSelectWindow
    {
        public ResearchColumnsSelectWindow(IEnumerable<ResearchColumnSettings> settings)
            : base(settings.Select(x => x.Clone()).Cast<IColumnSettings>())
        {
        }

        protected override string GetHeader(int key)
        {
            return ((ResearchColumn)key).GetDescription();
        }

        protected override IEnumerable<int> GetAllKeys()
        {
            return EnumExtensions.GetValues<ResearchColumn>().Where(x => x != ResearchColumn.None).Select(x => (int)x);
        }

        protected override IEnumerable<IColumnSettings> GetDefaultColumns()
        {
            var settings = new ResearchSettings();
            return settings.Columns.Cast<IColumnSettings>();
        }
    }
}
