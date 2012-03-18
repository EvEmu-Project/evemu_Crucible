using System;
using System.Collections.Generic;
using System.Text;

using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a read-only collection of blueprint groups
    /// </summary>
    public sealed class BlueprintMarketGroupCollection : ReadonlyCollection<BlueprintMarketGroup>
    {
        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="src"></param>
        internal BlueprintMarketGroupCollection(BlueprintMarketGroup group, SerializableBlueprintGroup[] src)
            : base(src == null ? 0 : src.Length)
        {
            if (src == null)
                return;

            foreach (var subGroup in src)
            {
                m_items.Add(new BlueprintMarketGroup(group, subGroup));
            }
        }
    }
}
