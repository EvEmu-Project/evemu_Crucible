using System;
using System.Collections.Generic;
using System.Text;
using System.Security;
using System.Runtime.InteropServices;

namespace EVEMon.WindowsApi
{
    /// <summary>
    /// Windows 7 Specific API Calls
    /// </summary>
    public static class Windows7
    {
        /// <summary>
        /// Calls SetCurrentProcessExplicitAppUserModelID() to set the current process AppID.
        /// </summary>
        /// <param name="appId">128 character or smaller Applcation ID.</param>
        public static void SetProcessAppID(string appId)
        {
            if (!OsFeatureCheck.TaskbarSupported)
                return;

            if (appId.Length > 128)
                throw new ArgumentException("AppID must be 128 characters or less", "appId");

            SafeNativeMethods.SetCurrentProcessExplicitAppUserModelID(appId);
        }
    }
    
    [SuppressUnmanagedCodeSecurity]
    internal static class SafeNativeMethods
    {
        /// <summary>
        /// http://msdn.microsoft.com/en-us/library/dd378422%28VS.85%29.aspx
        /// </summary>
        /// <param name="AppID">AppID string</param>
        [DllImport("shell32.dll")]
        internal static extern void SetCurrentProcessExplicitAppUserModelID([MarshalAs(UnmanagedType.LPWStr)] string AppID);
    }
}
