using System;
using System.Collections.Generic;

namespace EVEMon.Common
{
    /// <summary>
    /// Serializable class for an API method and its path. Each APIConfiguration maintains a list of APIMethods.
    /// </summary>
    public class APIMethod
    {
        private APIMethods m_method;
        private string m_path;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="method"></param>
        /// <param name="path"></param>
        public APIMethod(APIMethods method, string path)
        {
            m_method = method;
            m_path = path;
        }

        /// <summary>
        /// Returns the APIMethods enumeration member for this APIMethod.
        /// </summary>
        public APIMethods Method
        {
            get { return m_method; }
        }

        /// <summary>
        /// Returns the defined URL suffix path for this APIMethod.
        /// </summary>
        public string Path
        {
            get { return m_path; }
            set { m_path = value; }
        }

        /// <summary>
        /// Creates a set of API methods with their default urls.
        /// </summary>
        /// <returns></returns>
        public static IEnumerable<APIMethod> CreateDefaultSet()
        {
            foreach (string methodName in Enum.GetNames(typeof(APIMethods)))
            {
                APIMethods methodEnum = (APIMethods)Enum.Parse(typeof(APIMethods), methodName);
                string methodURL = NetworkConstants.ResourceManager.GetString(String.Format("API{0}", methodName));
                yield return new APIMethod(methodEnum, methodURL);
            }
        }
    }
}
