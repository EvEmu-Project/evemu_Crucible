using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents the collections of solar systems at a given number of jumps of a certain solar system.
    /// </summary>
    public sealed class SolarSystemRange : ReadonlyCollection<SolarSystem>
    {
        private SolarSystem m_source;
        private int m_range;

        /// <summary>
        /// Private constructor.
        /// </summary>
        /// <param name="range"></param>
        private SolarSystemRange(SolarSystem source, int range)
            : base(1)
        {
            m_source = source;
            m_range = range;
        }

        /// <summary>
        /// Gets the source solar system.
        /// </summary>
        public SolarSystem Source
        {
            get { return m_source; }
        }

        /// <summary>
        /// The number of jumps those system are located from the source.
        /// </summary>
        public int Range
        {
            get { return m_range; }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="system"></param>
        /// <param name="maxInclusiveNumberOfJumps">The maximum, inclusive, number of jumps.</param>
        /// <returns></returns>
        internal static List<SolarSystemRange> GetSystemRangesFrom(SolarSystem system, int maxInclusiveNumberOfJumps)
        {
            var collectedSystems = new Dictionary<long, SolarSystem>();
            var ranges = new List<SolarSystemRange>();
            var lastRange = new SolarSystemRange(system, 0);

            collectedSystems.Add(system.ID, system);
            lastRange.m_items.Add(system);

            for (int i = 1; i <= maxInclusiveNumberOfJumps; i++)
            {
                lastRange = lastRange.GetNextRange(collectedSystems);
                ranges.Add(lastRange);
            }

            return ranges;
        }

        /// <summary>
        /// gets the next 
        /// </summary>
        /// <returns></returns>
        private SolarSystemRange GetNextRange(Dictionary<long, SolarSystem> collectedSystems)
        {
            SolarSystemRange nextRange = new SolarSystemRange(m_source, m_range + 1);

            foreach (var system in m_items)
            {
                foreach (var child in system.Neighbors)
                {
                    if (!collectedSystems.ContainsKey(child.ID))
                    {
                        collectedSystems.Add(child.ID, child);
                        nextRange.m_items.Add(child);
                    }
                }
            }

            return nextRange;
        }
    }
}
