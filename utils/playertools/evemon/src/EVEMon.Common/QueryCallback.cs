using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    /// <summary>
    /// A delegate for query callbacks.
    /// </summary>
    /// <param name="document"></param>
    public delegate void QueryCallback<T>(APIResult<T> result);
}