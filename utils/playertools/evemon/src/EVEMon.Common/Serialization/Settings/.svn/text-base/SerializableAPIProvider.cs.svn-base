using System;
using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Settings
{
    /// <summary>
    /// Represents a serializable version of an API provider. Used for settings persistence.
    /// </summary>
    public sealed class SerializableAPIProvider
    {
        public SerializableAPIProvider()
        {
            Name = "New provider";
            Url = NetworkConstants.APIBase;
            Methods = new List<SerializableAPIMethod>();
            foreach (string methodName in Enum.GetNames(typeof(APIMethods)))
            {
                APIMethods methodEnum = (APIMethods)Enum.Parse(typeof(APIMethods), methodName);
                string methodURL = NetworkConstants.ResourceManager.GetString(methodName);
                Methods.Add(new SerializableAPIMethod { Method = methodEnum, Path = methodURL });
            }
        }

        [XmlElement("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlElement("url")]
        public string Url
        {
            get;
            set;
        }

        [XmlArray("methods")]
        [XmlArrayItem("method")]
        public List<SerializableAPIMethod> Methods
        {
            get;
            set;
        }
    }
}
