using System;
using System.Collections.Generic;
using System.Text;

using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    public class StaticRequiredMaterial : Item
    {

        #region Constructors

        /// <summary>
        /// Deserialization constructor.
        /// </summary>
        /// <param name="id"></param>
        /// <param name="quantity"></param>
        /// <param name="dmgPerJob"></param>
        /// <param name="activityId"></param>
        internal StaticRequiredMaterial(SerializableRequiredMaterial src)
            : base(src.ID,  GetName(src.ID))
        {
            this.Quantity = src.Quantity;
            this.DamagePerJob = src.DamagePerJob;
            this.Activity = (BlueprintActivity)Enum.ToObject(typeof(BlueprintActivity), src.Activity);
            this.WasteAffected = Convert.ToBoolean(src.WasteAffected);
        }

        #endregion


        #region Private Finders

        private static string GetName(int id)
        {
            Item item = StaticItems.GetItemByID(id);

            return (item != null ? item.Name : String.Empty);
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets or sets the quantity.
        /// </summary>
        public int Quantity
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the damage per job.
        /// </summary>
        public double DamagePerJob
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the activity.
        /// </summary>
        public BlueprintActivity Activity
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets if waste affected.
        /// </summary>
        public bool WasteAffected
        {
            get;
            set;
        }

        #endregion

    }
}