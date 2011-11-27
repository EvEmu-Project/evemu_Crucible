using System.Collections.Generic;

using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    /// <summary>
    /// Performs a comparison between two <see cref="IndustryJob" /> types.
    /// </summary>
    public sealed class IndustryJobComparer : Comparer<IndustryJob>
    {
        private IndustryJobColumn m_column;
        private bool m_isAscending;

        /// <summary>
        /// Initializes a new instance of the <see cref="IndustryJobComparer"/> class.
        /// </summary>
        /// <param name="column">The industry job column.</param>
        /// <param name="isAscending">Is ascending flag.</param>
        public IndustryJobComparer(IndustryJobColumn column, bool isAscending)
        {
            m_column = column;
            m_isAscending = isAscending;
        }

        /// <summary>
        /// Performs a comparison of two objects of the <see cref="IndustryJob" /> type and returns a value
        /// indicating whether one object is less than, equal to, or greater than the other.
        /// </summary>
        /// <param name="x">The first object to compare.</param>
        /// <param name="y">The second object to compare.</param>
        /// <returns>
        /// Less than zero
        /// <paramref name="x"/> is less than <paramref name="y"/>.
        /// Zero
        /// <paramref name="x"/> equals <paramref name="y"/>.
        /// Greater than zero
        /// <paramref name="x"/> is greater than <paramref name="y"/>.
        /// </returns>
        public override int Compare(IndustryJob x, IndustryJob y)
        {
            if (m_isAscending)
                return CompareCore(x, y);

            return -CompareCore(x, y);
        }

        /// <summary>
        /// Performs a comparison of two objects of the <see cref="IndustryJob" /> type and returns a value
        /// indicating whether one object is less than, equal to, or greater than the other.
        /// </summary>
        /// <param name="x">The first object to compare.</param>
        /// <param name="y">The second object to compare.</param>
        /// <returns>
        /// Less than zero
        /// <paramref name="x"/> is less than <paramref name="y"/>.
        /// Zero
        /// <paramref name="x"/> equals <paramref name="y"/>.
        /// Greater than zero
        /// <paramref name="x"/> is greater than <paramref name="y"/>.
        /// </returns>
        private int CompareCore(IndustryJob x, IndustryJob y)
        {
            switch (m_column)
            {
                case IndustryJobColumn.State:
                    if (x.State == JobState.Active)
                        return x.ActiveJobState.CompareTo(y.ActiveJobState);
                    return x.State.CompareTo(y.State);

                case IndustryJobColumn.TTC:
                    return x.EndProductionTime.CompareTo(y.EndProductionTime);

                case IndustryJobColumn.InstalledItem:
                    return x.InstalledItem.Name.CompareTo(y.InstalledItem.Name);

                case IndustryJobColumn.InstalledItemType:
                    return x.InstalledItem.MarketGroup.Name.CompareTo(y.InstalledItem.MarketGroup.Name);

                case IndustryJobColumn.OutputItem:
                    return x.OutputItem.Name.CompareTo(y.OutputItem.Name);

                case IndustryJobColumn.OutputItemType:
                    return x.OutputItem.MarketGroup.Name.CompareTo(y.OutputItem.MarketGroup.Name);

                case IndustryJobColumn.Activity:
                    return x.Activity.CompareTo(y.Activity);

                case IndustryJobColumn.InstallTime:
                    return x.InstalledTime.CompareTo(y.InstalledTime);

                case IndustryJobColumn.EndTime:
                    return x.EndProductionTime.CompareTo(y.EndProductionTime);

                case IndustryJobColumn.OriginalOrCopy:
                    return x.BlueprintType.CompareTo(y.BlueprintType);

                case IndustryJobColumn.InstalledME:
                    return x.InstalledME.CompareTo(y.InstalledME);

                case IndustryJobColumn.EndME:
                    return (x.InstalledME + x.Runs).CompareTo(y.InstalledME + y.Runs);

                case IndustryJobColumn.InstalledPE:
                    return x.InstalledPE.CompareTo(y.InstalledPE);

                case IndustryJobColumn.EndPE:
                    return (x.InstalledPE + x.Runs).CompareTo(y.InstalledPE + y.Runs);

                case IndustryJobColumn.Location:
                    return x.Installation.CompareTo(y.Installation);

                case IndustryJobColumn.Region:
                    return x.SolarSystem.Constellation.Region.CompareTo(y.SolarSystem.Constellation.Region);

                case IndustryJobColumn.SolarSystem:
                    return x.SolarSystem.CompareTo(y.SolarSystem);

                case IndustryJobColumn.Installation:
                    return x.Installation.CompareTo(y.Installation);

                case IndustryJobColumn.IssuedFor:
                    return x.IssuedFor.CompareTo(y.IssuedFor);

                case IndustryJobColumn.LastStateChange:
                    return x.LastStateChange.CompareTo(y.LastStateChange);

                default:
                    return 0;
            }
        }
    }
}
