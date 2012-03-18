using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a product of the reprocessing.
    /// </summary>
    public sealed class Material
    {
        private readonly Item m_product;
        private readonly int m_quantity;

        public Material(SerializableMaterialQuantity src)
        {
            m_product = StaticItems.GetItemByID(src.ID);
            m_quantity = src.Quantity;
        }

        /// <summary>
        /// Gets the reprocessing product.
        /// </summary>
        public Item Product
        {
            get { return m_product; }
        }

        /// <summary>
        /// Gets the produced quantity.
        /// </summary>
        public int Quantity
        {
            get { return m_quantity; }
        }
    }
}
