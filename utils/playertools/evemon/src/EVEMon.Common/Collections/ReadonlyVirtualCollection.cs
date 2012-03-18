using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Collections
{
    /// <summary>
    /// Represents a base readonly collection relying on a method override providing an <see cref="IEnumerable{T}"/>.
    /// </summary>
    public abstract class ReadonlyVirtualCollection<T> : IReadonlyCollection<T>
    {
        /// <summary>
        /// Protected default constructor
        /// </summary>
        protected ReadonlyVirtualCollection()
        {
        }

        /// <summary>
        /// The core method to implement : all other methods rely on this one.
        /// </summary>
        /// <returns></returns>
        protected abstract IEnumerable<T> Enumerate();

        /// <summary>
        /// Gets the number of items in this collection
        /// </summary>
        public int Count
        {
            get
            {
                int count = 0;
                foreach (var item in Enumerate()) count++;
                return count;
            }
        }

        #region Enumerators
        IEnumerator<T> IEnumerable<T>.GetEnumerator()
        {
            return Enumerate().GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return Enumerate().GetEnumerator();
        }
        #endregion
    }
}
