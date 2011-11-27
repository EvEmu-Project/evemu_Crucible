using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents the 
    /// </summary>
    public enum PlanSort
    {
        Name = 0,
        Time = 1, 
        SkillsCount = 2
    }

    /// <summary>
    /// Implements a plan comparer
    /// </summary>
    public sealed class PlanComparer : Comparer<Plan>
    {
        private SortOrder m_order;
        private PlanSort m_sort;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="psw"></param>
        /// <param name="ci"></param>
        public PlanComparer(PlanSort sort)
        {
            m_sort = sort;
        }

        /// <summary>
        /// Sort order (ascending, descending)
        /// </summary>
        public SortOrder Order
        {
            get { return m_order; }
            set { m_order = value; }
        }

        /// <summary>
        /// Sort criteria
        /// </summary>
        public PlanSort Sort
        {
            get { return m_sort; }
            set { m_sort = value; }
        }

        /// <summary>
        /// Comparison function
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <returns></returns>
        public override int Compare(Plan x, Plan y)
        {
            // Swap variables for descending order
            if (m_order == SortOrder.Descending)
            {
                var tmp = y;
                y = x;
                x = tmp;
            }

            // Compare plans
            switch (m_sort)
            {
                case PlanSort.Name:
                    return String.Compare(x.Name, y.Name);

                case PlanSort.Time:
                    var xtime = x.GetTotalTime(null, true);
                    var ytime = y.GetTotalTime(null, true);
                    return TimeSpan.Compare(xtime, ytime);

                case PlanSort.SkillsCount:
                    return x.UniqueSkillsCount - y.UniqueSkillsCount;

                default:
                    throw new NotImplementedException();
            }
        }
    }
}
