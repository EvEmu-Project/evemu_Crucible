#if !NO_THREAD_SAFETY
using System;
using System.Collections.Generic;
using System.Text;
using PostSharp.Extensibility;
using PostSharp.Laos;
using EVEMon.Common.Attributes.Internals;
using System.Reflection;

namespace EVEMon.Common.Attributes
{
    /// <summary>
    /// Auto-implements the <see cref="INotifyPropertyChanged"/> interface and transform the setters to make them fires the <see cref="INotifyPropertyChanged.PropertyChanged"/> event.
    /// </summary>
    [Serializable]
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct)]

    // Propagation to inheritors and subclasses and children structs
    [MulticastAttributeUsage(MulticastTargets.Class | MulticastTargets.Struct, Inheritance = MulticastInheritance.Strict)]

    // Definition
    public sealed class NotifyPropertyChangedAttribute : CompoundAspect
    { 
        [NonSerialized] 
        private int aspectPriority = 0; 
        
        public override void ProvideAspects(object targetElement, LaosReflectionAspectCollection collection) 
        { 
            // Get the target type. 
            Type targetType = (Type)targetElement; 
            
            // On the type, add a Composition aspect to implement the INotifyPropertyChanged interface. 
            collection.AddAspect(targetType, new AddNotifyPropertyChangedInterfaceSubAspect()); 
            
            // Add a OnMethodBoundaryAspect on each writable non-static property. 
            foreach (PropertyInfo property in targetType.GetProperties()) 
            { 
                if (property.DeclaringType == targetType && property.CanWrite ) 
                { 
                    MethodInfo method = property.GetSetMethod(); 
                    if (!method.IsStatic) 
                    { 
                        collection.AddAspect(method, new OnPropertySetSubAspect(property.Name, this)); 
                    } 
                } 
            } 
        } 
        
        public int AspectPriority 
        { 
            get 
            { 
                return aspectPriority; 
            } 
            set 
            { 
                aspectPriority = value; 
            } 
        } 
    }
}
#endif