using System;
using System.Collections.Generic;
using System.Text;
using System.Collections;
using System.Windows.Forms;

namespace EVEMon.Controls
{
    /// <summary>
    /// Simple comparer for <see cref="ListViewItem"/> relying on a <see cref="IComparer{T}"/> for tags comparison. 
    /// For use with <see cref="ListView.ListViewItemSorter"/>.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public sealed class ListViewItemComparerByTag<T> : IComparer
    {
        private readonly IComparer<T> m_comparer;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="comparer">The tags comparer</param>
        public ListViewItemComparerByTag(IComparer<T> comparer)
        {
            m_comparer = comparer;
        }

        /// <summary>
        /// Comparer two <see cref="ListViewItem"/>.
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <returns></returns>
        public int Compare(object x, object y)
        {
            var itemX = (ListViewItem)x;
            var itemY = (ListViewItem)y;
            var tagX = (T)itemX.Tag;
            var tagY = (T)itemY.Tag;

            return m_comparer.Compare(tagX, tagY);
        }
    }
}
