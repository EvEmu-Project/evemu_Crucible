using System;
using System.Collections.Generic;
using System.Text;

using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    public sealed class BlueprintMarketGroup : MarketGroup
    {
        private readonly BlueprintMarketGroupCollection m_subCategories;
        private readonly BlueprintCollection m_blueprints;
        
        #region Constructors

        /// <summary>
        /// Deserialization constructor for root category only.
        /// </summary>
        /// <param name="src"></param>
        public BlueprintMarketGroup(SerializableBlueprintGroup src)
            :base(src)
        {
            m_subCategories = new BlueprintMarketGroupCollection(this, src.SubGroups);
            m_blueprints = new BlueprintCollection(this, src.Blueprints);
        }

        /// <summary>
        /// Deserialization constructor.
        /// </summary>
        /// <param name="src"></param>
        public BlueprintMarketGroup(BlueprintMarketGroup parent, SerializableBlueprintGroup src)
            : base(parent, src)
        {
            m_subCategories = new BlueprintMarketGroupCollection(this, src.SubGroups);
            m_blueprints = new BlueprintCollection(this, src.Blueprints);
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets the sub categories.
        /// </summary>
        public new BlueprintMarketGroupCollection SubGroups
        {
            get { return m_subCategories; }
        }

        /// <summary>
        /// Gets the blueprints in this category.
        /// </summary>
        public BlueprintCollection Blueprints
        {
            get { return m_blueprints; }
        }

        /// <summary>
        /// Gets the collection of all the blueprints in this category and its descendants.
        /// </summary>
        public IEnumerable<Blueprint> AllBlueprints
        {
            get
            {
                foreach (var blueprint in m_blueprints)
                {
                    yield return blueprint;
                }

                foreach (var cat in m_subCategories)
                {
                    foreach (var subBlueprint in cat.AllBlueprints)
                    {
                        yield return subBlueprint;
                    }
                }
            }
        }

        #endregion

    }
}
