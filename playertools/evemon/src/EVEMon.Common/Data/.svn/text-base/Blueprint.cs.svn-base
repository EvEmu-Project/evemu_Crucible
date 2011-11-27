using System;
using System.Collections.Generic;
using System.Text;

using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    public class Blueprint : Item
    {
        protected readonly int m_maxProductionLimit;
        protected readonly short m_produceItemID;
        protected readonly int m_productionTime;
        protected readonly int m_productivityModifier;
        protected readonly int m_researchCopyTime;
        protected readonly int m_researchMaterialTime;
        protected readonly int m_researchProductivityTime;
        protected readonly int m_researchTechTime;
        protected readonly int m_techLevel;
        protected readonly short m_wasteFactor;

        protected readonly FastList<int> m_inventBlueprint;
        protected readonly FastList<StaticRequiredMaterial> m_materialRequirements;


        #region Constructors

        /// <summary>
        /// Deserialization constructor.
        /// </summary>
        /// <param name="group">Market Group the Blueprint will be a member of.</param>
        /// <param name="src">Source serializable blueprint.</param>
        internal Blueprint(BlueprintMarketGroup group, SerializableBlueprint src)
            : base(group, src)
        {

            m_maxProductionLimit = src.MaxProductionLimit;
            m_produceItemID = src.ProduceItemID;
            m_productionTime= src.ProductionTime;
            m_productivityModifier = src.ProductivityModifier;
            m_researchCopyTime = src.ResearchCopyTime;
            m_researchMaterialTime = src.ResearchMaterialTime;
            m_researchProductivityTime = src.ResearchProductivityTime;
            m_researchTechTime = src.ResearchTechTime;
            m_techLevel = src.TechLevel;
            m_wasteFactor = src.WasteFactor;

            // Invented blueprint
            m_inventBlueprint = new FastList<int>(src.InventionTypeID != null ? src.InventionTypeID.Length : 0);
            if (src.InventionTypeID != null)
            {
                foreach (var blueprintID in src.InventionTypeID)
                {
                    m_inventBlueprint.Add(blueprintID);
                }
            }

            // Materials prerequisites
            m_materialRequirements = new FastList<StaticRequiredMaterial>(src.ReqMaterial != null ? src.ReqMaterial.Length : 0);
            if (src.ReqMaterial == null)
                return;
            
            foreach (var prereq in src.ReqMaterial)
            {
                m_materialRequirements.Add(new StaticRequiredMaterial(prereq));
            }
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets the item this blueprint produces.
        /// </summary>
        public Item ProducesItem
        {
            get { return StaticItems.GetItemByID(m_produceItemID); }
        }

        /// <summary>
        /// Gets the maximum items this blueprint can produce.
        /// </summary>
        public int RunsPerCopy
        {
            get { return m_maxProductionLimit; }
        }

        /// <summary>
        /// Gets the production time of the blueprint.
        /// </summary>
        public double ProductionTime
        {
            get { return m_productionTime; }
        }

        /// <summary>
        /// Gets the production modifier of the blueprint.
        /// </summary>
        public double ProductivityModifier
        {
            get { return m_productivityModifier; }
        }

        /// <summary>
        /// Gets the copying time of the blueprint.
        /// </summary>
        public double ResearchCopyTime
        {
            get { return m_researchCopyTime * 2; }
        }

        /// <summary>
        /// Gets the material efficiency research time of the blueprint.
        /// </summary>
        public double ResearchMaterialTime
        {
            get { return m_researchMaterialTime; }
        }

        /// <summary>
        /// Gets the productivity efficiency research time of the blueprint.
        /// </summary>
        public double ResearchProductivityTime
        {
            get { return m_researchProductivityTime; }
        }

        /// <summary>
        /// Gets the invention time of the blueprint.
        /// </summary>
        public double ResearchTechTime
        {
            get { return m_researchTechTime; }
        }

        /// <summary>
        /// Gets the tech level of the blueprint.
        /// </summary>
        public int TechLevel
        {
            get { return m_techLevel; }
        }

        /// <summary>
        /// Gets the wastage factor of the blueprint.
        /// </summary>
        public short WasteFactor
        {
            get { return (m_wasteFactor); }
        }

        /// <summary>
        /// Gets the collection of materials this blueprint must satisfy to be build.
        /// </summary>
        public IEnumerable<StaticRequiredMaterial> MaterialRequirements
        {
            get { return m_materialRequirements; }
        }

        /// <summary>
        /// Gets the collection of blueprints this object can invent.
        /// </summary>
        public List<Blueprint> InventsBlueprint
        {
            get
            {
                var list = new List<Blueprint>();
                foreach (var itemID in m_inventBlueprint)
                {
                    list.Add(StaticBlueprints.GetBlueprintByID(itemID));
                }
                return list;
            }
        }

        #endregion

    }
}
