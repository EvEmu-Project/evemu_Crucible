using System;

namespace EVEMon.Sales
{
    /// <summary>
    /// Defines a class as a mineral parser, with a name.
    /// </summary>
    public class DefaultMineralParserAttribute : Attribute
    {
        private string m_name = String.Empty;

        /// <summary>
        /// Gets or sets the name.
        /// </summary>
        /// <value>The name.</value>
        public string Name
        {
            get { return m_name; }
            set { m_name = value; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="DefaultMineralParserAttribute"/> class.
        /// </summary>
        /// <param name="name">The name.</param>
        public DefaultMineralParserAttribute(string name)
        {
            m_name = name;
        }
    }
}
