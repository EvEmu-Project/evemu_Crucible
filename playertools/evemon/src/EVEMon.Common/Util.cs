using System;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Xml;
using System.Xml.Serialization;
using System.Xml.Xsl;

using EVEMon.Common.Net;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Threading;

namespace EVEMon.Common
{
    /// <summary>
    /// A delegate for downloads.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="result"></param>
    /// <param name="errorMessage"></param>
    public delegate void DownloadCallback<T>(T result, string errorMessage);

    /// <summary>
    /// A collection of helper methods for downloads and deserialization.
    /// </summary>
    public static class Util
    {
        /// <summary>
        /// Opens the provided url in a new process.
        /// </summary>
        /// <param name="url"></param>
        public static void OpenURL(string url)
        {
            try
            {
                Process.Start(url);
            }
            catch (Exception ex)
            {
                ExceptionHandler.LogException(ex, true);
            }
        }

        /// <summary>
        /// Loads an XSL transform with the provided name from the resources.
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        public static XslCompiledTransform LoadXSLT(string content)
        {
            XslCompiledTransform xslt = new XslCompiledTransform();
            using (var stringReader = new StringReader(content))
            {
                using (XmlTextReader reader = new XmlTextReader(stringReader))
                {
                    xslt.Load(reader);
                }
            }

            return xslt;
        }

        /// <summary>
        /// Deserializes an XML, returning null when exceptions occur.
        /// </summary>
        /// <typeparam name="T">The type to deserialize</typeparam>
        /// <param name="filename">The file to deserialize from</param>
        /// <returns>The deserialized object when success, <c>null</c> otherwise.</returns>
        public static T DeserializeXML<T>(string filename)
            where T : class
        {
            return DeserializeXML<T>(filename, null);
        }

        /// <summary>
        /// Process XML document.
        /// </summary>
        /// <typeparam name="T">The type to deserialize from the document</typeparam>
        /// <param name="filename">The XML document to deserialize from.</param>
        /// <param name="transform">The XSL transformation to apply. May be <c>null</c>.</param>
        /// <returns>The result of the deserialization.</returns>
        public static T DeserializeXML<T>(string filename, XslCompiledTransform transform)
            where T : class
        {
            try
            {
                if (transform != null)
                {
                    using (MemoryStream stream = new MemoryStream())
                    {
                        using (XmlTextWriter writer = new XmlTextWriter(stream, Encoding.UTF8))
                        {
                            // Apply the XSL transform
                            writer.Formatting = Formatting.Indented;
                            transform.Transform(filename, writer);
                            writer.Flush();

                            // Deserialize from the given stream
                            stream.Seek(0, SeekOrigin.Begin);
                            XmlSerializer xs = new XmlSerializer(typeof(T));
                            return (T)xs.Deserialize(stream);
                        }
                    }
                }
                // Deserialization without transform
                else
                {
                    using (var stream = FileHelper.OpenRead(filename, false))
                    {
                        XmlSerializer xs = new XmlSerializer(typeof(T));
                        return (T)xs.Deserialize(stream);
                    }
                }
            }
            // An error occurred during the XSL transform
            catch (XsltException exc)
            {
                ExceptionHandler.LogException(exc, true);
                return null;
            }
            // An error occurred during the deserialization
            catch (InvalidOperationException exc)
            {
                ExceptionHandler.LogException(exc, true);
                return null;
            }
            catch (XmlException exc)
            {
                ExceptionHandler.LogException(exc, true);
                return null;
            }
        }

        /// <summary>
        /// Deserializes a datafile.
        /// </summary>
        /// <typeparam name="T">The type to deserialize from the datafile</typeparam>
        /// <param name="filename">The datafile name</param>
        /// <returns></returns>
        internal static T DeserializeDatafile<T>(string filename)
        {
            // Gets the full path
            string path = Datafile.GetFullPath(filename);

            try
            {
                // Deserializes
                using (var s = FileHelper.OpenRead(path, false))
                {
                    using (GZipStream zs = new GZipStream(s, CompressionMode.Decompress))
                    {
                        XmlSerializer xs = new XmlSerializer(typeof(T));
                        return (T)xs.Deserialize(zs);
                    }
                }
            }
            catch (InvalidOperationException ex)
            {
                String message = String.Format(CultureConstants.DefaultCulture,
                    "An error occurred decompressing {0}, the error message was '{1}' from '{2}'. "
                    + "Try deleting all of the xml.gz files in %APPDATA%\\EVEMon.", filename, ex.Message, ex.Source);
                throw new ApplicationException(message, ex);
            }
            catch (XmlException ex)
            {
                String message = String.Format(CultureConstants.DefaultCulture,
                    "An error occurred reading the XML from {0}, the error message was '{1}' from '{2}'. "
                    + "Try deleting all of the xml.gz files in %APPDATA%\\EVEMon.", filename, ex.Message, ex.Source);
                throw new ApplicationException(message, ex);
            }
        }

        /// <summary>
        /// Deserialize an XML from a file.
        /// </summary>
        /// <typeparam name="T">The inner type to deserialize</typeparam>
        /// <param name="transform">The XSL transform to apply, may be null.</param>
        /// <returns>The deserialized result</returns>
        internal static APIResult<T> DeserializeAPIResult<T>(string filename, XslCompiledTransform transform)
        {
            try
            {
                XmlDocument doc = new XmlDocument();
                doc.Load(filename);
                return DeserializeAPIResultCore<T>(transform, doc);
            }
            catch (XmlException exc)
            {
                ExceptionHandler.LogException(exc, true);
                return new APIResult<T>(exc);
            }
        }

        /// <summary>
        /// Asynchronously download an XML and deserializes it into the specified type.
        /// </summary>
        /// <typeparam name="T">The inner type to deserialize</typeparam>
        /// <param name="url">The url to query</param>
        /// <param name="postData">The HTTP POST data to send, may be null.</param>
        /// <param name="transform">The XSL transform to apply, may be null.</param>
        /// <param name="callback">The callback to call once the query has been completed. It will be done on the data actor (see <see cref="DataObject.CommonActor"/>).</param>
        internal static void DownloadAPIResultAsync<T>(string url, HttpPostData postData, XslCompiledTransform transform, QueryCallback<T> callback)
        {
            EveClient.HttpWebService.DownloadXmlAsync(url, postData, (asyncResult, userState) =>
            {
                APIResult<T> result;

                // Was there an HTTP error ?
                if (asyncResult.Error != null)
                {
                    result = new APIResult<T>(asyncResult.Error);
                }
                // No http error, let's try to deserialize
                else
                {
                    result = DeserializeAPIResultCore<T>(transform, asyncResult.Result);
                }

                // We got the result, let's invoke the callback on this actor
                Dispatcher.Invoke(() => callback.Invoke(result));
            },
                null);
        }

        /// <summary>
        /// Synchronously download an XML and deserializes it into the specified type.
        /// </summary>
        /// <typeparam name="T">The inner type to deserialize</typeparam>
        /// <param name="url">The url to query</param>
        /// <param name="postData">The HTTP POST data to send, may be null.</param>
        /// <param name="transform">The XSL transform to apply, may be null.</param>
        internal static APIResult<T> DownloadAPIResult<T>(string url, HttpPostData postData, XslCompiledTransform transform)
        {
            APIResult<T> result = new APIResult<T>(APIEnumerations.APIErrors.Http, String.Format("Time out on querying {0}", url));

            // Query async and wait
            using (var wait = new EventWaitHandle(false, EventResetMode.AutoReset))
            {
                EveClient.HttpWebService.DownloadXmlAsync(url, postData, (asyncResult, userState) =>
                {
                    try
                    {
                        // Was there an HTTP error ?
                        if (asyncResult.Error != null)
                        {
                            result = new APIResult<T>(asyncResult.Error);
                        }
                        // No http error, let's try to deserialize
                        else
                        {
                            result = DeserializeAPIResultCore<T>(transform, asyncResult.Result);
                        }
                    }
                    catch (Exception e)
                    {
                        ExceptionHandler.LogException(e, true);
                        result = new APIResult<T>(APIEnumerations.APIErrors.Http, e.Message);
                    }
                    finally
                    {
                        // We got the result, so we resume the calling thread
                        wait.Set();
                    }
                },
                    null);

                // Wait for the completion of the background thread
                wait.WaitOne();
            }

            // Returns
            return result;
        }

        /// <summary>
        /// Process XML document.
        /// </summary>
        /// <typeparam name="T">The type to deserialize from the document</typeparam>
        /// <param name="transform">The XSL transformation to apply. May be <c>null</c>.</param>
        /// <param name="doc">The XML document to deserialize from.</param>
        /// <returns>The result of the deserialization.</returns>
        private static APIResult<T> DeserializeAPIResultCore<T>(XslCompiledTransform transform, XmlDocument doc)
        {
            APIResult<T> result = null;

            try
            {
                // Deserialization with a transform
                using (XmlNodeReader reader = new XmlNodeReader(doc))
                {
                    if (transform != null)
                    {
                        using (MemoryStream stream = new MemoryStream())
                        {
                            using (XmlTextWriter writer = new XmlTextWriter(stream, Encoding.UTF8))
                            {
                                // Apply the XSL transform
                                writer.Formatting = Formatting.Indented;
                                transform.Transform(reader, writer);
                                writer.Flush();

                                // Deserialize from the given stream
                                stream.Seek(0, SeekOrigin.Begin);
                                XmlSerializer xs = new XmlSerializer(typeof(APIResult<T>));
                                result = (APIResult<T>)xs.Deserialize(stream);
                            }
                        }
                    }
                    // Deserialization without transform
                    else
                    {
                        XmlSerializer xs = new XmlSerializer(typeof(APIResult<T>));
                        result = (APIResult<T>)xs.Deserialize(reader);
                    }
                }

                // Fix times
                DateTime requestTime = DateTime.UtcNow;
                double CCPOffset = (result.CurrentTime.Subtract(requestTime)).TotalMilliseconds;
                result.SynchronizeWithLocalClock(CCPOffset);
            }
            // An error occurred during the XSL transform
            catch (XsltException exc)
            {
                ExceptionHandler.LogException(exc, true);
                result = new APIResult<T>(exc);
            }
            // An error occurred during the deserialization
            catch (InvalidOperationException exc)
            {
                ExceptionHandler.LogException(exc, true);
                result = new APIResult<T>(exc);
            }
            catch (XmlException exc)
            {
                ExceptionHandler.LogException(exc, true);
                result = new APIResult<T>(exc);
            }

            // Stores XMLDocument
            result.XmlDocument = doc;
            return result;
        }

        /// <summary>
        /// Asynchronously download an object from an XML stream.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="url">The url to download from</param>
        /// <param name="postData">The http POST data to pass with the url. May be null.</param>
        /// <param name="callback">A callback invoked on the UI thread</param>
        /// <returns></returns>
        public static void DownloadXMLAsync<T>(string url, HttpPostData postData, DownloadCallback<T> callback)
            where T : class
        {
            EveClient.HttpWebService.DownloadXmlAsync(url, postData,

            // Callback
            (asyncResult, userState) =>
            {
                T result = null;
                string errorMessage = String.Empty;

                // Was there an HTTP error ??
                if (asyncResult.Error != null)
                {
                    errorMessage = asyncResult.Error.Message;
                }
                // No http error, let's try to deserialize
                else
                {
                    try
                    {
                        // Deserialize
                        using (XmlNodeReader reader = new XmlNodeReader(asyncResult.Result))
                        {
                            XmlSerializer xs = new XmlSerializer(typeof(T));
                            result = (T)xs.Deserialize(reader);
                        }
                    }
                    // An error occurred during the deserialization
                    catch (InvalidOperationException exc)
                    {
                        ExceptionHandler.LogException(exc, true);
                        errorMessage = (exc.InnerException == null ? exc.Message : exc.InnerException.Message);
                    }
                }

                // We got the result, let's invoke the callback on this actor
                Dispatcher.Invoke(() => callback.Invoke(result, errorMessage));
            },
            null);
        }

        /// <summary>
        /// Gets a nicely formatted string representation of a XML document.
        /// </summary>
        /// <param name="doc"></param>
        /// <returns></returns>
        public static string GetXMLStringRepresentation(XmlDocument doc)
        {
            // Creates the settings for the text writer
            XmlWriterSettings settings = new XmlWriterSettings();
            settings.Indent = true;
            settings.NewLineHandling = NewLineHandling.Replace;

            // Writes to a string builder
            StringBuilder xmlBuilder = new StringBuilder();
            XmlWriter xmlWriter = XmlWriter.Create(xmlBuilder, settings);
            doc.WriteContentTo(xmlWriter);
            xmlWriter.Flush();

            return xmlBuilder.ToString();
        }

        /// <summary>
        /// Serializes the given object to a XML document, dealt as the provided type.
        /// </summary>
        /// <param name="serializationType">The type to pass to the <see cref="XmlSerializer"/></param>
        /// <param name="data">The object to serialize.</param>
        /// <returns>The Xml document representing the given object.</returns>
        public static XmlDocument SerializeToXmlDocument(Type serializationType, object data)
        {
            using (var memStream = new MemoryStream())
            {
                // Serializes to the stream
                var serializer = new XmlSerializer(serializationType);
                serializer.Serialize(memStream, data);

                // Creates a XML doc from the stream
                memStream.Seek(0, System.IO.SeekOrigin.Begin);
                XmlDocument doc = new XmlDocument();
                doc.Load(memStream);

                return doc;
            }
        }

        /// <summary>
        /// Applies a XSLT to a <see cref="XmlDocument"/> and returns another <see cref="XmlDocument"/>.
        /// </summary>
        /// <param name="doc">The source <see cref="XmlDocument"/></param>
        /// <param name="transform">The XSLT to apply.</param>
        /// <returns>The transformed <see cref="XmlDocument"/>.</returns>
        public static XmlDocument Transform(XmlDocument doc, XslCompiledTransform transform)
        {
            using (MemoryStream stream = new MemoryStream())
            {
                using (XmlTextWriter writer = new XmlTextWriter(stream, Encoding.UTF8))
                {
                    // Apply the XSL transform
                    writer.Formatting = Formatting.Indented;
                    transform.Transform(doc, writer);
                    writer.Flush();

                    // Reads the XML document from the given stream.
                    stream.Seek(0, SeekOrigin.Begin);
                    XmlDocument outDoc = new XmlDocument();
                    outDoc.Load(stream);
                    return outDoc;
                }
            }
        }

        /// <summary>
        /// Opens a file and look for a "revision" attribute and return its value.
        /// </summary>
        /// <param name="filename"></param>
        /// <returns>The revision number of the assembly which generated this file,
        /// or <c>0</c> if no such file was found (old format, before the introduction of the revision numbers).</returns>
        public static int GetRevisionNumber(string filename)
        {
            // Uses a regex to retrieve the revision number.
            string content = File.ReadAllText(filename);
            Regex regex = new Regex("revision=\"([0-9]+)\"", RegexOptions.Compiled);
            var match = regex.Match(content);

            // No match ? Then there was no "revision" attribute, this is an old format.
            if (!match.Success || match.Groups.Count < 2)
                return 0;

            // Returns the revision number (first group is the whole match, the second one the capture)
            int revision = 0;
            Int32.TryParse(match.Groups[1].Value, out revision);
            return revision;
        }

        /// <summary>
        /// Uncompress the given gzipped file to a temporary file and returns its filename.
        /// </summary>
        /// <param name="filename"></param>
        /// <returns>The temporary file's name.</returns>
        public static string UncompressToTempFile(String filename)
        {
            string tempFile = Path.GetTempFileName();

            // We decompress the gzipped stream and writes it to a temporary file
            using (var stream = File.OpenRead(filename))
            {
                using (var gzipStream = new GZipStream(stream, CompressionMode.Decompress))
                {
                    using (var outStream = File.OpenWrite(tempFile))
                    {
                        byte[] bytes = new byte[4096];

                        // Since we're reading a compressed stream, the total number of bytes to decompress cannot be foreseen
                        // So we just continue reading until there were bytes to decompress
                        while (true)
                        {
                            int count = gzipStream.Read(bytes, 0, bytes.Length);
                            if (count == 0) break;

                            outStream.Write(bytes, 0, count);
                        }

                        // Done, we flush and recall this method with the temp file name
                        outStream.Flush();
                        outStream.Close();
                    }
                }
            }
            return tempFile;
        }

        /// <summary>
        /// Gets the XML Root Element for the specified XML File.
        /// </summary>
        /// <remarks>
        /// After doing some testing, this is the fastest robust
        /// mechanism for getting the root node. This takes 480 ticks
        /// as opposed to > 900 for XmlDocument methods.
        /// </remarks>
        /// <param name="filename">Filename of an XmlDocument</param>
        /// <returns>Text representation of the root node</returns>
        public static string GetXmlRootElement(string filename)
        {
            if (!File.Exists(filename))
                throw new FileNotFoundException("Document not found", filename);

            XmlTextReader reader = new XmlTextReader(filename);
            reader.XmlResolver = null;

            while (reader.Read())
            {
                if (reader.NodeType == XmlNodeType.Element)
                    return reader.Name;
            }

            return null;
        }
    }
}
