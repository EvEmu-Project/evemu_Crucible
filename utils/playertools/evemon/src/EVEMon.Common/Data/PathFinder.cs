using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Data
{
    internal sealed class PathFinder
    {
        private readonly PathFinder m_parent;
        private readonly SolarSystem m_system;

        /// <summary>
        /// Constructor for a starting point
        /// </summary>
        /// <param name="system"></param>
        private PathFinder(SolarSystem system)
        {
            m_parent = null;
            m_system = system;
        }

        /// <summary>
        /// Constructor for a child.
        /// </summary>
        /// <param name="parent"></param>
        /// <param name="system"></param>
        private PathFinder(PathFinder parent, SolarSystem system)
        {
            m_parent = parent;
            m_system = system;
        }

        /// <summary>
        /// Gets the best possible path (approximate solution) using a A* alogrithm.
        /// </summary>
        /// <param name="start"></param>
        /// <param name="target"></param>
        /// <param name="minSecurityLevel">The mininmum, inclusive, security level.</param>
        /// <returns></returns>
        public static List<SolarSystem> FindBestPath(SolarSystem start, SolarSystem target, float minSecurityLevel)
        {
            var result = FindBestPathCore(start, target, minSecurityLevel);

            // Transforms the result into a list
            var list = new List<SolarSystem>();
            for (var item = result; item != null; item = item.m_parent)
            {
                list.Insert(0, item.m_system);
            }

            return list;
        }

        /// <summary>
        /// Gets the best possible path (approximate solution) using a A* alogrithm.
        /// </summary>
        /// <param name="start"></param>
        /// <param name="target"></param>
        /// <param name="minSecurityLevel">The mininmum, inclusive, security level.</param>
        /// <returns></returns>
        private static PathFinder FindBestPathCore(SolarSystem start, SolarSystem target, float minSecurityLevel)
        {
            var bestDepthes = new Dictionary<SolarSystem, int>();
            var paths = new SortedList<float, PathFinder>();
            PathFinder root = new PathFinder(start);
            bestDepthes.Add(start, -1);
            paths.Add(0, root);

            while (true)
            {
                if (paths.Count == 0) return null;
                PathFinder best = null;
                int depth = 0;

                // Pick the best candidate path, but ensures it matches the best depth found so far
                while (true)
                {
                    // Picks the best one, returns if we found our target
                    best = paths.Values[0];
                    if (best.m_system == target) return best;

                    // Removes it from the list
                    paths.RemoveAt(0);

                    int bestDepth;
                    depth = best.Depth;
                    bestDepthes.TryGetValue(best.m_system, out bestDepth);
                    if (bestDepth == depth || best.m_system == start) break;
                }

                // Gets the subpaths for the best path so far.
                depth++;
                foreach (var child in best.GetChildren(depth, bestDepthes))
                {
                    // Gets the heuristic key: the shorter, the better the candidate.
                    var key = child.m_system.GetSquareDistanceWith(target) * (float)(depth * depth);
                    if (child.m_system.SecurityLevel < minSecurityLevel) key *= 100.0f;
                    while (paths.ContainsKey(key)) key++;

                    // Stores it in the sorted list.
                    paths.Add(key, child);
                }
            }

        }

        /// <summary>
        /// Gets all the immediate children paths.
        /// </summary>
        /// <returns></returns>
        private IEnumerable<PathFinder> GetChildren(int depth, Dictionary<SolarSystem, int> bestDepthes)
        {
            // Check this system is not already present with a lesser range.
            foreach (var neighbor in m_system.Neighbors)
            {
                // Checks the best depth so far
                int bestDepth;
                bestDepthes.TryGetValue(neighbor, out bestDepth);

                // Enumerates it
                if (bestDepth == 0 || depth < bestDepth)
                {
                    bestDepthes[neighbor] = depth;
                    yield return new PathFinder(this, neighbor);
                }
            }
        }

        /// <summary>
        /// Gets the depth of this node.
        /// </summary>
        private int Depth
        {
            get
            {
                int count = 0;
                for (var parent = m_parent; parent != null; parent = parent.m_parent)
                {
                    count++;
                }
                return count;
            }
        }

        public override string ToString()
        {
            return m_system.ToString();
        }
    }
}
