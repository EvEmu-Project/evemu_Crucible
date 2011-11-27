using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Collections
{

    #region IReadonlyCollection<T>
    /// <summary>
    /// Represents a read-only collection
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public interface IReadonlyCollection<T> : IEnumerable<T>
    {
        int Count { get; }
    }
    #endregion


    #region IReadonlyKeyedCollection<T>
    /// <summary>
    /// Represents a read-only collection
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public interface IReadonlyKeyedCollection<TKey, TItem> : IEnumerable<TItem>
    {
        int Count { get; }
        TItem this[TKey key] { get; }
    }
    #endregion

}
