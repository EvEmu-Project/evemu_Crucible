using System.Collections.Generic;
using System.Linq;

using EVEMon.Common;
using EVEMon.Common.SettingsObjects;
using EVEMon.Controls;

namespace EVEMon
{
    public sealed class EveNotificationsColumnsSelectWindow : ColumnSelectWindow
    {
        public EveNotificationsColumnsSelectWindow(IEnumerable<EveNotificationsColumnSettings> settings)
            : base(settings.Select(x => x.Clone()).Cast<IColumnSettings>())
        {
        }

        protected override string GetHeader(int key)
        {
            return ((EveNotificationsColumn)key).GetDescription();
        }

        protected override IEnumerable<int> GetAllKeys()
        {
            return EnumExtensions.GetValues<EveNotificationsColumn>().Where(x => x != EveNotificationsColumn.None).Select(x => (int)x);
        }

        protected override IEnumerable<IColumnSettings> GetDefaultColumns()
        {
            var settings = new EveNotificationsSettings();
            return settings.Columns.Cast<IColumnSettings>();
        }
    }
}
