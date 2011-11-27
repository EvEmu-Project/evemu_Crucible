using System;
using System.Collections;
using System.Windows.Forms;
using EVEMon.Common;
using System.ComponentModel;

using DescriptionAttribute = System.ComponentModel.DescriptionAttribute;

namespace EVEMon.Controls
{
    public class PersistentSplitContainer : SplitContainer
    {
        public PersistentSplitContainer()
            : base()
        {
        }
        
        private string m_rememberDistanceKey;

        /// <summary>
        /// A key used to store position for this control.
        /// Do not set up directly in the designer, call it from the control's constructor, after InitializeComponent().
        /// </summary>
        [Browsable(false)]
        public string RememberDistanceKey
        {
            get { return m_rememberDistanceKey; }
            set 
            {
                m_rememberDistanceKey = value;
                // Set the splitter width here rather than in an override of CreateControl()
                // because CreatControl is only called when we make the container visible
                // so if the container is created, but never shown, the persistant splitter 
                // width will be reset to the default for the base SplitContainer
                try
                {
                    if (!String.IsNullOrEmpty(m_rememberDistanceKey))
                    {
                        if (Settings.UI.Splitters.ContainsKey(m_rememberDistanceKey))
                        {
                            int d = Settings.UI.Splitters[m_rememberDistanceKey];
                            d = this.VerifyValidSplitterDistance(d);
                            this.SplitterDistance = d;
                        }
                        else
                        {
                            Settings.UI.Splitters.Add(m_rememberDistanceKey, Math.Min(this.Width / 4, 100));
                        }
                    }
                }
                catch (Exception err)
                {
                    // This occurs when we're in the designer. DesignMode doesn't get set
                    // when the control is a subcontrol of a user control, so we should handle
                    // this here :(
                    ExceptionHandler.LogException(err, true);
                    return;
                }

            }
        }

       protected override void Dispose(bool disposing)
        {
            if (!String.IsNullOrEmpty(m_rememberDistanceKey))
            {
                int d = this.SplitterDistance;
                if (VerifyValidSplitterDistance(d) == d)
                {
                    Settings.UI.Splitters[m_rememberDistanceKey] = d;
                }
            }

            base.Dispose(disposing);
        }

        private int VerifyValidSplitterDistance(int d)
        {
            int defaultDistance = this.SplitterDistance;

            if ((d < this.Panel1MinSize) || (d + this.Panel2MinSize > this.Width))
                return defaultDistance;
            else
                return d;
        }
    }
}