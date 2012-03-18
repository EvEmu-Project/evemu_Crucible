using System;
using System.Drawing;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace EVEMon.Controls
{
    /// <summary>
    /// A tab control which support drag and dropping.
    /// </summary>
    public class DraggableTabControl : TabControl
    {
        private int m_markerIndex;
        private bool m_markerOnLeft;
        private Point m_lastPoint = new Point();
        private InsertionMarker m_marker;

        /// <summary>
        /// Constructor.
        /// </summary>
        public DraggableTabControl()
        {
            m_markerIndex = -1;
            m_marker = new InsertionMarker(this);
            AllowDrop = true;
        }

        /// <summary>
        /// On size changing, we repaint, otherwise there are artifacts when a
        /// notification is closed.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnSizeChanged(EventArgs e)
        {
            base.OnSizeChanged(e);
            Invalidate();
        }

        /// <summary>
        /// On disposing, we close the marker and disposes it.
        /// </summary>
        /// <param name="disposing"></param>
        protected override void Dispose(bool disposing)
        {
            m_marker.Hide();
            m_marker.Dispose();
            base.Dispose(disposing);
        }

        /// <summary>
        /// On dragging, we updates the cursor and displays an insertion marker.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragOver(DragEventArgs e)
        {
            base.OnDragOver(e);
            var draggedTab = GetDraggedTab(e);

            // Retrieve the point in client coordinates
            Point pt = new Point(e.X, e.Y);
            pt = PointToClient(pt);
            if (pt.Equals(m_lastPoint)) return;
            m_lastPoint = pt;

            // Make sure there is a TabPage being dragged.
            if (draggedTab == null)
            {
                e.Effect = DragDropEffects.None;
                return;
            }

            // Retrieve the dragged page. If same as dragged page, return
            var hoveredTab = GetTabPageAt(pt);
            if (draggedTab == hoveredTab)
            {
                e.Effect = DragDropEffects.None;
                m_markerIndex = -1;
                return;
            }

            // Get the old and new marker indices
            bool onLeft;
            int newIndex = GetMarkerIndex(draggedTab, pt, out onLeft);
            var oldIndex = m_markerIndex;
            m_markerIndex = newIndex;
            m_markerOnLeft = onLeft;

            // Updates the new tab index
            if (oldIndex != newIndex)
            {
                UpdateMarker();
            }

            e.Effect = DragDropEffects.Move;
        }

        /// <summary>
        /// On drag and drop, we updates the tab order.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragDrop(DragEventArgs e)
        {
            var draggedTab = GetDraggedTab(e);

            m_lastPoint = new Point(Int32.MaxValue, Int32.MaxValue);
            m_markerIndex = -1;
            UpdateMarker();

            // Retrieve the point in client coordinates
            Point pt = new Point(e.X, e.Y);
            pt = PointToClient(pt);

            // Get the tab we are hovering over.
            bool onLeft;
            int markerIndex = GetMarkerIndex(draggedTab, pt, out onLeft);
            int index = GetInsertionIndex(markerIndex, onLeft);

            // Make sure there is a TabPage being dragged.
            if (draggedTab == null)
            {
                e.Effect = DragDropEffects.None;
                base.OnDragDrop(e);
                return;
            }

            // Retrieve the dragged page. If same as dragged page, return
            int draggedIndex = TabPages.IndexOf(draggedTab);
            if (draggedIndex == index || (draggedIndex == index - 1 && onLeft))
            {
                e.Effect = DragDropEffects.None;
                base.OnDragDrop(e);
                return;
            }

            // Move the tabs
            e.Effect = DragDropEffects.Move;
            SuspendLayout();
            try
            {
                if (TabPages.IndexOf(draggedTab) < index) index--;
                TabPages.Remove(draggedTab);
                TabPages.Insert(index, draggedTab);
                SelectedTab = draggedTab;
            }
            finally
            {
                ResumeLayout();
                base.OnDragDrop(e);
            }
        }

        /// <summary>
        /// On drag and drop leave, removes the marker index.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragLeave(EventArgs e)
        {
            if (m_marker.Bounds.Contains(Cursor.Position)) return;
            m_markerIndex = -1;
            UpdateMarker();
            base.OnDragLeave(e);
        }

        /// <summary>
        /// Get the dragged tab
        /// </summary>
        /// <returns></returns>
        private static TabPage GetDraggedTab(DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(typeof(TabPage))) return null;
            return (TabPage)e.Data.GetData(typeof(TabPage));
        }

        /// <summary>
        /// Gets the insertion index from the given point.
        /// </summary>
        /// <param name="pt"></param>
        /// <returns></returns>
        private int GetMarkerIndex(TabPage draggedPage, Point pt, out bool onLeft)
        {
            TabPage hoveredPage = GetTabPageAt(pt);
            if (hoveredPage == null)
            {
                // Is it on the left or the right side of this control ?
                if (pt.X < Width / 2)
                {
                    onLeft = true;
                    return 0;
                }
                else
                {
                    onLeft = false;
                    return TabCount;
                }
            }

            // So we're over a page, retrieves its index.
            int newIndex = TabPages.IndexOf(hoveredPage);

            // Is it on the left or the right side of the tab ?
            var rect = GetTabRect(newIndex);
            onLeft = (pt.X < (rect.Left + rect.Right) / 2);
            if (onLeft) return newIndex;

            // If there is a tab on the right, we may put the burden on it
            if (newIndex + 1 >= TabCount) return newIndex;
            var nextPage = GetTabPageAt(new Point(rect.Right + 1, pt.Y));

            if (nextPage != null && nextPage != draggedPage)
            {
                onLeft = true;
                newIndex++;
            }

            return newIndex;
        }

        /// <summary>
        /// Gets the insertion index.
        /// </summary>
        /// <param name="markerIndex"></param>
        /// <param name="onLeft"></param>
        /// <returns></returns>
        private static int GetInsertionIndex(int markerIndex, bool onLeft)
        {
            if (markerIndex == -1) return 0;
            if (onLeft) return markerIndex;
            return markerIndex + 1;
        }

        /// <summary>
        /// When the user moves the mouse with the left button pressed, we do a drag and drop operation.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);

            if ((e.Button & MouseButtons.Left) == MouseButtons.Left)
            {
                TabPage tp = SelectedTab;

                if (tp != null)
                {
                    DoDragDrop(tp, DragDropEffects.All);
                }
            }
        }

        /// <summary>
        /// Finds the TabPage whose tab is contains the given point.
        /// </summary>
        /// <param name="pt">The point (given in client coordinates) to look for a TabPage.</param>
        /// <returns>The TabPage whose tab is at the given point (null if there isn't one).</returns>
        private TabPage GetTabPageAt(Point pt)
        {
            TabPage tp = null;

            for (int i = 0; i < TabPages.Count; i++)
            {
                if (GetTabRect(i).Contains(pt))
                {
                    tp = TabPages[i];
                    break;
                }
            }

            return tp;
        }

        /// <summary>
        /// Updates the insertion marker.
        /// </summary>
        private void UpdateMarker()
        {
            if (m_markerIndex == -1)
            {
                m_marker.Visible = false;
                return;
            }

            var rect = GetTabRect(m_markerIndex);
            rect.Height -= 1;
            rect.X += 1;
            rect.Y += 1;

            var topLeft = PointToScreen(new Point(rect.Left, rect.Top));
            var topRight = PointToScreen(new Point(rect.Right, rect.Top));

            m_marker.Reversed = !m_markerOnLeft;
            m_marker.ShowInactiveTopmost();

            if (m_markerOnLeft)
            {
                m_marker.SetBounds(topLeft.X, topLeft.Y, 5, rect.Height);
            }
            else
            {
                m_marker.SetBounds(topRight.X - 7, topRight.Y, 5, rect.Height);
            }
        }


        #region InsertionMarker
        /// <summary>
        /// A window displaying the insertion marker.
        /// </summary>
        private class InsertionMarker : Form
        {
            private bool m_reversed;
            private DraggableTabControl m_owner;

            /// <summary>
            /// Constructor.
            /// </summary>
            public InsertionMarker(DraggableTabControl owner)
            {
                m_owner = owner;

                SetStyle(ControlStyles.UserPaint, true);
                SetStyle(ControlStyles.AllPaintingInWmPaint, true);
                SetStyle(ControlStyles.OptimizedDoubleBuffer, true);

                Opacity = 0.5;
                FormBorderStyle = FormBorderStyle.None;
                ShowInTaskbar = false;
                AllowDrop = true;
                Height = 16;
                Width = 6;
            }

            /// <summary>
            /// gets or sets the gradent shoudl be reversed.
            /// </summary>
            public bool Reversed
            {
                get { return m_reversed; }
                set
                {
                    if (m_reversed == value) return;
                    m_reversed = value;
                    Invalidate();
                }
            }

            /// <summary>
            /// Performs the painting.
            /// </summary>
            /// <param name="e"></param>
            protected override void OnPaint(PaintEventArgs e)
            {
                Rectangle rect = ClientRectangle;
                var startColor = Color.FromArgb(0, 148, 255);
                var endColor = Color.FromArgb(0, 255, 255);

                // Computes the marker rectangle and the gradient
                if (m_reversed)
                {
                    var tempColor = startColor;
                    startColor = endColor;
                    endColor = tempColor;
                }

                // Draws the marker rectangle
                using (var brush = new LinearGradientBrush(new Point(0, 0), new Point(Width, 0), startColor, endColor))
                {
                    e.Graphics.FillRectangle(brush, rect);
                }
            }

            /// <summary>
            /// Raises the <see cref="E:System.Windows.Forms.Control.DragEnter"/> event.
            /// </summary>
            /// <param name="e">A <see cref="T:System.Windows.Forms.DragEventArgs"/> that contains the event data.</param>
            protected override void OnDragEnter(DragEventArgs e)
            {
                m_owner.OnDragEnter(e);
                base.OnDragEnter(e);
            }

            /// <summary>
            /// Raises the <see cref="E:System.Windows.Forms.Control.DragDrop"/> event.
            /// </summary>
            /// <param name="e">A <see cref="T:System.Windows.Forms.DragEventArgs"/> that contains the event data.</param>
            protected override void OnDragDrop(DragEventArgs e)
            {
                m_owner.OnDragDrop(e);
                base.OnDragDrop(e);
            }

            /// <summary>
            /// Raises the <see cref="E:System.Windows.Forms.Control.DragOver"/> event.
            /// </summary>
            /// <param name="e">A <see cref="T:System.Windows.Forms.DragEventArgs"/> that contains the event data.</param>
            protected override void OnDragOver(DragEventArgs e)
            {
                m_owner.OnDragOver(e);
                base.OnDragOver(e);
            }

            /// <summary>
            /// Raises the <see cref="E:System.Windows.Forms.Control.DragLeave"/> event.
            /// </summary>
            /// <param name="e">An <see cref="T:System.EventArgs"/> that contains the event data.</param>
            protected override void OnDragLeave(EventArgs e)
            {
                Point pt = m_owner.PointToClient(Cursor.Position);
                if (m_owner.ClientRectangle.Contains(pt)) return;

                m_owner.OnDragLeave(e);
                base.OnDragLeave(e);
            }
        }
        #endregion

    }
}
