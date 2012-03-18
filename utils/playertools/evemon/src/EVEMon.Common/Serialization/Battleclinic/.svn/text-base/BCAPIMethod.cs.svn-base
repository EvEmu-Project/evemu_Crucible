using System;
using System.Collections.Generic;

namespace EVEMon.Common.Serialization.BattleClinic
{
    /// <summary>
    /// Serializable class for a Battleclinic's API method and its path.
    /// Each BCAPIConfiguration maintains a list of Battleclinic's APIMethods.
    /// </summary>
    public class BCAPIMethod
    {
        private BCAPIMethods m_method;
        private string m_path;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="method"></param>
        /// <param name="path"></param>
        public BCAPIMethod(BCAPIMethods method, string path)
        {
            m_method = method;
            m_path = path;
        }

        /// <summary>
        /// Returns the BCAPIMethods enumeration member for this Battleclinic's APIMethod.
        /// </summary>
        public BCAPIMethods Method
        {
            get { return m_method; }
        }

        /// <summary>
        /// Returns the defined URL suffix path for this BCAPIMethod.
        /// </summary>
        public string Path
        {
            get { return m_path; }
            set { m_path = value; }
        }

        /// <summary>
        /// Creates a set of Battleclinic's API methods with their default urls.
        /// </summary>
        /// <returns></returns>
        public static IEnumerable<BCAPIMethod> CreateDefaultSet()
        {
            foreach (string methodName in Enum.GetNames(typeof(BCAPIMethods)))
            {
                BCAPIMethods methodEnum = (BCAPIMethods)Enum.Parse(typeof(BCAPIMethods), methodName);
                string methodURL = NetworkConstants.ResourceManager.GetString(String.Format("BCAPI{0}", methodName));
                yield return new BCAPIMethod(methodEnum, methodURL);
            }
        }
    }
}
