using System;
using System.Collections.Generic;
using System.Text;

using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a read-only collection of blueprints
    /// </summary>
    public sealed class BlueprintCollection : ReadonlyCollection<Blueprint>
    {
        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="group">Blueprint Market Group that the blueprint will contain</param>
        /// <param name="src">One or more serializable blueprints</param>
        internal BlueprintCollection(BlueprintMarketGroup group, SerializableBlueprint[] src)
            : base(src == null ? 0 : src.Length)
        {
            if (src == null)
                return;

            foreach (var blueprint in src)
            {
                m_items.Add(new Blueprint(group, blueprint));
            }
        }
    }
}
