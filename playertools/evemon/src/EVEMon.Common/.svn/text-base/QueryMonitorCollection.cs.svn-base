using System;
using System.Linq;
using System.Collections.Generic;
using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    public sealed class QueryMonitorCollection : ReadonlyCollection<IQueryMonitor>
    {
        /// <summary>
        /// Gets the monitor for the given query.
        /// </summary>
        /// <param name="method"></param>
        /// <returns></returns>
        public IQueryMonitor this[APIMethods method]
        {
            get 
            {
                foreach (var monitor in m_items)
                {
                    if (monitor.Method == method) return monitor;
                }
                return null;
            }
        }

        /// <summary>
        /// Gets true when at least one of the monitors is being updated.
        /// </summary>
        public bool AnyUpdating
        {
            get
            {
                return m_items.Any(x => x.IsUpdating);
            }
        }

        /// <summary>
        /// Gets true when at least one of the monitors encountered an error on last try.
        /// </summary>
        public bool HasErrors
        {
            get
            {
                return m_items.Any(x => x.LastResult != null && x.LastResult.HasError);
            }
        }

        /// <summary>
        /// Gets the last API results gotten.
        /// </summary>
        public IEnumerable<IAPIResult> APIResults
        {
            get
            {
                return m_items.Where(x => x.LastResult != null).Select(x => x.LastResult);
            }
        }

        /// <summary>
        /// Gets the list of monitors to be auto-updated, ordered from the earliest to the latest.
        /// </summary>
        public IEnumerable<IQueryMonitor> OrderedByUpdateTime
        {
            get
            {
                // Caches the times and sort
                var times = m_items.Select(x => x.NextUpdate).ToArray();
                var monitors = m_items.ToArray();
                Array.Sort(times, monitors);

                // Returns the monitors which are planned for an autoupdate
                for (int i = 0; i < monitors.Length; i++)
                {
                    IQueryMonitorEx monitor = (IQueryMonitorEx)monitors[i];
                    if (monitor.Status == QueryStatus.Pending || monitor.Status == QueryStatus.Updating)
                    {
                        yield return monitors[i];
                    }
                }

                // Returns the monitors which won't autoupdate
                for (int i = 0; i < monitors.Length; i++)
                {
                    IQueryMonitorEx monitor = (IQueryMonitorEx)monitors[i];
                    if (monitor.Status != QueryStatus.Pending && monitor.Status != QueryStatus.Updating)
                    {
                        yield return monitors[i];
                    }
                }

            }
        }

        /// <summary>
        /// Gets the next query to be auto-updated, or null.
        /// </summary>
        public IQueryMonitor NextUpdate
        {
            get
            {
                DateTime nextTime = DateTime.MaxValue;
                IQueryMonitor nextMonitor = null;
                foreach (IQueryMonitorEx monitor in m_items)
                {
                    if (monitor.Status == QueryStatus.Pending || monitor.Status == QueryStatus.Updating)
                    {
                        var monitorNextTime = monitor.NextUpdate;
                        if (monitorNextTime < nextTime)
                        {
                            nextMonitor = monitor;
                            nextTime = monitorNextTime;
                        }
                    }
                }
                return nextMonitor;
            }
        }

        /// <summary>
        /// Requests an update for the given method.
        /// </summary>
        /// <param name="method"></param>
        public void Query(APIMethods method)
        {
            var monitor = this[method] as IQueryMonitorEx;
            monitor.ForceUpdate(false);
        }

        /// <summary>
        /// Requests an update from CCP now.
        /// </summary>
        public void QueryEverything()
        {
            foreach (IQueryMonitorEx monitor in m_items)
            {
                monitor.ForceUpdate(false);
            }
        }

        /// <summary>
        /// Checks for auto-updates for every monitor in this collection.
        /// </summary>
        /// <param name="monitor"></param>
        internal void UpdateOnOneSecondTick()
        {
            foreach (IQueryMonitorEx monitor in m_items)
            {
                monitor.UpdateOnOneSecondTick();
            }
        }

        /// <summary>
        /// Adds this monitor to the collection.
        /// </summary>
        /// <param name="monitor"></param>
        internal void Add(IQueryMonitorEx monitor)
        {
            m_items.Add(monitor);
        }
    }
}
