#if !NO_THREAD_SAFETY
using System;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel;
using PostSharp.Laos;

namespace EVEMon.Common.Attributes.Internals
{
    /// <summary>
    /// Provides an implementation for <see cref="INotifyPropertyChanged"/>. It never inherited, just reimplemented by our custom attributes.
    /// </summary>
    internal class NotifyPropertyChangedImplementation : INotifyPropertyChanged 
    { 
        private object instance;
        public event PropertyChangedEventHandler PropertyChanged;

        public NotifyPropertyChangedImplementation(object instance) 
        { 
            this.instance = instance; 
        } 
        
        public void OnPropertyChanged(string propertyName) 
        { 
            if (this.PropertyChanged != null) 
            { 
                this.PropertyChanged(this.instance, new PropertyChangedEventArgs(propertyName)); 
            } 
        } 
    }

    /// <summary>
    /// Auto-implements <see cref="INotifyPropertyChanged"/> on the class, copying the <see cref="NotifyPropertyChangedImplementation"/> implementation.
    /// This attribute does not transform the property setters though.
    /// </summary>
    [Serializable]
    internal class AddNotifyPropertyChangedInterfaceSubAspect : CompositionAspect
    {
        public override object CreateImplementationObject(InstanceBoundLaosEventArgs eventArgs) 
        { 
            return new NotifyPropertyChangedImplementation(eventArgs.Instance); 
        }    
        
        public override Type GetPublicInterface(Type containerType) 
        { 
            return typeof(INotifyPropertyChanged); 
        }

        public override CompositionAspectOptions GetOptions()
        {
            return CompositionAspectOptions.GenerateImplementationAccessor | CompositionAspectOptions.IgnoreIfAlreadyImplemented;
        }
    }

    /// <summary>
    /// Auto-fires the <see cref="INotifyPropertyChanged.PropertyChanged"/> event at the end of the method.
    /// </summary>
    [Serializable]
    internal class OnPropertySetSubAspect : OnMethodBoundaryAspect
    {
        string propertyName; 
        
        public OnPropertySetSubAspect(string propertyName, NotifyPropertyChangedAttribute parent) 
        { 
            this.AspectPriority = parent.AspectPriority; 
            this.propertyName = propertyName; 
        }

        public override void OnSuccess(MethodExecutionEventArgs eventArgs)
        {
            var instance = (IComposed<INotifyPropertyChanged>)eventArgs.Instance;
            var implementation = (NotifyPropertyChangedImplementation)instance.GetImplementation(eventArgs.InstanceCredentials); 
            implementation.OnPropertyChanged(this.propertyName);
        }
    }

}
#endif