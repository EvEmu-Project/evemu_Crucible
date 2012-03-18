using System;
using System.Linq;
using System.Collections.Generic;
using System.Text;
using System.Reflection;
using System.ComponentModel;
using EVEMon.Common.Attributes;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    public static class EnumExtensions
    {
        /// <summary>
        /// Gets the description bound to the given enumeration member.
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public static string GetDescription(this Enum item)
        {
            return GetAttribute<DescriptionAttribute>(item).Description;
        }

        /// <summary>
        /// Checks whether the given member has a header.
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public static bool HasHeader(this Enum item)
        {
            return GetAttribute<HeaderAttribute>(item) != null;
        }

        /// <summary>
        /// Gets the header bound to the given enumeration member.
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public static string GetHeader(this Enum item)
        {
            return GetAttribute<HeaderAttribute>(item).Header;
        }

        /// <summary>
        /// Gets the default value bound to the given enumeration member.
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public static object GetDefaultValue(this Enum item)
        {
            return GetAttribute<DefaultValueAttribute>(item).Value;
        }

        /// <summary>
        /// Gets the attribute associated to the given enumeration item.
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public static bool HasAttribute<TAttribute>(this Enum item)
            where TAttribute : Attribute
        {
            return GetAttribute<TAttribute>(item) != null;
        }

        /// <summary>
        /// Gets the attribute associated to the given enumeration item.
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        public static TAttribute GetAttribute<TAttribute>(this Enum item)
            where TAttribute : Attribute
        {
            MemberInfo[] members = item.GetType().GetMember(item.ToString());
            if (members != null && members.Length > 0)
            {
                object[] attrs = members[0].GetCustomAttributes(typeof(TAttribute), false);
                if (attrs != null && attrs.Length > 0)
                {
                    return (TAttribute)attrs[0];
                }
            }
            return null;
        }

        /// <summary>
        /// Gets the values for this enum.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="item"></param>
        /// <returns></returns>
        public static IEnumerable<T> GetValues<T>()
        {
            return Enum.GetValues(typeof(T)).Cast<T>();
        }

        /// <summary>
        /// Gets the values that are powers of two for this flag enum, excluding the one for zero.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="item"></param>
        /// <returns></returns>
        public static IEnumerable<T> GetBaseValues<T>()
        {
            foreach (var value in Enum.GetValues(typeof(T)))
            {
                // Check it matches a power of two 
                bool found = false;
                for (int i = 0; i < 32; i++)
                {
                    if ((int)value == (1 << i))
                    {
                        found = !found;

                        // If two bits matched, found is false again and value is not a power of two.
                        if (!found) break;
                    }
                }

                // Is it a power of two ?
                if (found) yield return (T)value;
            }
        }

    }
}