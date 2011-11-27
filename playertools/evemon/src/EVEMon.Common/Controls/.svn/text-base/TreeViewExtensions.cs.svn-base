using System;
using System.Collections;
using System.Collections.Generic;
using System.Windows.Forms;

namespace EVEMon.Common.Controls
{
    public static class TreeViewExtensions
    {
        /// <summary>
        /// Gets an enumerator over all the tree nodes
        /// </summary>
        /// <param name="treeView"></param>
        /// <returns></returns>
        public static IEnumerable<TreeNode> GetAllNodes(this EVEMon.Common.Controls.TreeView treeView)
        {
            Stack<IEnumerator> enumerators = new Stack<IEnumerator>();
            IEnumerator currentEnumerator = treeView.Nodes.GetEnumerator();

            try
            {
                while (true)
                {
                    // Is there still a node at this level ?
                    if (currentEnumerator.MoveNext())
                    {
                        // Retrieve the new current node
                        TreeNode node = (TreeNode)currentEnumerator.Current;

                        // Pushes the enumerator over its children
                        if (node.Nodes.Count != 0)
                        {
                            enumerators.Push(currentEnumerator);
                            currentEnumerator = node.Nodes.GetEnumerator();
                        }

                        // Return this node
                        yield return node;
                    }
                    // Is there a previous level ?
                    else if (enumerators.Count != 0)
                    {
                        // Clean up
                        IDisposable disposable = currentEnumerator as IDisposable;
                        if (disposable != null)
                            disposable.Dispose();

                        // Previous level
                        currentEnumerator = enumerators.Pop();
                    }
                    // Then, we're done
                    else
                    {
                        yield break;
                    }
                }
            }
            finally
            {
                // Clean up the enumerators
                foreach(var e in enumerators) 
                {
                    IDisposable disposable = e as IDisposable;
                    if (disposable != null)
                        disposable.Dispose();
                }
            }
        }

        /// <summary>
        /// Gets an eneumerator over all the tree nodes
        /// </summary>
        /// <param name="treeView"></param>
        /// <returns></returns>
        public static IEnumerable<TreeNode> GetAllVisibleNodes(this EVEMon.Common.Controls.TreeView treeView)
        {
            for (TreeNode node = treeView.TopNode; node != null; node = node.NextVisibleNode)
            {
                yield return node;
            }
        }

        /// <summary>
        /// Retrieves the node with the provided tag.
        /// </summary>
        /// <remarks>Tags are compared using reference comparison, so avoid boxed value types and such.</remarks>
        /// <param name="treeView"></param>
        /// <param name="tag">The tag to search for, using object references comparison</param>
        /// <returns>The matching node if found, null otherwise</returns>
        public static TreeNode GetNodeWithTag(this EVEMon.Common.Controls.TreeView treeView, Object tag)
        {
            foreach (var node in GetAllNodes(treeView))
            {
                if (Object.ReferenceEquals(node.Tag, tag))
                    return node;
            }
            return null;
        }

        /// <summary>
        /// Selects the node with the provided tag, or unselect the current selection if the tag was not found.
        /// </summary>
        /// <remarks>Tags are compared using reference comparison, so avoid boxed value types and such.</remarks>
        /// <param name="treeView"></param>
        /// <param name="tag"></param>
        /// <returns>The selected node, null if this tag was not found</returns>
        public static TreeNode SelectNodeWithTag(this EVEMon.Common.Controls.TreeView treeView, Object tag)
        {
            // Is it already selected ?
            if (treeView.SelectedNode != null && treeView.SelectedNode.Tag == tag)
                return treeView.SelectedNode;

            foreach (var node in GetAllNodes(treeView))
            {
                if (Object.ReferenceEquals(node.Tag, tag))
                {
                    node.EnsureVisible();
                    treeView.UnselectAllNodesExceptNode(node, TreeViewAction.ByMouse);
                    treeView.SelectNode(node, true, TreeViewAction.ByMouse);
                    return node;
                }
            }

            treeView.SelectedNode = null;
            return null;
        }
    }
}
