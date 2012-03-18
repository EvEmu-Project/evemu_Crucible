using System.Collections.Generic;
using System.Text;

using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents an item category
    /// </summary>
    public class MarketGroup
    {
        private readonly MarketGroupCollection m_subCategories;
        private readonly ItemCollection m_items;
        private readonly MarketGroup m_parent;
        private readonly string m_name;
        private readonly long m_id;


        #region Constructors

        /// <summary>
        /// Deserialization constructor for root category only
        /// </summary>
        /// <param name="src">Source Serializable Market Group</param>
        public MarketGroup(SerializableMarketGroup src)
        {
            m_id = src.ID;
            m_name = src.Name;
            m_subCategories = new MarketGroupCollection(this, src.SubGroups);
            m_items = new ItemCollection(this, src.Items);
        }

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="parent">The Market Group this Market Group is contained within</param>
        /// <param name="src">Source Serializable Market Group</param>
        public MarketGroup(MarketGroup parent, SerializableMarketGroup src)
            : this(src)
        {
            m_parent = parent;
        }

        /// <summary>
        /// Deserialization constructor for root category only
        /// </summary>
        /// <param name="src"></param>
        public MarketGroup(SerializableBlueprintGroup src)
        {
            m_id = src.ID;
            m_name = src.Name;
        }

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="parent">The Market Group this Market Group is contained within</param>
        /// <param name="src">Source Blueprint Group</param>
        public MarketGroup(MarketGroup parent, SerializableBlueprintGroup src)
            :this (src)
        {
            m_parent = parent;
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets the group ID.
        /// </summary>
        public long ID
        {
            get { return m_id; }
        }

        /// <summary>
        /// Gets the parent category. <c>Null</c> for the root category.
        /// </summary>
        public MarketGroup ParentGroup
        {
            get { return m_parent; }
        }

        /// <summary>
        /// Gets the sub categories.
        /// </summary>
        public MarketGroupCollection SubGroups
        {
            get { return m_subCategories; }
        }

        /// <summary>
        /// Gets the items in this category.
        /// </summary>
        public ItemCollection Items
        {
            get { return m_items; }
        }

        /// <summary>
        /// Gets this category's name.
        /// </summary>
        public string Name
        {
            get { return m_name; }
        }

        /// <summary>
        /// Gets this category full category name.
        /// </summary>
        public string FullCategoryName
        {
            get
            {
                StringBuilder fullPathName = new StringBuilder();
                MarketGroup parent = m_parent;

                fullPathName.Append(m_name);
                
                while (parent != null)
                {
                    if (parent != null)
                        fullPathName.Insert(0, " > ");
                    fullPathName.Insert(0, parent.Name);
                    parent = parent.ParentGroup;
                }

                return fullPathName.ToString();
            }
        }

        /// <summary>
        /// Gets the collection of all the items in this category and its descendants.
        /// </summary>
        public IEnumerable<Item> AllItems
        {
            get
            {
                foreach (var item in m_items)
                {
                    yield return item;
                }

                foreach (var cat in m_subCategories)
                {
                    foreach (var subItem in cat.AllItems)
                    {
                        yield return subItem;
                    }
                }
            }
        }

        #endregion


        #region Overidden Methods

        /// <summary>
        /// Gets the name of this item.
        /// </summary>
        /// <returns>Name of the Market Group.</returns>
        public override string ToString()
        {
            return m_name;
        }

        #endregion

    }
}
