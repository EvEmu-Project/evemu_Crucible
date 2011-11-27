using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.BattleClinic
{
    public sealed class SerializableBCAPIFiles
    {
        [XmlArray("files")]
        [XmlArrayItem("file")]
        public List<SerializableFilesListItem> Files;
    }
}
