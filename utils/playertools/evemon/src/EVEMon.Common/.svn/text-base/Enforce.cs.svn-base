using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace EVEMon.Common
{
    /// <summary>
    /// Helper class for enforcing specific arguments, or object states.
    /// </summary>
    [DebuggerStepThrough]
    public static class Enforce
    {
        /// <summary>
        /// Enforce Argument check.
        /// </summary>
        /// <param name="check">if set to <c>true</c> [check].</param>
        /// <param name="argumentName">Name of the argument.</param>
        /// <param name="checkName">Name of the check.</param>
        public static void Argument(bool check, string argumentName, string checkName)
        {
            if (!check)
            {
                throw new ArgumentException("Check: " + checkName + " failed", argumentName);
            }
        }

        /// <summary>
        /// Enforce Argument the not null.
        /// </summary>
        /// <param name="argument">The argument.</param>
        /// <param name="argumentName">Name of the argument.</param>
        public static void ArgumentNotNull(object argument, string argumentName)
        {
            if (argument == null)
            {
                throw new ArgumentNullException(argumentName);
            }
        }

        /// <summary>
        /// Enforce Argument the not null or empty.
        /// </summary>
        /// <param name="argument">The argument.</param>
        /// <param name="argumentName">Name of the argument.</param>
        public static void ArgumentNotNullOrEmpty(string argument, string argumentName)
        {
            if (String.IsNullOrEmpty(argument))
            {
                throw new ArgumentNullException(argumentName);
            }
        }

        /// <summary>
        /// Enforce value not the null.
        /// </summary>
        /// <param name="value">The value.</param>
        public static void NotNull(object value)
        {
            if (value == null)
            {
                throw new InvalidOperationException("value is null");
            }
        }

        /// <summary>
        /// Nots the null.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <param name="message">The message.</param>
        public static void NotNull(object value, string message)
        {
            if (value == null)
            {
                throw new InvalidOperationException(message);
            }
        }

        /// <summary>
        /// Enforce value not the null.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <param name="name">The name.</param>
        public static void NotNullOrEmpty(string value, string name)
        {
            if (String.IsNullOrEmpty(value))
            {
                throw new InvalidOperationException(name + " is null");
            }
        }

        /// <summary>
        /// Enforce value not the null.
        /// </summary>
        /// <param name="value">The value.</param>
        public static void NotNullOrEmpty(string value)
        {
            if (String.IsNullOrEmpty(value))
            {
                throw new InvalidOperationException("value is null");
            }
        }


        /// <summary>
        /// Enforces the specified check.
        /// </summary>
        /// <param name="check">if set to <c>true</c> [check].</param>
        /// <param name="checkName">Name of the check.</param>
        public static void That(bool check, string checkName)
        {
            if (!check)
            {
                throw new InvalidOperationException("Check: " + checkName + " failed");
            }
        }

        /// <summary>
        /// Implementses the specified obj.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj">The obj.</param>
        /// <returns></returns>
        public static T Implementation<T>(object obj) where T : class
        {
            return Enforce.Implementation<T>(obj, String.Empty);
        }

        /// <summary>
        /// Implementses the specified obj.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj">The obj.</param>
        /// <param name="message">The message.</param>
        /// <returns></returns>
        public static T Implementation<T>(object obj, string message) where T : class
        {
            T cast = obj as T;

            if (cast == null)
            {
                message = string.IsNullOrEmpty(message) ? string.Concat(obj, " must implement ", typeof(T)) : message;
                throw new InvalidCastException(message);
            }

            return cast;
        }
    }
}
