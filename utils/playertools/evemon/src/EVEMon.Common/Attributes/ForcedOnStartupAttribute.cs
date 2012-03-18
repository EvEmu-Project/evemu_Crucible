using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Attributes
{
    /// <summary>
    /// This attribute marks the API methods which are always queried on startup.
    /// </summary>
    public sealed class ForcedOnStartupAttribute : Attribute
    {
    }
}
