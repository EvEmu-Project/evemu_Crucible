using System.Collections.Generic;
using System.Linq;

using EVEMon.Common;
using EVEMon.Common.SettingsObjects;
using EVEMon.Controls;

namespace EVEMon
{
    public sealed class EveMailMessagesColumnsSelectWindow : ColumnSelectWindow
    {
        public EveMailMessagesColumnsSelectWindow(IEnumerable<EveMailMessagesColumnSettings> settings)
            : base(settings.Select(x => x.Clone()).Cast<IColumnSettings>())
        {
        }

        protected override string GetHeader(int key)
        {
            return ((EveMailMessagesColumn)key).GetDescription();
        }

        protected override IEnumerable<int> GetAllKeys()
        {
            return EnumExtensions.GetValues<EveMailMessagesColumn>().Where(x => x != EveMailMessagesColumn.None).Select(x => (int)x);
        }

        protected override IEnumerable<IColumnSettings> GetDefaultColumns()
        {
            var settings = new EveMailMessagesSettings();
            return settings.Columns.Cast<IColumnSettings>();
        }
    }
}
