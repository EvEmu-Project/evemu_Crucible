using System;
using System.Linq;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;
using EVEMon.Common.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class SkillPieChartSettings
    {
        public SkillPieChartSettings()
        {
            Colors = new List<SerializableColor>();
        }

        [XmlElement("sortBySize")]
        public bool SortBySize
        {
            get;
            set;
        }

        [XmlElement("mergeMinorGroups")]
        public bool MergeMinorGroups
        {
            get;
            set;
        }

        [XmlElement("sliceHeight")]
        public float SliceHeight
        {
            get;
            set;
        }

        [XmlElement("initialAngle")]
        public float InitialAngle
        {
            get;
            set;
        }

        [XmlElement("colors")]
        public List<SerializableColor> Colors
        {
            get;
            set;
        }

        internal SkillPieChartSettings Clone()
        {
            var clone = new SkillPieChartSettings();
            clone.Colors.AddRange(this.Colors.Select(x => x.Clone()));
            clone.MergeMinorGroups = this.MergeMinorGroups;
            clone.InitialAngle = this.InitialAngle;
            clone.SliceHeight = this.SliceHeight;
            clone.SortBySize = this.SortBySize;
            return clone;
        }
    }
}
