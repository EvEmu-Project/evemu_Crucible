using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.ComponentModel;
using EVEMon.Controls.Design;

namespace EVEMon.Controls
{
    /// <summary>
    /// A page of the <see cref="MultiPanel"/> control.
    /// </summary>
    /// <remarks>
    /// Based on the work from Liron Levi on Code Project, under public domain. 
    /// See http://www.codeproject.com/KB/cs/multipanelcontrol.aspx
    /// </remarks>
    [Designer(typeof(MultiPanelPageDesigner))]
    public class MultiPanelPage : ContainerControl
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        public MultiPanelPage()
        {
            base.Dock = DockStyle.Fill;
        }

        /// <summary>
        /// Gets <see cref="DockStyle.Fill"/>. Sets is available but always set <see cref="DockStyle.Fill"/>.
        /// </summary>
        public override DockStyle Dock
        {
            get { return base.Dock; }
            set { base.Dock = DockStyle.Fill; }
        }

        /// <summary>
        /// Only here so that it shows up in the property panel.
        /// </summary>
        [Category("Design")]
        [Description("The text identifying the page.")]
        public override string Text
        {
            get { return base.Text; }
            set { base.Text = value; }
        }

        /// <summary>
        /// Overriden. Creates the underlying controls collection.
        /// </summary>
        /// <returns>A <see cref="MultiPanelPage.ControlCollection"/>.</returns>
        protected override Control.ControlCollection CreateControlsInstance()
        {
            return new PageControlCollection(this);
        }

        #region ControlCollection
        /// <summary>
        /// A control collection when ensures only <see cref="MultiPagePanel"/> are added
        /// </summary>
        private sealed class PageControlCollection : Control.ControlCollection
        {
            /// <summary>
            /// Constructor. 
            /// </summary>
            public PageControlCollection(Control owner)
                : base(owner)
            {
                // Should not happen
                if (owner == null)
                {
                    throw new ArgumentNullException("owner", "Tried to create a MultiPanelPage.ControlCollection with a null owner.");
                }

                // Should not happen
                MultiPanelPage c = owner as MultiPanelPage;
                if (c == null)
                {
                    throw new ArgumentException("Tried to create a MultiPanelPage.ControlCollection with a non-MultiPanelPage owner.", "owner");
                }
            }

            /// <summary>
            /// Adds an item to the control. Ensures it is a <see cref="MultiPagePanel"/>
            /// </summary>
            public override void Add(Control value)
            {
                if (value == null)
                {
                    throw new ArgumentNullException("value", "Tried to add a null value to the MultiPanelPage.ControlCollection.");
                }

                MultiPanelPage p = value as MultiPanelPage;
                if (p != null)
                {
                    throw new ArgumentException("Tried to add a MultiPanelPage control to the MultiPanelPage.ControlCollection.", "value");
                }

                base.Add(value);
            }
        }
        #endregion

    }
}
