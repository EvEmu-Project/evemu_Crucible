using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Serialization;
using System.Xml.Xsl;

using EVEMon.Common.Serialization;

namespace EVEMon.Common
{
    public static class EveNotificationType
    {
        private static SerializableNotificationRefTypeIDs s_notificationTypes;
        private static bool s_isLoaded;


        /// <summary>
        /// Gets the description of the notification type.
        /// </summary>
        /// <param name="typeID">The type ID.</param>
        /// <returns></returns>
        internal static string GetType(int typeID)
        {
            EnsureInitialized();

            var type = s_notificationTypes.Types.FirstOrDefault(x => x.TypeID == typeID);
            if (type != null)
                return type.TypeName;

            return "Unknown";
        }

        /// <summary>
        /// Ensures the notification types data have been intialized.
        /// </summary>
        private static void EnsureInitialized()
        {
            if (s_isLoaded)
                return;

            // Read the resource file
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(Properties.Resources.NotificationRefTypeIDs);
            
            // Read the nodes
            using (XmlNodeReader reader = new XmlNodeReader(xmlDoc))
            {
                // Create a memory stream to transform the xml 
                using (MemoryStream stream = new MemoryStream())
                {
                    // Write the xml output to the stream
                    using (XmlTextWriter writer = new XmlTextWriter(stream, Encoding.UTF8))
                    {
                        // Apply the XSL transform
                        XslCompiledTransform transform = Util.LoadXSLT(Properties.Resources.RowsetsXSLT);
                        writer.Formatting = Formatting.Indented;
                        transform.Transform(reader, writer);
                        writer.Flush();

                        // Deserialize from the given stream
                        stream.Seek(0, SeekOrigin.Begin);
                        XmlSerializer xs = new XmlSerializer(typeof(SerializableNotificationRefTypeIDs));
                        s_notificationTypes = (SerializableNotificationRefTypeIDs)xs.Deserialize(stream);
                    }
                }
            }

            s_isLoaded = true;
        }

    }
}
