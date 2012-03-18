using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class PlanSorting
    {
        public PlanSorting()
        {
            Criteria = PlanEntrySort.None;
            Order = ThreeStateSortOrder.None;
        }

        [XmlAttribute("criteria")]
        public PlanEntrySort Criteria
        {
            get;
            set;
        }

        [XmlAttribute("order")]
        public ThreeStateSortOrder Order
        {
            get;
            set;
        }

        [XmlAttribute("groupByPriority")]
        public bool GroupByPriority
        {
            get;
            set;
        }

        internal PlanSorting Clone()
        {
            return (PlanSorting)MemberwiseClone();
        }
    }

    public enum ThreeStateSortOrder
    {
        None = 0,
        Ascending = 1,
        Descending = 2
    };
}
