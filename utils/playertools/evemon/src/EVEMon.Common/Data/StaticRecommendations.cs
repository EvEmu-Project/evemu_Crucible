using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a list of recommendations
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public sealed class StaticRecommendations<T> : ReadonlyCollection<T>
    {
        /// <summary>
        /// Default constructor, only used during datafiles initialization
        /// </summary>
        internal StaticRecommendations()
        {
        }

        /// <summary>
        /// Adds the given item to the recommendations list.
        /// </summary>
        /// <param name="item"></param>
        internal void Add(T item)
        {
            m_items.Add(item);
        }
    }
}
