using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common.Attributes
{
    public sealed class UpdateAttribute : Attribute
    {

        #region Constructors

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="defaultPeriod">Default length of time between updates.</param>
        /// <param name="min">Minimum length of time between updates.</param>
        /// <param name="cacheStyle">Cache style.</param>
        public UpdateAttribute(UpdatePeriod defaultPeriod, UpdatePeriod min, CacheStyle cacheStyle)
        {
            CreateUpdateAttribute(defaultPeriod, min, UpdatePeriod.Week, cacheStyle);
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="defaultPeriod">Default length of time between updates.</param>
        /// <param name="min">Minimum length of time between updates.</param>
        /// <param name="max">Maximum length of time between updates.</param>
        /// <param name="cacheStyle">Cache style.</param>
        public UpdateAttribute(UpdatePeriod defaultPeriod, UpdatePeriod min, UpdatePeriod max, CacheStyle cacheStyle)
        {
            CreateUpdateAttribute(defaultPeriod, min, max, cacheStyle);
        }

        /// <summary>
        /// Constructor helper method.
        /// </summary>
        /// <param name="defaultPeriod">Default length of time between updates.</param>
        /// <param name="min">Minimum length of time between updates.</param>
        /// <param name="max">Maximum length of time between updates.</param>
        /// <param name="cacheStyle">Cache style.</param>
        private void CreateUpdateAttribute(UpdatePeriod defaultPeriod, UpdatePeriod min, UpdatePeriod max, CacheStyle cacheStyle)
        {
            this.DefaultPeriod = defaultPeriod;
            this.Minimum = min;
            this.Maximum = max;
            this.CacheStyle = cacheStyle;
        }

        #endregion


        #region Public Properties

        public UpdatePeriod DefaultPeriod
        {
            get;
            set;
        }

        public UpdatePeriod Minimum
        {
            get;
            set;
        }

        public UpdatePeriod Maximum
        {
            get;
            set;
        }

        public CacheStyle CacheStyle
        {
            get;
            set;
        }

        #endregion

    }
}
