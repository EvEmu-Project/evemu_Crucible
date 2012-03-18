// (C) 2007 Jay Beavers (mailto:Jay@HikingHomeschoolers.Org), All Rights Reserved.
// Submitted by Jay Beavers to EVEMon as an enhancement, permission to include in EVEMon and use appropriately.
// Submission does not constitute release of Copyright Ownership.

using System;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;

namespace EVEMon.Common
{
    /// <summary>
    /// The purpose of AutoShrink is to provide a simple way for long running applications to reduce their memory footprint when idle.
    /// This is especially useful for applications that run out of the system tray and occasionally make/receive network connections
    /// as part of their functionality.
    /// </summary>
    /// <example>
    /// DoSomeNetworkOperation();
    /// AutoShrink.Dirty();
    /// GoIdle();
    /// </example>
    /// <see cref="http://www.hikinghomeschoolers.org/Jay/Wiki/AutoShrink.aspx"/>
    public static class AutoShrink
    {
        /// <summary>
        /// 65 seconds not work for you?  Change it here.
        /// </summary>
        /// <remarks>
        /// Typically this should not be too short.  You want the .NET classes you call, which may be using ThreadPool threads
        /// or timers themselves, to have a chance to "run out" before the cleanup occurs.  If you leave this value long, you
        /// can also call Dirty() early on and be assured that the cleanup won't take place until your operation is completed.
        /// 
        /// A good reason to change this value would be to lengthen it to handle longer running operations.  For instance, if your
        /// app takes 5 minutes to service incoming network calls typically then you might want to increase this timer to say 7-10
        /// minutes.
        /// 
        /// A default value of 65 seconds was chosen because Windows Communications Foundation times out network connections after 60
        /// seconds.  By calling Dirty() at the start of a network operation, we're most likely going to clean up after WCF has
        /// completed processing either the successful call or the timeout.
        /// </remarks>
        private static TimeSpan m_idleMillisecondsBeforeClean = new TimeSpan(0, 0, 0, 0, 65000);
        private static readonly TimeSpan m_infinite = new TimeSpan(0, 0, 0, 0, -1);
        private static readonly Timer m_dirtyTimer = new Timer(DirtyCallback);

        /// <summary>
        /// Call this method after an operation has occurred that might have increased the working set of the application
        /// temporarily (e.g. making or servicing a network call).  Calling this after an operation that allocates and holds
        /// onto memory (e.g. loading an ADO.NET Table from SQL) will not have much effect as that memory isn't available
        /// to be Collected/Released.
        /// </summary>
        /// <remarks>
        /// This method has a built-in timer to "dampen" multiple rapid calls to this method.  This means that you can safely
        /// place the call inside a "frequently called" submethod (like Connection.Close) and memory clean will not occur until
        /// calls to Dirty() stop coming in.  The timer is also by default set to a large value (65 seconds) so you don't have
        /// to be terribly precise about when you call it.  For instance, if your operation completes within 65 seconds you
        /// could simply call Dirty() at Operation.Start() and then the cleanup wouldn't occur until 65 seconds later after the
        /// operation had finished.
        /// </remarks>
        public static void Dirty()
        {
            m_dirtyTimer.Change(m_idleMillisecondsBeforeClean, m_infinite);
        }

        /// <summary>
        /// Call this method after an operation has occurred that might have increased the working set of the application
        /// temporarily (e.g. making or servicing a network call).  Calling this after an operation that allocates and holds
        /// onto memory (e.g. loading an ADO.NET Table from SQL) will not have much effect as that memory isn't available
        /// to be Collected/Released.
        /// </summary>
        /// <remarks>
        /// This method has a built-in timer to "dampen" multiple rapid calls to this method.  This means that you can safely
        /// place the call inside a "frequently called" submethod (like Connection.Close) and memory clean will not occur until
        /// calls to Dirty() stop coming in.
        ///
        /// This overload allows you to specify timeout so you can use a shorter or longer timeout for "well known behavior"
        /// situations like minimizing the window to systray.
        /// </remarks>
        /// <param name="idleMillisecondsBeforeClean">
        /// TimeSpan to wait for Dirty calls to dampen (e.g. time to pass without another call to dirty) before cleaning memory.
        /// </param>
        public static void Dirty(TimeSpan idleMillisecondsBeforeClean)
        {
            m_dirtyTimer.Change(idleMillisecondsBeforeClean, m_infinite);
        }

        /// <summary>
        /// Forces an immediate cleanup
        /// </summary>
        public static void DirtyImmediate()
        {
            DirtyCallback(null);
            m_dirtyTimer.Change(m_infinite, m_infinite);
        }

        private static void DirtyCallback(object state)
        {
            // These operations are very expensive and should be performed rarely.  One of the ironies of GC.Collect is that it touches
            // the memory of *every single* managed object in a program as part of it's "can I get rid of you now?" algorithm.
            // Ergo, if a piece of memory in a managed app was paged out into disk, GC.Collect will force it to be reloaded.
            // This is why if you want to manually "trim" your memory using GC.Collect, afterwards you should call
            // SetProcessWorkingSetSize to trim the memory pages that were brought in only for the GC interrogation.
            GC.Collect();

            // Not only collect, also wait for the finalizers routines to be run so that all the memory churn is finished before we
            // trim the working set.
            GC.WaitForPendingFinalizers();

            // Performs the same operation that Windows does upon "minimize window".  This releases all memory pages not currently in use
            // which greatly reduces the amount of RAM that a managed application take up when idle.
            if (Environment.OSVersion.Platform == PlatformID.Win32NT)
            {
                SetProcessWorkingSetSize(Process.GetCurrentProcess().Handle, -1, -1);
            }
        }

        [DllImport("kernel32.dll")]
        private static extern bool SetProcessWorkingSetSize(IntPtr proc, int min, int max);
    }
}
