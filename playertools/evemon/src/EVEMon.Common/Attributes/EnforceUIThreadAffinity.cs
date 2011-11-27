using System;
using System.Collections.Generic;
using System.Text;
using System.Reflection;
using EVEMon.Common.Threading;

#if !NO_THREAD_SAFETY
using PostSharp.Laos;
using PostSharp.Reflection;
using PostSharp.Extensibility;
#endif

namespace EVEMon.Common.Attributes
{

#if NO_THREAD_SAFETY
    internal sealed class EnforceUIThreadAffinityAttribute : Attribute
    {
    }
#else
    /// <summary>
    /// All public and internal descendant methods (methods, properties accessors, etc - not constructors) will begin with a call to AssertAccess() (will check the execution performs on the <see cref="DataObject.CommonActor"/> thread)
    /// </summary>
    [Serializable]
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct | AttributeTargets.Property | AttributeTargets.Method)]

    // Propagation rule, apply to all public and internal descendant methods (on a class, will transform all methods, props accessors, etc)
    [MulticastAttributeUsage(MulticastTargets.Method, Inheritance = MulticastInheritance.Multicast,
        TargetMemberAttributes = 
        MulticastAttributes.AnyScope | 
        MulticastAttributes.Managed | MulticastAttributes.NonAbstract | 
        MulticastAttributes.Public | MulticastAttributes.Internal | MulticastAttributes.InternalOrProtected)]

    // Method
    internal sealed class EnforceUIThreadAffinityAttribute : OnMethodBoundaryAspect
    {
        // Specify the code to write at the beginning of every public / internal method
        public override void OnEntry(MethodExecutionEventArgs context)
        {
            Dispatcher.AssertAccess();
        }
    }
#endif
}
