using System;
using EVEMon.Common;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Provides a way for implant calculator and attributes optimization form to add a column showing the training time difference.
    /// </summary>
    public interface IPlanOrderPluggable
    {
        event EventHandler Disposed;
        void UpdateStatistics(BasePlan plan, out bool areRemappingPointsActive);
        void UpdateOnImplantSetChange();
    }
}
