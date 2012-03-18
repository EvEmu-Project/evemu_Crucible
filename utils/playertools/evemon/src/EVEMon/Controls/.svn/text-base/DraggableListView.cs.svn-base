using System;
using System.Collections;
using System.Drawing;
using System.Windows.Forms;

namespace EVEMon.Controls
{
    public class DraggableListView : ListView
    {
        public DraggableListView()
            : base()
        {
            DraggableInit();
        }

        #region Draggable stuff
        private const string REORDER = "Reorder";

        private bool allowRowReorder = true;

        public bool AllowRowReorder
        {
            get { return this.allowRowReorder; }
            set
            {
                this.allowRowReorder = value;
                base.AllowDrop = value;
            }
        }

        public new SortOrder Sorting
        {
            get { return SortOrder.None; }
            set { base.Sorting = SortOrder.None; }
        }

        private void DraggableInit()
        {
            this.AllowRowReorder = true;
        }

        public event EventHandler<ListViewDragEventArgs> ListViewItemsDragging;
        public event EventHandler<EventArgs> ListViewItemsDragged;

        private bool m_dragging = false;

        private EVEMon.Common.Skill GetDraggingSkill(DragEventArgs e)
        {
            if (e.Data.GetDataPresent("System.Windows.Forms.TreeNode"))
            {
                return (EVEMon.Common.Skill)((TreeNode)e.Data.GetData("System.Windows.Forms.TreeNode")).Tag;
            }
            return null;
        }

        protected override void OnDragDrop(DragEventArgs e)
        {
            EVEMon.Common.Skill dragSkill = GetDraggingSkill(e);
            if (dragSkill != null)
            {
                base.OnDragDrop(e);
                return;
            }
            base.OnDragDrop(e);
            m_dragging = false;
            ClearDropMarker();
            if (!this.AllowRowReorder)
            {
                return;
            }
            if (base.SelectedItems.Count == 0)
            {
                return;
            }
            Point cp = base.PointToClient(new Point(e.X, e.Y));
            ListViewItem dragToItem = base.GetItemAt(cp.X, cp.Y);
            if (dragToItem == null)
            {
                return;
            }
            int dropIndex = dragToItem.Index;
            if (dropIndex > base.SelectedItems[0].Index)
            {
                dropIndex++;
            }

            if (ListViewItemsDragging != null)
            {
                ListViewDragEventArgs args = new ListViewDragEventArgs(base.SelectedItems[0].Index,
                                                                       base.SelectedItems.Count, dropIndex);
                ListViewItemsDragging(this, args);
                if (args.Cancel)
                {
                    return;
                }
            }

            ArrayList insertItems = new ArrayList(base.SelectedItems.Count);
            // Make a copy of all the selected items

            foreach (ListViewItem item in base.SelectedItems)
            {
                insertItems.Add(item.Clone());
            }

            // insert the copied items in reverse order at the drop index so 
            // they appear in the right order after they've all been inserted
            for (int i=insertItems.Count-1; i>=0; i--)
            {
                base.Items.Insert(dropIndex, (ListViewItem)insertItems[i]);
            }
            // remove the selected items
            foreach (ListViewItem item in base.SelectedItems)
            {
                // must clear the items icon index or an exception is thrown when it is removed
                item.StateImageIndex = -1;
                base.Items.Remove(item);
            }

            if (ListViewItemsDragged != null)
            {
                ListViewItemsDragged(this, new EventArgs());
            }

            // if the item was dragged to the end of the plan.
            if (dropIndex >= Items.Count)
            {
                base.EnsureVisible(Items.Count - 1);
            }
            else
            {
                base.EnsureVisible(dropIndex);
            }
        }

        protected override void OnDragOver(DragEventArgs e)
        {
            EVEMon.Common.Skill dragSkill = GetDraggingSkill(e);
            if (dragSkill != null)
            {
                base.OnDragOver(e);
                return;
            }
            if (!this.AllowRowReorder)
            {
                e.Effect = DragDropEffects.None;
                ClearDropMarker();
                return;
            }
            if (!e.Data.GetDataPresent(DataFormats.Text))
            {
                e.Effect = DragDropEffects.None;
                ClearDropMarker();
                return;
            }
            Point cp = base.PointToClient(new Point(e.X, e.Y));
            ListViewItem hoverItem = base.GetItemAt(cp.X, cp.Y);
            if (hoverItem == null)
            {
                e.Effect = DragDropEffects.None;
                ClearDropMarker();
                return;
            }
            foreach (ListViewItem moveItem in base.SelectedItems)
            {
                if (moveItem.Index == hoverItem.Index)
                {
                    e.Effect = DragDropEffects.None;
                    hoverItem.EnsureVisible();
                    ClearDropMarker();
                    return;
                }
            }
            base.OnDragOver(e);
            String text = (String)e.Data.GetData(REORDER.GetType());
            if (text.CompareTo(REORDER) == 0)
            {
                e.Effect = DragDropEffects.Move;
                hoverItem.EnsureVisible();

                Rectangle hoverBounds = hoverItem.GetBounds(ItemBoundsPortion.ItemOnly);
                DrawDropMarker(hoverItem.Index, (cp.Y > (hoverBounds.Top + (hoverBounds.Height / 2))));
            }
            else
            {
                e.Effect = DragDropEffects.None;
                ClearDropMarker();
            }
        }

        protected override void OnDragEnter(DragEventArgs e)
        {
            EVEMon.Common.Skill dragSkill = GetDraggingSkill(e);
            if (dragSkill != null)
            {
                base.OnDragEnter(e);
                return;
            }
            base.OnDragEnter(e);
            if (!this.AllowRowReorder)
            {
                e.Effect = DragDropEffects.None;
                ClearDropMarker();
                return;
            }
            if (!e.Data.GetDataPresent(DataFormats.Text))
            {
                e.Effect = DragDropEffects.None;
                ClearDropMarker();
                return;
            }
            base.OnDragEnter(e);
            String text = (String)e.Data.GetData(REORDER.GetType());
            if (text.CompareTo(REORDER) == 0)
            {
                e.Effect = DragDropEffects.Move;
            }
            else
            {
                e.Effect = DragDropEffects.None;
                ClearDropMarker();
            }
        }

        private int m_dropMarkerOn = -1;
        private bool m_dropMarkerBelow = false;

        public void ClearDropMarker()
        {
            if (m_dropMarkerOn != -1)
            {
                this.RestrictedPaint();
            }
            m_dropMarkerOn = -1;
        }

        public void DrawDropMarker(int index, bool below)
        {
            if (m_dropMarkerOn != -1 && (m_dropMarkerOn != index || m_dropMarkerBelow != below))
            {
                ClearDropMarker();
            }
            if (m_dropMarkerOn != index)
            {
                m_dropMarkerOn = index;
                m_dropMarkerBelow = below;
                this.RestrictedPaint();
            }
        }

        private void RestrictedPaint()
        {
            Rectangle itemRect = base.GetItemRect(m_dropMarkerOn, ItemBoundsPortion.ItemOnly);
            Point start;
            Point end;
            start = new Point(itemRect.Left, (m_dropMarkerBelow ? itemRect.Bottom : itemRect.Top));
            end = new Point((this.Width < itemRect.Right ? this.Width : itemRect.Right), (m_dropMarkerBelow ? itemRect.Bottom : itemRect.Top));
            start = this.PointToScreen(start);
            end = this.PointToScreen(end);
            ControlPaint.DrawReversibleLine(start, end, SystemColors.Window);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            if (m_dragging)
            {
                RestrictedPaint();
            }
        }

        protected override void OnItemDrag(ItemDragEventArgs e)
        {
            base.OnItemDrag(e);
            if (!this.AllowRowReorder)
            {
                return;
            }
            base.DoDragDrop(REORDER, DragDropEffects.Move);
            m_dragging = true;
        }
        #endregion
    }

    public class ListViewDragEventArgs : EventArgs
    {
        private int m_movingFrom;

        public int MovingFrom
        {
            get { return m_movingFrom; }
        }

        private int m_movingCount;

        public int MovingCount
        {
            get { return m_movingCount; }
        }

        private int m_movingTo;

        public int MovingTo
        {
            get { return m_movingTo; }
        }

        private bool m_cancel = false;

        public bool Cancel
        {
            get { return m_cancel; }
            set { m_cancel = value; }
        }

        internal ListViewDragEventArgs(int from, int count, int to)
        {
            m_movingFrom = from;
            m_movingCount = count;
            m_movingTo = to;
        }
    }
}