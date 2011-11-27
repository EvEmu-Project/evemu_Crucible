using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents the character's list of plans
    /// </summary>
    public sealed class PlanCollection : BaseList<Plan>
    {
        private readonly Character m_owner;

        /// <summary>
        /// Constructor
        /// </summary>
        internal PlanCollection(Character owner)
        {
            m_owner = owner;
        }

        /// <summary>
        /// Gets the plan with the given name, null when not found
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public Plan this[string name]
        {
            get
            {
                foreach (var plan in m_items)
                {
                    if (plan.Name == name) return plan;
                }
                return null;
            }
        }

        /// <summary>
        /// When we add a plan, we may have to clone it (and maybe changed the character it is bound to) and connects it
        /// </summary>
        /// <param name="item"></param>
        protected override void OnAdding(ref Plan item)
        {
            if (item.Character != m_owner)
            {
                item = item.Clone(m_owner);
            }
            else if (this.Contains(item))
            {
                item = item.Clone();
            }

            item.IsConnected = true;
        }

        /// <summary>
        /// When removing a plan, we need to disconnect it
        /// </summary>
        /// <param name="oldItem"></param>
        protected override void OnRemoving(Plan oldItem)
        {
            oldItem.IsConnected = false;
        }

        /// <summary>
        /// When the collection changed, the global event is fired
        /// </summary>
        protected override void OnChanged()
        {
            EveClient.OnCharacterPlanCollectionChanged(m_owner);
        }

        /// <summary>
        /// Imports data from the given deserialization object
        /// </summary>
        /// <param name="plans"></param>
        internal void Import(IEnumerable<SerializablePlan> plans)
        {
            List<Plan> newPlanList = new List<Plan>();

            // We first update existing plans
            foreach (var serialPlan in plans)
            {
                // Filter plans which belong to this owner
                if (serialPlan.Owner != m_owner.Guid) continue;

                // If a plan with the same name already exists, we update it
                var plan = this[serialPlan.Name];

                if (plan != null) plan.Import(serialPlan);
                else plan = new Plan(m_owner, serialPlan);

                newPlanList.Add(plan);
            }

            // We now add the new plans
            m_items.Clear();
            foreach (var plan in newPlanList)
            {
                m_items.Add(plan);
                plan.IsConnected = true;
            }

            // Fire the global event
            EveClient.OnCharacterPlanCollectionChanged(m_owner);
        }
    }
}
