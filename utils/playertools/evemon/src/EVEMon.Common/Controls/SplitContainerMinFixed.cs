using System;
using System.Collections;
using System.Windows.Forms;
using System.Drawing;

namespace EVEMon.Common.Controls
{
    /// <summary>
    /// Amended version of the standard SplitContainer class to overcome known issue with VS2005 when using panel min size attribute
    /// </summary>
    public class SplitContainerMinFixed : SplitContainer
    {
        #region Private Properties
        private bool    m_sizeSet = false;
        private int     m_panel2MinSize;
        #endregion

        #region Constructor
        public SplitContainerMinFixed()
            : base()
        {
            // Default value
            m_panel2MinSize = base.Panel2MinSize;
        }
        #endregion

        #region Public Properties
        /// <summary>
        /// Property override defers setting base attribute unless size attribute has already been set
        /// </summary>
        public new int Panel2MinSize 
        { 
            get
            {
                if (m_sizeSet)
                    return base.Panel2MinSize;
                return m_panel2MinSize;
            }
            set
            {
                m_panel2MinSize = value;
                if (m_sizeSet)
                    base.Panel2MinSize = m_panel2MinSize;
            }
        }

        /// <summary>
        /// Property override sets base panel2MinSize after base.Size is set
        /// </summary>
        public new Size Size 
        { 
            get {return base.Size;}
            set
            {
                base.Size = value;
                if (!m_sizeSet)
                {
                    m_sizeSet = true;
                    if (m_panel2MinSize != 0)
                        Panel2MinSize = m_panel2MinSize;
                }
            }
        } 
        #endregion
    }
}
