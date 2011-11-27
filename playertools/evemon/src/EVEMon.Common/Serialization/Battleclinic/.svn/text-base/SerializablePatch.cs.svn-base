using System.Xml.Serialization;
using System.Collections.Generic;

namespace EVEMon.Common.Serialization.BattleClinic
{
    [XmlRoot("evemon")]
    public sealed class SerializablePatch
    {
        private List<SerializableDatafile> m_changedDataFiles = new List<SerializableDatafile>();

        public SerializablePatch()
        {
            Release = new SerializableRelease();
            Datafiles = new List<SerializableDatafile>();
        }
        
        internal List<SerializableDatafile> ChangedDataFiles
        {
            get { return m_changedDataFiles; }
        }

        [XmlElement("newest")]
        public SerializableRelease Release
        {
            get;
            set;
        }

        [XmlArray("datafiles")]
        [XmlArrayItem("datafile")]
        public List<SerializableDatafile> Datafiles
        {
            get;
            set;
        }

        [XmlIgnore]
        internal bool FilesHaveChanged
        {
            get
            {
                m_changedDataFiles.Clear();

                foreach (SerializableDatafile dfv in Datafiles)
                {
                    foreach (var datafile in EveClient.Datafiles)
                    {
                        if (datafile.Filename == dfv.Name)
                        {
                            if (datafile.MD5Sum != dfv.MD5Sum)
                                m_changedDataFiles.Add(dfv);

                            break;
                        }
                    }
                }

                return ChangedDataFiles.Count > 0;
            }
        }
    }
}
