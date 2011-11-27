using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents a serializable version of an API method. Used for settings persistence.
    /// </summary>
    public sealed class SerializableAPIMethod
    {
        [XmlAttribute("code")]
        public APIMethods Method
        {
            get;
            set;
        }

        [XmlAttribute("path")]
        public string Path
        {
            get;
            set;
        }
    }
}
