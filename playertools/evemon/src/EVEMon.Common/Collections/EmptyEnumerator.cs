using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace EVEMon.Common.Collections
{
    #region EmptyEnumerator<T>
    /// <summary>
    /// Implémente un énumérateur sur un ensemble vide.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public sealed class EmptyEnumerator<T> : IEnumerator<T>
    {
        /// <summary>
        /// Singleton implementation
        /// </summary>
        public static readonly EmptyEnumerator<T> Instance = new EmptyEnumerator<T>();

        private EmptyEnumerator()
        {
        }

        /// <summary>
        /// Throws an exception
        /// </summary>
        public T Current
        {
            get { throw new InvalidOperationException("The enumerator is empty"); }
        }

        /// <summary>
        /// Dispose the class (do nothing)
        /// </summary>
        public void Dispose()
        {
        }

        /// <summary>
        /// Throws an exception
        /// </summary>
        object System.Collections.IEnumerator.Current
        {
            get { throw new InvalidOperationException("The enumerator is empty"); }
        }

        /// <summary>
        /// Move to the next element, always return false
        /// </summary>
        /// <returns>false</returns>
        public bool MoveNext()
        {
            return false;
        }

        /// <summary>
        /// Resets the enumerator (do nothing)
        /// </summary>
        public void Reset()
        {
        }
    }
    #endregion


    #region EmptyEnumerable<T>
    /// <summary>
    /// Cette classe implémente une collection vide et immuable.
    /// </summary>
    public class EmptyEnumerable<T> : IEnumerable<T>, IEnumerable
    {
        /// <summary>
        /// Singleton implementation
        /// </summary>
        public static readonly EmptyEnumerable<T> Instance = new EmptyEnumerable<T>();

        /// <summary>
        /// Default constructor
        /// </summary>
        private EmptyEnumerable()
        {
        }

        /// <summary>
        /// Gets an empty enumerator
        /// </summary>
        /// <returns>An empty enumerator</returns>
        public IEnumerator<T> GetEnumerator()
        {
            return EmptyEnumerator<T>.Instance;
        }

        /// <summary>
        /// Gets an empty enumerator
        /// </summary>
        /// <returns>An empty enumerator</returns>
        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return EmptyEnumerator<T>.Instance;
        }
    }
    #endregion

}
