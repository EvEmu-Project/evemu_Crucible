using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.BattleClinic
{
    public sealed class SerializableFilesListItem
    {
        [XmlAttribute("id")]
        public int FileID;

        [XmlAttribute("name")]
        public string FileName;

        [XmlAttribute("updated")]
        public string FileUpdatedXml
        {
            get { return FileUpdated.DateTimeToTimeString(); }
            set
            {
                if (!String.IsNullOrEmpty(value))
                    FileUpdated = value.TimeStringToDateTime();
            }
        }

        [XmlAttribute("revision")]
        public int FileRevision;

        [XmlText]
        public string FileContent;

        [XmlIgnore]
        public DateTime FileUpdated;
    }
}