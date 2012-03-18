using System;
using System.Collections.Generic;
using System.Text;

namespace System.Linq
{
    public static class EnumerableExtensions
    {
        /// <summary>
        /// Checks whether this enumerable is empty.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="src"></param>
        public static bool IsEmpty<T>(this IEnumerable<T> src)
        {
            foreach (var item in src) return false;
            return true;
        }

        /// <summary>
        /// Uses an insertion sort algorithm to perform a stable sort (keep the initial order of the keys with equal values).
        /// </summary>
        /// <remarks>Memory overhead is null, average complexity is O(n.ln(n)), worst-case is O(n²).</remarks>
        /// <typeparam name="T"></typeparam>
        /// <param name="src"></param>
        /// <param name="comparison"></param>
        public static void StableSort<T>(this IList<T> list, IComparer<T> comparer)
        {
            list.StableSort(comparer.Compare);
        }

        /// <summary>
        /// Uses an insertion sort algorithm to perform a stable sort (keep the initial order of the keys with equal values).
        /// </summary>
        /// <remarks>Memory overhead is null, average complexity is O(n.ln(n)), worst-case is O(n²).</remarks>
        /// <typeparam name="T"></typeparam>
        /// <param name="src"></param>
        /// <param name="comparison"></param>
        public static void StableSort<T>(this IList<T> list, Comparison<T> comparison)
        {
            // For every key
            for (int i = 1; i < list.Count; i++)
            {
                var value = list[i];
                int j = i - 1;

                // Move the key backward while the previous items are lesser than it, shifting those items to the right
                while (j >= 0 && comparison(list[j], value) > 0)
                {
                    list[j + 1] = list[j];
                    j--;
                }

                // Insert at the left of the scrolled sequence, immediately on the right of the first lesser or equal value it found
                list[j + 1] = value;
            }
        }

        /// <summary>
        /// Gets the index of the given element in this enumeration, or -1 when the item is absent from the enumeration.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="src"></param>
        /// <param name="item"></param>
        /// <returns></returns>
        public static int IndexOf<T>(this IEnumerable<T> src, T item)
        {
            int index = 0;
            foreach (var srcItem in src)
            {
                if (Object.Equals((object)item, (object)srcItem)) return index;
                index++;
            }
            return -1;
        }
    }
}
